/* ----------------------------------- Local -------------------------------- */
#include "specter/observe/property/observer.h"

#include "specter/module.h"
#include "specter/search/utils.h"
#include "specter/thread/invoke.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
#include <QBrush>
#include <QFont>
#include <QIcon>
#include <QMetaProperty>
/* --------------------------------- Standard ------------------------------- */
#include <queue>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ PropertyObserver -------------------------- */

PropertyObserver::PropertyObserver()
    : m_object(nullptr), m_observing(false), m_check_timer(new QTimer(this)) {
  m_check_timer->setInterval(100);
  connect(
    m_check_timer, &QTimer::timeout, this, &PropertyObserver::checkForChanges);
}

PropertyObserver::~PropertyObserver() { stop(); }

void PropertyObserver::setObject(QObject *object) {
  InvokeInObjectThread(qApp, [this, object]() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_object != object) {
      m_object = object;
      m_tracked_properties = getTrackedProperties();
    }
  });
}

QObject *PropertyObserver::getObject() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_object;
}

void PropertyObserver::start() {
  InvokeInObjectThread(qApp, [this]() { startChangesTracker(); });
}

void PropertyObserver::stop() {
  InvokeInObjectThread(qApp, [this]() { stopChangesTracker(); });
}

bool PropertyObserver::isObserving() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_observing;
}

void PropertyObserver::startChangesTracker() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_observing) return;

  m_observing = true;
  m_check_timer->start();
  m_tracked_properties = getTrackedProperties();
}

void PropertyObserver::stopChangesTracker() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_observing) return;

  m_observing = false;
  m_check_timer->stop();
  m_tracked_properties.clear();
}

void PropertyObserver::checkForChanges() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_object) return;

  auto current_properties = getTrackedProperties();
  for (const auto &old_prop : m_tracked_properties) {
    if (!current_properties.contains(old_prop.first)) {
      Q_EMIT actionReported(
        PropertyObservedAction::PropertyRemoved{old_prop.first});
    }
  }

  for (const auto &cur_prop : current_properties) {
    if (!m_tracked_properties.contains(cur_prop.first)) {
      Q_EMIT actionReported(
        PropertyObservedAction::PropertyAdded{cur_prop.first, cur_prop.second});
    } else {
      const auto &old_value = m_tracked_properties[cur_prop.first];
      if (!variantEqual(old_value, cur_prop.second)) {
        Q_EMIT actionReported(PropertyObservedAction::PropertyUpdated{
          cur_prop.first, old_value, cur_prop.second});
      }
    }
  }

  m_tracked_properties = current_properties;
}

std::map<QString, QVariant> PropertyObserver::getTrackedProperties() const {
  if (!m_object) return {};

  auto unique_properties = std::set<QString>{};
  auto meta_object = m_object->metaObject();
  for (auto i = 0; i < meta_object->propertyCount(); ++i) {
    const auto name = meta_object->property(i).name();
    unique_properties.insert(name);
  }

  auto tracked_properties = std::map<QString, QVariant>{};
  for (auto unique_property : unique_properties) {
    const auto value =
      m_object->property(unique_property.toStdString().c_str());
    tracked_properties[unique_property] = value;
  }

  return tracked_properties;
}

bool PropertyObserver::variantEqual(
  const QVariant &a, const QVariant &b) const {
  if (a.userType() != b.userType()) return false;

  switch (a.userType()) {
    case QMetaType::QIcon: {
      QIcon iconA = a.value<QIcon>();
      QIcon iconB = b.value<QIcon>();
      return iconA.cacheKey() == iconB.cacheKey();
    }
    case QMetaType::QPixmap: {
      QPixmap pxA = a.value<QPixmap>();
      QPixmap pxB = b.value<QPixmap>();
      return pxA.cacheKey() == pxB.cacheKey();
    }
    case QMetaType::QImage: {
      QImage imgA = a.value<QImage>();
      QImage imgB = b.value<QImage>();
      return imgA == imgB;
    }
    case QMetaType::QBrush: {
      QBrush brushA = a.value<QBrush>();
      QBrush brushB = b.value<QBrush>();
      return brushA == brushB;
    }
    case QMetaType::QFont: {
      QFont f1 = a.value<QFont>();
      QFont f2 = b.value<QFont>();
      return f1 == f2;
    }

    default:
      return a == b;
  }
}

/* ---------------------------- PropertyObserverQueue ----------------------- */

PropertyObserverQueue::PropertyObserverQueue() : m_observer(nullptr) {}

PropertyObserverQueue::~PropertyObserverQueue() = default;

void PropertyObserverQueue::setObserver(PropertyObserver *observer) {
  if (m_observer) {
    m_observer->disconnect(m_on_action_reported);
    m_observed_actions.clear();
  }

  m_observer = observer;

  if (m_observer) {
    m_on_action_reported = QObject::connect(
      m_observer, &PropertyObserver::actionReported,
      [this](const auto recorder_action) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_observed_actions.push_back(recorder_action);
        }

        m_cv.notify_one();
      });
  }
}

bool PropertyObserverQueue::isEmpty() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_observed_actions.empty();
}

PropertyObservedAction PropertyObserverQueue::popAction() {
  std::lock_guard<std::mutex> lock(m_mutex);
  Q_ASSERT(!m_observed_actions.empty());
  return m_observed_actions.takeFirst();
}

PropertyObservedAction PropertyObserverQueue::waitPopAction() {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_cv.wait(lock, [this] { return !m_observed_actions.empty(); });

  return m_observed_actions.takeFirst();
}

}// namespace specter
