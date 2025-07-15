/* ----------------------------------- Local -------------------------------- */
#include "specter/observe/tree/observer.h"

#include "specter/module.h"
#include "specter/search/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
/* --------------------------------- Standard ------------------------------- */
#include <queue>
#include <set>
/* -------------------------------------------------------------------------- */

namespace specter {

/* -------------------------------- TreeObserver ---------------------------- */

TreeObserver::TreeObserver()
    : m_observing(false), m_check_timer(new QTimer(this)) {
  m_check_timer->setInterval(100);
  connect(m_check_timer, &QTimer::timeout, this, &TreeObserver::intervalCheck);
}

TreeObserver::~TreeObserver() { stop(); }

void TreeObserver::start() {
  if (m_observing) return;
  m_observing = true;

  startIntervalCheck();
}

void TreeObserver::stop() {
  if (!m_observing) return;
  m_observing = false;

  stopIntervalCheck();
}

bool TreeObserver::isObserving() const { return m_observing; }

void TreeObserver::startIntervalCheck() {
  auto objects = std::queue<QObject *>{};
  for (auto top_widget : getTopLevelObjects()) { objects.push(top_widget); }

  while (!objects.empty()) {
    auto object = objects.front();
    objects.pop();

    auto query = searcher().getQuery(object);
    m_tracked_objects.insert(
      std::make_pair(
        object, TrackedObjectCache{query, object, object->parent()}));

    for (const auto child : object->children()) { objects.push(child); }
  }

  m_check_timer->start();
}

void TreeObserver::stopIntervalCheck() {
  m_check_timer->stop();
  m_tracked_objects.clear();
}

void TreeObserver::intervalCheck() {
  checkForCreatedObjects();
  checkForDestroyedObjects();
  checkForReparentedObjects();
  checkForRenamedObjects();
}

void TreeObserver::checkForCreatedObjects() {
  std::queue<QObject *> objects;
  for (auto top_widget : getTopLevelObjects()) { objects.push(top_widget); }

  while (!objects.empty()) {
    auto object = objects.front();
    objects.pop();

    if (!m_tracked_objects.contains(object)) {
      auto query = searcher().getQuery(object);
      auto parent = object->parent();
      auto parent_query =
        parent ? m_tracked_objects.at(parent).query : ObjectQuery{};

      m_tracked_objects.insert(
        std::make_pair(object, TrackedObjectCache{query, object, parent}));

      Q_EMIT actionReported(
        TreeObservedAction::ObjectAdded{query, parent_query});
    }

    for (const auto child : object->children()) { objects.push(child); }
  }
}

void TreeObserver::checkForDestroyedObjects() {
  std::vector<QObject *> objects_to_remove;
  std::queue<QObject *> queue;

  queue.push(nullptr);

  while (!queue.empty()) {
    auto parent = queue.front();
    queue.pop();

    for (auto it = m_tracked_objects.begin(); it != m_tracked_objects.end();
         ++it) {
      const auto object = it->first;
      const auto &cache = it->second;

      if (cache.parent == parent) {
        if (!cache.object) { objects_to_remove.push_back(object); }
        queue.push(object);
      }
    }
  }

  for (auto it = objects_to_remove.rbegin(); it != objects_to_remove.rend();
       ++it) {
    auto object = *it;
    const auto &query = m_tracked_objects.at(object).query;
    Q_EMIT actionReported(TreeObservedAction::ObjectRemoved{query});
    m_tracked_objects.erase(object);
  }
}

void TreeObserver::checkForReparentedObjects() {
  std::vector<QObject *> objects_to_reparent;
  std::queue<QObject *> queue;

  queue.push(nullptr);

  while (!queue.empty()) {
    auto parent = queue.front();
    queue.pop();

    for (auto it = m_tracked_objects.begin(); it != m_tracked_objects.end();
         ++it) {
      const auto object = it->first;
      const auto &cache = it->second;

      if (cache.parent == parent) {
        auto current_parent = object->parent();
        if (cache.parent != current_parent) {
          objects_to_reparent.push_back(object);
        }
        queue.push(object);
      }
    }
  }

  for (auto it = objects_to_reparent.rbegin(); it != objects_to_reparent.rend();
       ++it) {
    auto object = *it;
    auto current_parent = object->parent();
    auto &cache = m_tracked_objects.at(object);

    Q_EMIT actionReported(
      TreeObservedAction::ObjectReparented{
        cache.query, current_parent ? m_tracked_objects.at(current_parent).query
                                    : ObjectQuery{}});
    cache.parent = current_parent;
  }
}

void TreeObserver::checkForRenamedObjects() {
  std::queue<QObject *> queue;
  queue.push(nullptr);

  while (!queue.empty()) {
    auto parent = queue.front();
    queue.pop();

    for (auto it = m_tracked_objects.begin(); it != m_tracked_objects.end();
         ++it) {
      const auto object = it->first;
      auto &cache = it->second;

      if (cache.parent == parent) {
        auto new_query = searcher().getQuery(object);
        if (cache.query != new_query) {
          auto old_query = cache.query;
          cache.query = new_query;
          Q_EMIT actionReported(
            TreeObservedAction::ObjectRenamed{old_query, new_query});
        }
        queue.push(object);
      }
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
