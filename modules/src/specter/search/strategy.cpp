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

#include <QMetaObject>
/* -------------------------------------------------------------------------- */

namespace {

bool regexCompare(const QVariant &variant, const QString &subject) {
  if (!variant.canConvert<QString>()) return false;

  QRegularExpression re(variant.toString());
  if (!re.isValid()) { return false; }

  return re.match(subject).hasMatch();
}

}// namespace

namespace specter {

/* ------------------------------ SearchStrategy ---------------------------- */

SearchStrategy::SearchStrategy(Kind kind) : m_kind(kind) {}

SearchStrategy::~SearchStrategy() = default;

SearchStrategy::Kind SearchStrategy::kind() const { return m_kind; }

/* -------------------------------- TypeSearch ------------------------------ */

TypeSearch::TypeSearch() : SearchStrategy(Kind::Type) {}

TypeSearch::~TypeSearch() = default;

bool TypeSearch::matchesObjectQuery(
  const QObject *object, const QVariantMap &query) const {
  if (query.contains(type_query)) {
    return regexCompare(query[type_query], object->metaObject()->className());
  }

  return true;
}

QVariantMap TypeSearch::createObjectQuery(const QObject *object) const {
  auto query = QVariantMap{};
  query[type_query] = object->metaObject()->className();

  return query;
}

/* ----------------------------- PropertiesSearch --------------------------- */

PropertiesSearch::PropertiesSearch() : SearchStrategy(Kind::Properties) {}

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

PathSearch::PathSearch() : SearchStrategy(Kind::Path) {}

PathSearch::~PathSearch() = default;

bool PathSearch::matchesObjectQuery(
  const QObject *object, const QVariantMap &query) const {
  if (query.contains(path_query)) {
    return regexCompare(query[path_query].toString(), getPath(object));
  }

  return true;
}

QVariantMap PathSearch::createObjectQuery(const QObject *object) const {
  auto query = QVariantMap{};
  query[path_query] = getPath(object);

  return query;
}

QString PathSearch::getPath(const QObject *object) const {
  auto current_object = object;
  auto objects_path = QStringList{};

  while (current_object) {
    auto object_name = current_object->objectName();
    if (object_name.isEmpty()) {
      object_name = current_object->metaObject()->className();
    }

    objects_path.prepend(object_name);
    current_object = current_object->parent();
  }

  return objects_path.join("/");
}

/* ----------------------------- OrderIndexSearch --------------------------- */

OrderIndexSearch::OrderIndexSearch() : SearchStrategy(Kind::OrderIndex) {}

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
