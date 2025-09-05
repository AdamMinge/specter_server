/* ----------------------------------- Local -------------------------------- */
#include "specter/service/utils.h"

#include "specter/module.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QColor>
#include <QDateTime>
#include <QMetaObject>
#include <QMetaProperty>
#include <QPoint>
#include <QRect>
/* -------------------------------------------------------------------------- */

namespace {

google::protobuf::Value convertQtBaseTypeIntoValue(const QVariant &variant) {
  google::protobuf::Value value;

  int typeId = variant.typeId();

  switch (typeId) {
    case QMetaType::QString:
      value.set_string_value(variant.toString().toStdString());
      return value;

    case QMetaType::Bool:
      value.set_bool_value(variant.toBool());
      return value;

    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Double:
    case QMetaType::Float:
      value.set_number_value(variant.toDouble());
      return value;
  }

  if (variant.canConvert<QRect>()) {
    QRect r = variant.toRect();
    auto s = value.mutable_struct_value();
    (*s->mutable_fields())["type"].set_string_value("QRect");
    (*s->mutable_fields())["x"].set_number_value(r.x());
    (*s->mutable_fields())["y"].set_number_value(r.y());
    (*s->mutable_fields())["width"].set_number_value(r.width());
    (*s->mutable_fields())["height"].set_number_value(r.height());
    return value;
  }

  if (variant.canConvert<QPoint>()) {
    QPoint p = variant.toPoint();
    auto s = value.mutable_struct_value();
    (*s->mutable_fields())["type"].set_string_value("QPoint");
    (*s->mutable_fields())["x"].set_number_value(p.x());
    (*s->mutable_fields())["y"].set_number_value(p.y());
    return value;
  }

  if (variant.canConvert<QSize>()) {
    QSize sz = variant.toSize();
    auto s = value.mutable_struct_value();
    (*s->mutable_fields())["type"].set_string_value("QSize");
    (*s->mutable_fields())["width"].set_number_value(sz.width());
    (*s->mutable_fields())["height"].set_number_value(sz.height());
    return value;
  }

  if (variant.canConvert<QDateTime>()) {
    QDateTime dt = variant.toDateTime();
    auto s = value.mutable_struct_value();
    (*s->mutable_fields())["type"].set_string_value("QDateTime");
    (*s->mutable_fields())["iso"].set_string_value(
      dt.toString(Qt::ISODate).toStdString());
    return value;
  }

  if (variant.canConvert<QDate>()) {
    QDate d = variant.toDate();
    auto s = value.mutable_struct_value();
    (*s->mutable_fields())["type"].set_string_value("QDate");
    (*s->mutable_fields())["iso"].set_string_value(
      d.toString(Qt::ISODate).toStdString());
    return value;
  }

  if (variant.canConvert<QTime>()) {
    QTime t = variant.toTime();
    auto s = value.mutable_struct_value();
    (*s->mutable_fields())["type"].set_string_value("QTime");
    (*s->mutable_fields())["iso"].set_string_value(
      t.toString(Qt::ISODate).toStdString());
    return value;
  }

  if (variant.canConvert<QColor>()) {
    QColor c = variant.value<QColor>();
    auto s = value.mutable_struct_value();
    (*s->mutable_fields())["type"].set_string_value("QColor");
    (*s->mutable_fields())["rgba"].set_string_value(
      c.name(QColor::HexArgb).toStdString());
    return value;
  }

  if (typeId == QMetaType::QVariantList) {
    auto list = variant.toList();
    auto list_value = value.mutable_list_value();
    for (const auto &item : list)
      *list_value->add_values() = specter::convertIntoValue(item);
    return value;
  }

  if (typeId == QMetaType::QVariantMap) {
    auto map = variant.toMap();
    auto struct_value = value.mutable_struct_value();
    for (auto it = map.begin(); it != map.end(); ++it)
      (*struct_value->mutable_fields())[it.key().toStdString()] =
        specter::convertIntoValue(it.value());
    return value;
  }

  return value;
}

google::protobuf::Value convertQtObjectTypeIntoValue(const QVariant &variant) {
  google::protobuf::Value value;

  if (variant.canConvert<QObject *>()) {
    QObject *obj = variant.value<QObject *>();
    if (obj) {
      const QMetaObject *mo = obj->metaObject();
      auto struct_value = value.mutable_struct_value();
      (*struct_value->mutable_fields())["type"] =
        specter::convertIntoValue(QString(mo->className()));

      for (int i = 0; i < mo->propertyCount(); ++i) {
        QMetaProperty prop = mo->property(i);
        if (prop.isReadable()) {
          QString name = prop.name();
          QVariant propValue = prop.read(obj);
          (*struct_value->mutable_fields())[name.toStdString()] =
            specter::convertIntoValue(propValue);
        }
      }
      return value;
    }
  }

  return value;
}

google::protobuf::Value convertQtGadgetTypeIntoValue(const QVariant &variant) {
  google::protobuf::Value value;

  int typeId = variant.typeId();
  const QMetaType metaType(typeId);

  if (metaType.flags().testFlag(QMetaType::IsGadget)) {
    const QMetaObject *mo = QMetaType(typeId).metaObject();
    if (mo) {
      void *ptr = const_cast<void *>(variant.constData());
      auto struct_value = value.mutable_struct_value();
      (*struct_value->mutable_fields())["type"] =
        specter::convertIntoValue(QString(mo->className()));

      for (int i = 0; i < mo->propertyCount(); ++i) {
        QMetaProperty prop = mo->property(i);
        if (prop.isReadable()) {
          QVariant propValue = prop.readOnGadget(ptr);
          (*struct_value->mutable_fields())[prop.name()] =
            specter::convertIntoValue(propValue);
        }
      }
      return value;
    }
  }

  return value;
}

QVariant convertBaseTypeIntoVariant(const google::protobuf::Value &value) {
  using KindCase = google::protobuf::Value::KindCase;

  switch (value.kind_case()) {
    case KindCase::kStringValue:
      return QVariant::fromValue(QString::fromStdString(value.string_value()));

    case KindCase::kNumberValue:
      return QVariant::fromValue(value.number_value());

    case KindCase::kBoolValue:
      return QVariant::fromValue(value.bool_value());

    case KindCase::kListValue: {
      QVariantList list;
      for (const auto &v : value.list_value().values())
        list.append(specter::convertIntoVariant(v));
      return list;
    }
  }

  return {};
}

QVariant convertStructTypeIntoVariant(const google::protobuf::Value &value) {
  using KindCase = google::protobuf::Value::KindCase;

  switch (value.kind_case()) {
    case KindCase::kStructValue: {
      const auto &fields = value.struct_value().fields();

      auto typeField = fields.find("type");
      QString typeName;
      if (
        typeField != fields.end() &&
        typeField->second.kind_case() == KindCase::kStringValue) {
        typeName = QString::fromStdString(typeField->second.string_value());
      }

      if (
        typeField != fields.end() &&
        typeField->second.kind_case() == KindCase::kStringValue) {
        typeName = QString::fromStdString(typeField->second.string_value());
      }

      if (typeName == "QRect") {
        return QRect(
          int(fields.at("x").number_value()),
          int(fields.at("y").number_value()),
          int(fields.at("width").number_value()),
          int(fields.at("height").number_value()));
      }

      if (typeName == "QPoint") {
        return QPoint(
          int(fields.at("x").number_value()),
          int(fields.at("y").number_value()));
      }

      if (typeName == "QSize") {
        return QSize(
          int(fields.at("width").number_value()),
          int(fields.at("height").number_value()));
      }

      if (typeName == "QDateTime") {
        return QDateTime::fromString(
          QString::fromStdString(fields.at("iso").string_value()), Qt::ISODate);
      }

      if (typeName == "QDate") {
        return QDate::fromString(
          QString::fromStdString(fields.at("iso").string_value()), Qt::ISODate);
      }

      if (typeName == "QTime") {
        return QTime::fromString(
          QString::fromStdString(fields.at("iso").string_value()), Qt::ISODate);
      }

      if (typeName == "QColor") {
        return QColor(QString::fromStdString(fields.at("rgba").string_value()));
      }

      if (!typeName.isEmpty()) {
        int typeId = QMetaType::fromName(typeName.toUtf8().constData()).id();
        QMetaType metaType(typeId);

        if (metaType.flags().testFlag(QMetaType::IsGadget)) {
          void *ptr = metaType.create();
          const QMetaObject *mo = metaType.metaObject();

          for (int i = 0; i < mo->propertyCount(); ++i) {
            QMetaProperty prop = mo->property(i);
            auto it = fields.find(prop.name());
            if (it != fields.end()) {
              QVariant propValue = specter::convertIntoVariant(it->second);
              prop.writeOnGadget(ptr, propValue);
            }
          }

          QVariant gadgetVariant(QMetaType(typeId), ptr);
          metaType.destroy(ptr);
          return gadgetVariant;
        }
      }

      QVariantMap map;
      for (const auto &pair : fields) {
        if (pair.first != "type")
          map[QString::fromStdString(pair.first)] =
            specter::convertIntoVariant(pair.second);
      }
      return map;
    }
  }

  return {};
}

}// namespace

