/* ----------------------------------- Local -------------------------------- */
#include "specter/observe/tree/observer.h"

#include "specter/module.h"
#include "specter/search/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
/* --------------------------------- Standard ------------------------------- */
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

void TreeObserver::startIntervalCheck() { m_check_timer->start(); }

void TreeObserver::stopIntervalCheck() {
  m_check_timer->stop();
  m_tracked_objects.clear();
}

void TreeObserver::intervalCheck() {
  checkForDestroyedObjects();
  checkForCreatedObjects();
  checkForRenamedObjects();
  checkForReparentedObjects();
}

void TreeObserver::checkForCreatedObjects() {
  std::queue<QObject *> objects;
  for (auto top_widget : getTopLevelObjects()) { objects.push(top_widget); }

  while (!objects.empty()) {
    auto object = objects.front();
    objects.pop();

    if (!m_tracked_objects.contains(object)) {
      auto object_id = searcher().getId(object);
      auto object_query = searcher().getQuery(object);
      auto parent = object->parent();
      auto parent_id =
        parent ? m_tracked_objects.at(parent).object_id : ObjectId{};

      m_tracked_objects.insert(
        std::make_pair(
          object, TrackedObjectCache{object_id, object_query, object, parent}));

      Q_EMIT actionReported(
        TreeObservedAction::ObjectAdded{object_id, parent_id});
      Q_EMIT actionReported(
        TreeObservedAction::ObjectRenamed{object_id, object_query});
    }

    for (const auto child : object->children()) { objects.push(child); }
  }
}

void TreeObserver::checkForDestroyedObjects() {
  auto toRemove = [this](auto object) {
    auto parents = std::queue<QObject *>{};
    parents.push(object);

    while (!parents.empty()) {
      auto parent = parents.front();
      parents.pop();

      const auto &cache = m_tracked_objects.at(parent);
      if (!cache.object_ptr) return true;
      if (cache.parent) parents.push(cache.parent);
    }

    return false;
  };

  auto objects = getTrackedObjectsInDFSOrder();
  for (auto object : objects) {
    if (toRemove(object)) {
      const auto &cache = m_tracked_objects.at(object);
      const auto &object_id = cache.object_id;

      Q_EMIT actionReported(TreeObservedAction::ObjectRemoved{object_id});
      m_tracked_objects.erase(object);
    }
  }
}

void TreeObserver::checkForRenamedObjects() {
  auto objects = getTrackedObjectsInDFSOrder();
  for (auto object : objects) {
    auto &cache = m_tracked_objects.at(object);
    const auto current_query = searcher().getQuery(object);

    if (cache.object_query != current_query) {
      Q_EMIT actionReported(
        TreeObservedAction::ObjectRenamed{cache.object_id, current_query});
      cache.object_query = current_query;
    }
  }
}

void TreeObserver::checkForReparentedObjects() {
  auto objects = getTrackedObjectsInDFSOrder();
  for (auto object : objects) {
    auto &cache = m_tracked_objects.at(object);
    if (!cache.object_ptr) continue;

    const auto &object_id = cache.object_id;
    const auto current_parent = object->parent();

    auto opt_action = std::optional<TreeObservedAction::ObjectReparented>{};

    if (cache.parent != current_parent) {
      opt_action = TreeObservedAction::ObjectReparented{
        object_id, current_parent
                     ? m_tracked_objects.at(current_parent).object_id
                     : ObjectId{}};
      cache.parent = current_parent;
    }

    if (opt_action) Q_EMIT actionReported(*opt_action);
  }
}

QList<QObject *> TreeObserver::getTrackedObjectsInDFSOrder() const {
  QList<QObject *> result = {};
  QSet<QObject *> visited = {};

  QSet<QObject *> allActiveTrackedObjects = {};
  QMap<QObject *, QObject *> objectToParentMap = {};
  QMap<QObject *, QList<QObject *>> childrenOf = {};

  for (const auto &pair : m_tracked_objects) {
    auto currentObjectKey = pair.first;
    const auto &cache = pair.second;

    allActiveTrackedObjects.insert(currentObjectKey);
    objectToParentMap[currentObjectKey] = cache.parent;
  }

  for (auto currentObject : allActiveTrackedObjects) {
    auto parent = objectToParentMap.value(currentObject, nullptr);

    if (parent != nullptr && allActiveTrackedObjects.contains(parent)) {
      childrenOf[parent].append(currentObject);
    }
  }

  QList<QObject *> roots = {};
  for (auto obj : allActiveTrackedObjects) {
    auto parent = objectToParentMap.value(obj, nullptr);

    if (parent == nullptr || !allActiveTrackedObjects.contains(parent)) {
      roots.append(obj);
    }
  }


  std::function<void(QObject *)> dfs_post_order = [&](auto currentObject) {
    if (
      !allActiveTrackedObjects.contains(currentObject) ||
      visited.contains(currentObject)) {
      return;
    }
    visited.insert(currentObject);

    if (childrenOf.contains(currentObject)) {
      for (auto child : childrenOf.value(currentObject)) {
        dfs_post_order(child);
      }
    }

    result.append(currentObject);
  };

  for (auto root : roots) { dfs_post_order(root); }

  return result;
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
