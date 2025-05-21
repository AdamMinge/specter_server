/* ----------------------------------- Local -------------------------------- */
#include "specter/service/object.h"

#include "specter/module.h"
/* -------------------------------------------------------------------------- */

namespace specter {

QVariant convertIntoVariant(const google::protobuf::Value &value) {
  switch (value.kind_case()) {
    case google::protobuf::Value::kStringValue:
      return QVariant::fromValue(QString::fromStdString(value.string_value()));

    case google::protobuf::Value::kNumberValue:
      return QVariant::fromValue(value.number_value());

    case google::protobuf::Value::kBoolValue:
      return QVariant::fromValue(value.bool_value());

    default:
      return QVariant{};
  }
}

google::protobuf::Value convertIntoValue(const QVariant &variant) {
  auto value = google::protobuf::Value{};

  switch (variant.typeId()) {
    case QMetaType::Type::QString:
      value.set_string_value(variant.value<QString>().toStdString());
      break;

    case QMetaType::Type::Int:
      value.set_number_value(variant.toInt());
      break;

    case QMetaType::Type::Double:
      value.set_number_value(variant.toDouble());
      break;

    case QMetaType::Type::Bool:
      value.set_bool_value(variant.toBool());
      break;
  }

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

}// namespace specter