namespace specter {

QVariant convertIntoVariant(const google::protobuf::Value &value) {
  auto variant = QVariant{};

  if (variant = convertBaseTypeIntoVariant(value); !variant.isNull())
    return variant;
  if (variant = convertStructTypeIntoVariant(value); !variant.isNull())
    return variant;

  return variant;
}

google::protobuf::Value convertIntoValue(const QVariant &variant) {
  auto value = google::protobuf::Value{};

  if (value = convertQtBaseTypeIntoValue(variant); !value.null_value())
    return value;
  if (value = convertQtObjectTypeIntoValue(variant); !value.null_value())
    return value;
  if (value = convertQtGadgetTypeIntoValue(variant); !value.null_value())
    return value;

  return value;
}

std::pair<grpc::Status, QObjectList> tryGetObjects(const ObjectQuery &query) {
  return {grpc::Status::OK, searcher().getObjects(query)};
}

std::pair<grpc::Status, QObject *>
tryGetSingleObject(const ObjectQuery &query) {
  auto [status, objects] = tryGetObjects(query);
  if (!status.ok()) return {status, nullptr};

  if (objects.empty()) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "There is not object for passed query"),
      nullptr};
  }

  if (objects.size() > 1) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "There is more then single object for passed query"),
      nullptr};
  }

  return {grpc::Status::OK, objects[0]};
}

