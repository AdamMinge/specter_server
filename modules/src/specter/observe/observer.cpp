/* ----------------------------------- Local -------------------------------- */
#include "specter/observe/observer.h"

#include "specter/module.h"
#include "specter/search/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
/* --------------------------------- Standard ------------------------------- */
#include <queue>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------- ObjectObserver --------------------------- */

ObjectObserver::ObjectObserver()
    : m_observing(false), m_check_timer(new QTimer(this)) {
  m_check_timer->setInterval(100);
  connect(
    m_check_timer, &QTimer::timeout, this, &ObjectObserver::checkForRenames);
}

ObjectObserver::~ObjectObserver() { stop(); }

void ObjectObserver::start() {
  if (m_observing) return;


  QMetaObject::invokeMethod(
    qApp,
    [this]() {
      m_observing = true;
      startRenameTracker();
      qApp->installEventFilter(this);
    },
    Qt::QueuedConnection);
}

void ObjectObserver::stop() {
  if (!m_observing) return;


  QMetaObject::invokeMethod(
    qApp,
    [this]() {
      m_observing = false;
      stopRenameTracker();
      qApp->removeEventFilter(this);
    },
    Qt::QueuedConnection);
}

bool ObjectObserver::isObserving() const { return m_observing; }

bool ObjectObserver::eventFilter(QObject *object, QEvent *event) {
  switch (event->type()) {
    case QEvent::Create: {
      std::lock_guard<std::mutex> lock(m_mutex);

      m_tracked_objects.insert(object, searcher().getQuery(object));

      Q_EMIT actionReported(ObservedAction::ObjectAdded{
        m_tracked_objects.value(object),
        m_tracked_objects.value(object->parent())});

      break;
    }

    case QEvent::Destroy: {
      std::lock_guard<std::mutex> lock(m_mutex);

      Q_EMIT actionReported(
        ObservedAction::ObjectRemoved{m_tracked_objects.value(object)});

      m_tracked_objects.remove(object);
      break;
    }

    case QEvent::ParentChange: {
      std::lock_guard<std::mutex> lock(m_mutex);

      if (!m_tracked_objects.contains(object)) break;

      Q_EMIT actionReported(ObservedAction::ObjectReparented{
        m_tracked_objects.value(object),
        m_tracked_objects.value(object->parent())});
      break;
    }
  }

  return QObject::eventFilter(object, event);
}

void ObjectObserver::startRenameTracker() {
  std::lock_guard<std::mutex> lock(m_mutex);

  m_check_timer->start();

  const auto top_widgets = getTopLevelObjects();
  auto objects = std::queue<QObject *>{};
  for (auto top_widget : top_widgets) { objects.push(top_widget); }

  while (!objects.empty()) {
    auto object = objects.front();
    objects.pop();

    auto object_query = searcher().getQuery(object);
    m_tracked_objects.insert(object, object_query);

    for (const auto child : object->children()) { objects.push(child); }
  }
}

void ObjectObserver::stopRenameTracker() {
  std::lock_guard<std::mutex> lock(m_mutex);

  m_check_timer->stop();
  m_tracked_objects.clear();
}

void ObjectObserver::checkForRenames() {
  std::lock_guard<std::mutex> lock(m_mutex);

  for (auto it = m_tracked_objects.begin(); it != m_tracked_objects.end();
       ++it) {
    const auto &object = it.key();
    auto &query = it.value();

    auto new_query = searcher().getQuery(object);
    if (query != new_query) {
      auto old_query = query;
      query = new_query;

      Q_EMIT actionReported(
        ObservedAction::ObjectRenamed{old_query, new_query});
    }
  }
}

/* ----------------------------- ObjectObserverQueue ------------------------ */

ObjectObserverQueue::ObjectObserverQueue() : m_observer(nullptr) {}

ObjectObserverQueue::~ObjectObserverQueue() = default;

void ObjectObserverQueue::setObserver(ObjectObserver *observer) {
  if (m_observer) {
    m_observer->disconnect(m_on_action_reported);
    m_observed_actions.clear();
  }

  m_observer = observer;

  if (m_observer) {
    m_on_action_reported = QObject::connect(
      m_observer, &ObjectObserver::actionReported,
      [this](const auto recorder_action) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_observed_actions.push_back(recorder_action);
        }

        m_cv.notify_one();
      });
  }
}

bool ObjectObserverQueue::isEmpty() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_observed_actions.empty();
}

ObservedAction ObjectObserverQueue::popAction() {
  std::lock_guard<std::mutex> lock(m_mutex);
  Q_ASSERT(!m_observed_actions.empty());
  return m_observed_actions.takeFirst();
}

ObservedAction ObjectObserverQueue::waitPopAction() {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_cv.wait(lock, [this] { return !m_observed_actions.empty(); });

  return m_observed_actions.takeFirst();
}

}// namespace specter
