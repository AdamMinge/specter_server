#ifndef SPECTER_SEARCH_ID_H
#define SPECTER_SEARCH_ID_H

/* ------------------------------------- Qt --------------------------------- */
#include <QObject>
#include <QString>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ----------------------------------- ObjectId ----------------------------- */

class LIB_SPECTER_API ObjectId {
  friend class Searcher;

public:
  [[nodiscard]] static ObjectId fromString(const QString &id);

public:
  explicit ObjectId();
  ~ObjectId();

  [[nodiscard]] QString toString() const;

  [[nodiscard]] bool operator==(const ObjectId &other) const;
  [[nodiscard]] bool operator!=(const ObjectId &other) const;

protected:
  explicit ObjectId(const QObject *object);

private:
  quint64 m_data;
};

}// namespace specter

#endif// SPECTER_SEARCH_ID_H