std::pair<grpc::Status, QObject *> tryGetSingleObject(const ObjectId &id) {
  auto object = searcher().getObject(id);

  if (!object) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "There is not object for passed id"),
      nullptr};
  }

  return {grpc::Status::OK, object};
}

std::pair<grpc::Status, QWidget *>
tryGetSingleWidget(const ObjectQuery &query) {
  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return {status, nullptr};

  auto widget = qobject_cast<QWidget *>(object);
  if (!widget) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "There is not widget for passed query"),
      nullptr};
  }

  return {grpc::Status::OK, widget};
}

std::pair<grpc::Status, QWidget *> tryGetSingleWidget(const ObjectId &query) {
  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return {status, nullptr};

  auto widget = qobject_cast<QWidget *>(object);
  if (!widget) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "There is not widget for passed id"),
      nullptr};
  }

  return {grpc::Status::OK, widget};
}

QPoint resolvePosition(QWidget *widget, const specter_proto::Offset &offset) {
  return QPoint(offset.x(), offset.y());
}

QPoint resolvePosition(QWidget *widget, const specter_proto::Anchor &anchor) {
  QRect r = widget->rect();
  switch (anchor) {
    case specter_proto::CENTER:
      return r.center();
    case specter_proto::LEFT_UP:
      return r.topLeft();
    case specter_proto::RIGHT_UP:
      return r.topRight();
    case specter_proto::LEFT_BOTTOM:
      return r.bottomLeft();
    case specter_proto::RIGHT_BOTTOM:
      return r.bottomRight();
  }

  return QPoint{};
}

}// namespace specter
