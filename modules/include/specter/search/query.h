#ifndef SPECTER_SEARCH_QUERY_H
#define SPECTER_SEARCH_QUERY_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QObject>
#include <QVariantMap>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ---------------------------------- ObjectQuery --------------------------- */

class LIB_SPECTER_API ObjectQuery {
  friend class Searcher;

public:
  [[nodiscard]] static ObjectQuery fromString(const QString &query);

public:
  explicit ObjectQuery();
  ~ObjectQuery();

  [[nodiscard]] QString toString() const;

  [[nodiscard]] bool operator==(const ObjectQuery &other) const;
  [[nodiscard]] bool operator!=(const ObjectQuery &other) const;

protected:
  explicit ObjectQuery(QVariantMap data);

private:
  QVariantMap m_data;
};

}// namespace specter

#endif// SPECTER_SEARCH_QUERY_H