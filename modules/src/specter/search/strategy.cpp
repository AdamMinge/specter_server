/* ----------------------------------- Local -------------------------------- */
#include "specter/search/strategy.h"

#include "specter/search/utils.h"
/* --------------------------------- Standard ------------------------------- */
#include <set>
/* ------------------------------------ Qt ---------------------------------- */
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMetaProperty>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSlider>
#include <QTabWidget>
#include <QTableWidget>
#include <QVariantMap>
#include <QWidget>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ SearchStrategy ---------------------------- */

SearchStrategy::SearchStrategy() = default;

SearchStrategy::~SearchStrategy() = default;

/* -------------------------------- TypeSearch ------------------------------ */

TypeSearch::TypeSearch() = default;

TypeSearch::~TypeSearch() = default;

bool TypeSearch::matchesObjectQuery(
  const QObject *object, const QVariantMap &query) const {
  if (query.contains(type_query)) {
    return object->metaObject()->className() == query[type_query];
  }

  return true;
}

QVariantMap TypeSearch::createObjectQuery(const QObject *object) const {
  auto query = QVariantMap{};
  query[type_query] = object->metaObject()->className();

  return query;
}

/* ----------------------------- PropertiesSearch --------------------------- */

PropertiesSearch::PropertiesSearch() = default;

PropertiesSearch::~PropertiesSearch() = default;

bool PropertiesSearch::matchesObjectQuery(
  const QObject *object, const QVariantMap &query) const {
  if (query.contains(properties_query)) {
    const auto properties = query[properties_query].toMap();

    const auto used_properties = getUsedProperties(object);
    for (const auto &property : used_properties) {
      if (object->property(property.toUtf8().data()) != properties[property])
        return false;
    }
  }

  return true;
}

QVariantMap PropertiesSearch::createObjectQuery(const QObject *object) const {
  auto query = QVariantMap{};
  auto properties = QVariantMap{};

  const auto used_properties = getUsedProperties(object);
  for (const auto &property : used_properties) {
    if (auto value = object->property(property.toUtf8().data());
        value.isValid()) {
      properties[property] = value;
    }
  }

  query[properties_query] = properties;

  return query;
}

QSet<QString> PropertiesSearch::getUsedProperties(const QObject *object) {
  static const auto type_to_properties = getTypeToProperties();

  auto used_properties = QSet<QString>{};

  auto meta_object = object->metaObject();
  while (meta_object) {
    const auto type_id = meta_object->metaType().id();
    const auto type_properties = type_to_properties.find(type_id);

    if (type_properties != type_to_properties.end()) {
      return *type_properties;
    }

    meta_object = meta_object->superClass();
  }

  return used_properties;
}

QMap<int, QSet<QString>> PropertiesSearch::getTypeToProperties() {
#define DEF_PROP(Object, ...)                                                  \
  {                                                                            \
    qMetaTypeId<Object>(),                                                     \
      (QSet<QString>({"objectName", "visible", "enabled"}) +                   \
       QSet<QString>({__VA_ARGS__}))                                           \
  }

  const auto type_to_properties = QMap<int, QSet<QString>>{
    DEF_PROP(QWidget),
    DEF_PROP(QPushButton, "text"),
    DEF_PROP(QLineEdit, "text", "placeholderText"),
    DEF_PROP(QLabel, "text"),
    DEF_PROP(QMenu, "title"),
    DEF_PROP(QCheckBox, "checked", "text"),
    DEF_PROP(QComboBox, "currentText"),
    DEF_PROP(QRadioButton, "checked", "text"),
    DEF_PROP(QAbstractSlider, "value", "maximum", "minimum"),
    DEF_PROP(QProgressBar, "value", "maximum", "minimum"),
    DEF_PROP(QTabWidget, "currentIndex", "count"),
    DEF_PROP(QTableWidget, "rowCount", "columnCount"),
    DEF_PROP(QPlainTextEdit, "plainText"),
  };

#undef DEF_PROP

  return type_to_properties;
}

/* -------------------------------- PathSearch ------------------------------ */

PathSearch::PathSearch() = default;

PathSearch::~PathSearch() = default;

bool PathSearch::matchesObjectQuery(
  const QObject *object, const QVariantMap &query) const {
  if (query.contains(path_query)) {
    return getPath(object).contains(query[path_query].toString());
  }

  return true;
}

QVariantMap PathSearch::createObjectQuery(const QObject *object) const {
  auto query = QVariantMap{};
  query[path_query] = getPath(object);

  return query;
}

QString PathSearch::getPath(const QObject *object) const {
  auto objects_path = QStringList{};

  while (object) {
    auto object_name = object->objectName();
    if (object_name.isEmpty()) {
      object_name = object->metaObject()->className();
    }

    objects_path.prepend(object_name);
    object = object->parent();
  }

  return objects_path.join(".");
}

/* ----------------------------- OrderIndexSearch --------------------------- */

OrderIndexSearch::OrderIndexSearch() = default;

OrderIndexSearch::~OrderIndexSearch() = default;

bool OrderIndexSearch::matchesObjectQuery(
  const QObject *object, const QVariantMap &query) const {
  if (query.contains(order_index_query)) {
    return getOrderIndex(object) == query[order_index_query];
  }

  return true;
}

QVariantMap OrderIndexSearch::createObjectQuery(const QObject *object) const {
  auto query = QVariantMap{};
  query[order_index_query] = getOrderIndex(object);

  return query;
}

uint OrderIndexSearch::getOrderIndex(const QObject *object) const {
  if (auto parent = object->parent(); parent) {
    return parent->children().indexOf(object);
  }

  auto topObjects = getTopLevelObjects();
  return topObjects.indexOf(object);
}

}// namespace specter
