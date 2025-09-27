/* ----------------------------------- Local -------------------------------- */
#include "specter/search/searcher.h"

#include "specter/search/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
#include <QWidget>
/* --------------------------------- Standard ------------------------------- */
#include <queue>
/* -------------------------------------------------------------------------- */

namespace specter {

/* --------------------------------- Searcher ------------------------------- */

Searcher::Searcher() = default;

Searcher::~Searcher() = default;

QObject *Searcher::getObject(const ObjectQuery &query) const {
  const auto objects = findObjects(query, 1);
  return objects.empty() ? nullptr : objects.first();
}

QObject *Searcher::getObject(const ObjectId &id) const {
  const auto top_widgets = getTopLevelObjects();
  auto objects = std::queue<QObject *>{};
  for (auto top_widget : top_widgets) {
    if (!top_widget->parent()) objects.push(top_widget);
  }

  while (!objects.empty()) {
    auto object = objects.front();
    objects.pop();

    if (id == getId(object)) return object;

    for (const auto &child : object->children()) { objects.push(child); }
  }

  return nullptr;
}

QList<QObject *> Searcher::getObjects(const ObjectQuery &query) const {
  const auto objects = findObjects(query);
  return objects;
}

ObjectQuery Searcher::getQuery(const QObject *object) const {
  return getQueryFiltered(object, {});
}

ObjectQuery Searcher::getQueryUsingKinds(
  const QObject *object, const QSet<SearchStrategy::Kind> &kinds) const {
  if (!object) return ObjectQuery{};

  auto query = QVariantMap{};
  for (const auto &search_strategy : m_strategies) {
    if (kinds.contains(search_strategy->kind())) {
      const auto sub_query = search_strategy->createObjectQuery(object);
      query.insert(sub_query);
    }
  }

  return ObjectQuery(query);
}

ObjectQuery Searcher::getQueryFiltered(
  const QObject *object, const QSet<SearchStrategy::Kind> &excludeKinds) const {

  QSet<SearchStrategy::Kind> kinds;
  for (const auto &search_strategy : m_strategies) {
    if (!excludeKinds.contains(search_strategy->kind())) {
      kinds.insert(search_strategy->kind());
    }
  }

  return getQueryUsingKinds(object, kinds);
}

ObjectId Searcher::getId(const QObject *object) const {
  if (!object) return ObjectId{};
  return ObjectId(object);
}

void Searcher::addStrategy(std::unique_ptr<SearchStrategy> &&strategy) {
  m_strategies.emplace_back(std::move(strategy));
}

QList<QObject *>
Searcher::findObjects(const ObjectQuery &query, qsizetype limit) const {
  const auto top_widgets = getTopLevelObjects();
  auto objects = std::queue<QObject *>{};
  for (auto top_widget : top_widgets) {
    if (!top_widget->parent()) objects.push(top_widget);
  }

  auto found_objects = QList<QObject *>{};
  while (!objects.empty() && found_objects.size() < limit) {
    auto object = objects.front();
    objects.pop();

    const auto matches_query = std::all_of(
      m_strategies.begin(), m_strategies.end(),
      [object, &query](const auto &search_strategy) {
        return search_strategy->matchesObjectQuery(object, query.m_data);
      });

    if (matches_query) { found_objects.push_back(object); }

    for (const auto &child : object->children()) { objects.push(child); }
  }

  return found_objects;
}

}// namespace specter
