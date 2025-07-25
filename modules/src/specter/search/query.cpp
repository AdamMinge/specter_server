/* ----------------------------------- Local -------------------------------- */
#include "specter/search/query.h"

#include "specter/search/strategy.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QJsonDocument>
#include <QJsonObject>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ---------------------------------- ObjectQuery --------------------------- */

ObjectQuery::ObjectQuery() : ObjectQuery(QVariantMap{}) {}

ObjectQuery::ObjectQuery(QVariantMap data) : m_data(std::move(data)) {}

ObjectQuery::~ObjectQuery() = default;

ObjectQuery ObjectQuery::fromString(const QString &id) {
  auto data = QVariantMap{};

  auto parser_error = QJsonParseError{};
  const auto json_document =
    QJsonDocument::fromJson(id.toUtf8(), &parser_error);

  if (parser_error.error == QJsonParseError::NoError) {
    const auto json_object = json_document.object();

    for (auto it = json_object.begin(); it != json_object.end(); ++it) {
      data.insert(it.key(), it.value().toVariant());
    }
  }

  return ObjectQuery(data);
}

QString ObjectQuery::toString() const {
  auto json_object = QJsonObject{};
  for (auto it = m_data.begin(); it != m_data.end(); ++it) {
    json_object.insert(it.key(), QJsonValue::fromVariant(it.value()));
  }

  const auto json_document = QJsonDocument(json_object);
  return QString(json_document.toJson(QJsonDocument::Compact));
}

bool ObjectQuery::operator==(const ObjectQuery &other) const {
  return m_data == other.m_data;
}

bool ObjectQuery::operator!=(const ObjectQuery &other) const {
  return m_data != other.m_data;
}

/* --------------------------------- CommonQueries -------------------------- */

ObjectQuery CommonQueries::createPathObjectQuery(const QString &path) {
  return ObjectQuery(
    QVariantMap{{PathSearch::path_query, QVariant::fromValue(path)}});
}

ObjectQuery CommonQueries::createTypeObjectQuery(const QMetaType &type) {
  return ObjectQuery(
    QVariantMap{{TypeSearch::type_query, QVariant::fromValue(type.name())}});
}

ObjectQuery
CommonQueries::createPropertiesObjectQuery(const QVariantMap &properties) {
  return ObjectQuery(
    QVariantMap{
      {PropertiesSearch::properties_query, QVariant::fromValue(properties)}});
}

}// namespace specter
