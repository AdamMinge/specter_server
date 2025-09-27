#ifndef SPECTER_SEARCH_SEARCHER_H
#define SPECTER_SEARCH_SEARCHER_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QObject>
/* --------------------------------- Standard ------------------------------- */
#include <list>
#include <map>
#include <memory>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/id.h"
#include "specter/search/query.h"
#include "specter/search/strategy.h"
/* -------------------------------------------------------------------------- */

namespace specter {

class SearchStrategy;

/* ----------------------------------- Searcher ----------------------------- */

class LIB_SPECTER_API Searcher : public QObject {
  Q_OBJECT

public:
  explicit Searcher();
  ~Searcher() override;

  [[nodiscard]] QObject *getObject(const ObjectQuery &query) const;
  [[nodiscard]] QObject *getObject(const ObjectId &id) const;
  [[nodiscard]] QList<QObject *> getObjects(const ObjectQuery &query) const;

  [[nodiscard]] ObjectQuery getQuery(const QObject *object) const;
  [[nodiscard]] ObjectQuery getQueryUsingKinds(
    const QObject *object, const QSet<SearchStrategy::Kind> &kinds) const;
  [[nodiscard]] ObjectQuery getQueryFiltered(
    const QObject *object,
    const QSet<SearchStrategy::Kind> &excludeKinds) const;

  [[nodiscard]] ObjectId getId(const QObject *object) const;

  void addStrategy(std::unique_ptr<SearchStrategy> &&strategy);

private:
  [[nodiscard]] QList<QObject *> findObjects(
    const ObjectQuery &query,
    qsizetype limit = std::numeric_limits<qsizetype>::max()) const;

private:
  std::list<std::unique_ptr<SearchStrategy>> m_strategies;
};

}// namespace specter

#endif// SPECTER_SEARCH_SEARCHER_H