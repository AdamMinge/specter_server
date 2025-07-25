#ifndef SPECTER_SEARCH_STRATEGY_H
#define SPECTER_SEARCH_STRATEGY_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QLatin1String>
#include <QObject>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ SearchStrategy ---------------------------- */

class LIB_SPECTER_API SearchStrategy {
public:
  explicit SearchStrategy();
  virtual ~SearchStrategy();

  [[nodiscard]] virtual bool
  matchesObjectQuery(const QObject *object, const QVariantMap &query) const = 0;
  [[nodiscard]] virtual QVariantMap
  createObjectQuery(const QObject *object) const = 0;
};

/* -------------------------------- TypeSearch ------------------------------ */

class LIB_SPECTER_API TypeSearch : public SearchStrategy {
public:
  static constexpr auto type_query = "type";

public:
  explicit TypeSearch();
  ~TypeSearch() override;

  [[nodiscard]] bool matchesObjectQuery(
    const QObject *object, const QVariantMap &query) const override;
  [[nodiscard]] QVariantMap
  createObjectQuery(const QObject *object) const override;
};

/* ----------------------------- PropertiesSearch --------------------------- */

class LIB_SPECTER_API PropertiesSearch : public SearchStrategy {
public:
  static constexpr auto properties_query = "properties";

public:
  explicit PropertiesSearch();
  ~PropertiesSearch() override;

  [[nodiscard]] bool matchesObjectQuery(
    const QObject *object, const QVariantMap &query) const override;
  [[nodiscard]] QVariantMap
  createObjectQuery(const QObject *object) const override;

private:
  [[nodiscard]] static QSet<QString> getUsedProperties(const QObject *object);
  [[nodiscard]] static QMap<int, QSet<QString>> getTypeToProperties();
};

/* -------------------------------- PathSearch ------------------------------ */

class LIB_SPECTER_API PathSearch : public SearchStrategy {
public:
  static constexpr auto path_query = "path";

public:
  explicit PathSearch();
  ~PathSearch() override;

  [[nodiscard]] bool matchesObjectQuery(
    const QObject *object, const QVariantMap &query) const override;
  [[nodiscard]] QVariantMap
  createObjectQuery(const QObject *object) const override;

private:
  [[nodiscard]] QString getPath(const QObject *object) const;
};

/* ----------------------------- OrderIndexSearch --------------------------- */

class LIB_SPECTER_API OrderIndexSearch : public SearchStrategy {
public:
  static constexpr auto order_index_query = "order_index";

public:
  explicit OrderIndexSearch();
  ~OrderIndexSearch() override;

  [[nodiscard]] bool matchesObjectQuery(
    const QObject *object, const QVariantMap &query) const override;
  [[nodiscard]] QVariantMap
  createObjectQuery(const QObject *object) const override;

private:
  [[nodiscard]] uint getOrderIndex(const QObject *object) const;
};

/* ----------------------------- SessionIdSearch ---------------------------- */

class LIB_SPECTER_API SessionIdSearch : public SearchStrategy {
public:
  static constexpr auto session_id_query = "session_id";

public:
  explicit SessionIdSearch();
  ~SessionIdSearch() override;

  [[nodiscard]] bool matchesObjectQuery(
    const QObject *object, const QVariantMap &query) const override;
  [[nodiscard]] QVariantMap
  createObjectQuery(const QObject *object) const override;

private:
  [[nodiscard]] uint getSessionId(const QObject *object) const;
};

}// namespace specter

#endif// SPECTER_SEARCH_STRATEGY_H