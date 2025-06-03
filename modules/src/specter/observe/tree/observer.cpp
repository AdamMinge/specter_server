/* ----------------------------------- Local -------------------------------- */
#include "specter/observe/tree/observer.h"

#include "specter/module.h"
#include "specter/search/utils.h"
#include "specter/thread/invoke.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
/* --------------------------------- Standard ------------------------------- */
#include <queue>
/* -------------------------------------------------------------------------- */

namespace specter {

/* -------------------------------- TreeObserver ---------------------------- */

TreeObserver::TreeObserver()
    : m_observing(false), m_check_timer(new QTimer(this)) {
  m_check_timer->setInterval(100);
  connect(
    m_check_timer, &QTimer::timeout, this, &TreeObserver::checkForRenames);
}

TreeObserver::~TreeObserver() { stop(); }

void TreeObserver::start() {
  InvokeInObjectThread(qApp, [this]() {
    if (m_observing) return;
    m_observing = true;

    startRenameTracker();
    qApp->installEventFilter(this);
  });
}

void TreeObserver::stop() {
  InvokeInObjectThread(qApp, [this]() {
    if (!m_observing) return;
    m_observing = false;

    stopRenameTracker();
    qApp->removeEventFilter(this);
  });
}

bool TreeObserver::isObserving() const { return m_observing; }

bool TreeObserver::eventFilter(QObject *object, QEvent *event) {
  switch (event->type()) {
    case QEvent::Create: {
      std::lock_guard<std::mutex> lock(m_mutex);

      m_tracked_objects.insert(object, searcher().getQuery(object));

      Q_EMIT actionReported(TreeObservedAction::ObjectAdded{
        m_tracked_objects.value(object),
        m_tracked_objects.value(object->parent())});

      break;
    }

    case QEvent::Destroy: {
      std::lock_guard<std::mutex> lock(m_mutex);

      Q_EMIT actionReported(
        TreeObservedAction::ObjectRemoved{m_tracked_objects.value(object)});

      m_tracked_objects.remove(object);
      break;
    }

    case QEvent::ParentChange: {
      std::lock_guard<std::mutex> lock(m_mutex);

      if (!m_tracked_objects.contains(object)) break;

      Q_EMIT actionReported(TreeObservedAction::ObjectReparented{
        m_tracked_objects.value(object),
        m_tracked_objects.value(object->parent())});
      break;
    }
  }

  return QObject::eventFilter(object, event);
}

void TreeObserver::startRenameTracker() {
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

void TreeObserver::stopRenameTracker() {
  std::lock_guard<std::mutex> lock(m_mutex);

  m_check_timer->stop();
  m_tracked_objects.clear();
}

void TreeObserver::checkForRenames() {
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
        TreeObservedAction::ObjectRenamed{old_query, new_query});
    }
  }
}

/* ------------------------------ TreeObserverQueue ------------------------- */

TreeObserverQueue::TreeObserverQueue() : m_observer(nullptr) {}

TreeObserverQueue::~TreeObserverQueue() = default;

void TreeObserverQueue::setObserver(TreeObserver *observer) {
  if (m_observer) {
    m_observer->disconnect(m_on_action_reported);
    m_observed_actions.clear();
  }

  m_observer = observer;

  if (m_observer) {
    m_on_action_reported = QObject::connect(
      m_observer, &TreeObserver::actionReported,
      [this](const auto recorder_action) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_observed_actions.push_back(recorder_action);
        }

        m_cv.notify_one();
      });
  }
}

bool TreeObserverQueue::isEmpty() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_observed_actions.empty();
}

TreeObservedAction TreeObserverQueue::popAction() {
  std::lock_guard<std::mutex> lock(m_mutex);
  Q_ASSERT(!m_observed_actions.empty());
  return m_observed_actions.takeFirst();
}

TreeObservedAction TreeObserverQueue::waitPopAction() {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_cv.wait(lock, [this] { return !m_observed_actions.empty(); });

  return m_observed_actions.takeFirst();
}

}// namespace specter
