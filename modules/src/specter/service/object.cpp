/* ----------------------------------- Local -------------------------------- */
#include "specter/service/object.h"

#include "specter/module.h"
#include "specter/observe/property/action.h"
#include "specter/observe/property/observer.h"
#include "specter/observe/tree/action.h"
#include "specter/observe/tree/observer.h"
#include "specter/search/utils.h"
#include "specter/service/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
#include <QWidget>
/* --------------------------------- Standard ------------------------------- */
#include <queue>
/* -------------------------------------------------------------------------- */

namespace specter {

/* --------------------------- TreeObservedActionsMapper ------------------------ */

class TreeObservedActionsMapper {
public:
  specter_proto::ObjectChange
  operator()(const TreeObservedAction::ObjectAdded &action) const {
    specter_proto::ObjectChange response;
    auto added = response.mutable_added();
    added->mutable_object()->set_query(action.object.toString().toStdString());
    added->mutable_parent()->set_query(action.parent.toString().toStdString());
    return response;
  }

  specter_proto::ObjectChange
  operator()(const TreeObservedAction::ObjectRemoved &action) const {
    specter_proto::ObjectChange response;
    auto removed = response.mutable_removed();
    removed->mutable_object()->set_query(
      action.object.toString().toStdString());
    return response;
  }

  specter_proto::ObjectChange
  operator()(const TreeObservedAction::ObjectReparented &action) const {
    specter_proto::ObjectChange response;
    auto reparented = response.mutable_reparented();
    reparented->mutable_object()->set_query(
      action.object.toString().toStdString());
    reparented->mutable_parent()->set_query(
      action.parent.toString().toStdString());
    return response;
  }

  specter_proto::ObjectChange
  operator()(const TreeObservedAction::ObjectRenamed &action) const {
    specter_proto::ObjectChange response;
    auto renamed = response.mutable_renamed();
    renamed->mutable_old_object()->set_query(
      action.old_object.toString().toStdString());
    renamed->mutable_new_object()->set_query(
      action.new_object.toString().toStdString());
    return response;
  }
};

/* ------------------------- PropertyObservedActionsMapper ---------------------- */

class PropertyObservedActionsMapper {
public:
  specter_proto::PropertyChange
  operator()(const PropertyObservedAction::PropertyAdded &action) const {
    specter_proto::PropertyChange response;
    auto added = response.mutable_added();
    added->set_property(action.property.toStdString());
    *added->mutable_value() = convertIntoValue(action.value);
    added->set_read_only(action.read_only);
    return response;
  }

  specter_proto::PropertyChange
  operator()(const PropertyObservedAction::PropertyRemoved &action) const {
    specter_proto::PropertyChange response;
    auto removed = response.mutable_removed();
    removed->set_property(action.property.toStdString());
    return response;
  }

  specter_proto::PropertyChange
  operator()(const PropertyObservedAction::PropertyUpdated &action) const {
    specter_proto::PropertyChange response;
    auto updated = response.mutable_updated();
    updated->set_property(action.property.toStdString());
    *updated->mutable_old_value() = convertIntoValue(action.old_value);
    *updated->mutable_new_value() = convertIntoValue(action.new_value);
    return response;
  }
};

/* ------------------------------ ObjectGetTreeCall -------------------------- */

ObjectGetTreeCall::ObjectGetTreeCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::RequestGetTree) {}

ObjectGetTreeCall::~ObjectGetTreeCall() = default;

std::unique_ptr<ObjectGetTreeCallData> ObjectGetTreeCall::clone() const {
  return std::make_unique<ObjectGetTreeCall>(getService(), getQueue());
}

ObjectGetTreeCall::ProcessResult
ObjectGetTreeCall::process(const Request &request) const {
  auto objects = QObjectList{};

  if (request.has_query()) {
    auto query =
      ObjectQuery::fromString(QString::fromStdString(request.query()));
    auto [status, object] = tryGetSingleObject(query);
    if (!status.ok()) return {status, {}};
    objects.append(object);
  } else {
    objects.append(getTopLevelObjects());
  }

  const auto response = tree(objects);
  return {grpc::Status::OK, response};
}

ObjectGetTreeCall::Response
ObjectGetTreeCall::tree(const QObjectList &objects) const {
  auto response = ObjectGetTreeCall::Response{};

  auto objectsToProcess =
    std::queue<std::pair<QObject *, specter_proto::ObjectNode *>>{};
  for (const auto object : objects) {
    objectsToProcess.push(std::make_pair(object, response.add_roots()));
  }

  while (!objectsToProcess.empty()) {
    auto objectToProcess = objectsToProcess.front();
    auto object = objectToProcess.first;
    auto object_children = objectToProcess.second;
    objectsToProcess.pop();

    const auto object_query = searcher().getQuery(object);
    object_children->mutable_object()->set_query(
      object_query.toString().toStdString());

    for (const auto child : object->children()) {
      objectsToProcess.push(
        std::make_pair(child, object_children->add_children()));
    }
  }


  return response;
}

/* ------------------------------ ObjectFindCall -------------------------- */

ObjectFindCall::ObjectFindCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::RequestFind) {}

ObjectFindCall::~ObjectFindCall() = default;

std::unique_ptr<ObjectFindCallData> ObjectFindCall::clone() const {
  return std::make_unique<ObjectFindCall>(getService(), getQueue());
}

ObjectFindCall::ProcessResult
ObjectFindCall::process(const Request &request) const {
  const auto query =
    ObjectQuery::fromString(QString::fromStdString(request.query()));

  auto [status, objects] = tryGetObjects(query);
  if (!status.ok()) return {status, {}};

  const auto response = find(objects);
  return {grpc::Status::OK, response};
}

ObjectFindCall::Response
ObjectFindCall::find(const QObjectList &objects) const {
  auto response = ObjectFindCall::Response{};
  for (const auto object : objects) {
    const auto query = searcher().getQuery(object);
    response.add_objects()->set_query(query.toString().toStdString());
  }

  return response;
}

/* ----------------------------- ObjectParentCall ------------------------- */

ObjectParentCall::ObjectParentCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::RequestGetParent) {}

ObjectParentCall::~ObjectParentCall() = default;

std::unique_ptr<ObjectParentCallData> ObjectParentCall::clone() const {
  return std::make_unique<ObjectParentCall>(getService(), getQueue());
}

ObjectParentCall::ProcessResult
ObjectParentCall::process(const Request &request) const {
  const auto query =
    ObjectQuery::fromString(QString::fromStdString(request.query()));

  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return {status, {}};

  const auto response = parent(object);
  return {grpc::Status::OK, response};
}

ObjectParentCall::Response
ObjectParentCall::parent(const QObject *object) const {
  auto response = ObjectParentCall::Response{};

  const auto parent_query =
    object->parent() ? searcher().getQuery(object->parent()) : ObjectQuery{};
  response.set_query(parent_query.toString().toStdString());


  return response;
}

/* ---------------------------- ObjectChildrenCall ------------------------ */

ObjectChildrenCall::ObjectChildrenCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::RequestGetChildren) {}

ObjectChildrenCall::~ObjectChildrenCall() = default;

std::unique_ptr<ObjectChildrenCallData> ObjectChildrenCall::clone() const {
  return std::make_unique<ObjectChildrenCall>(getService(), getQueue());
}

ObjectChildrenCall::ProcessResult
ObjectChildrenCall::process(const Request &request) const {
  const auto query =
    ObjectQuery::fromString(QString::fromStdString(request.query()));

  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return {status, {}};

  const auto response = children(object);
  return {grpc::Status::OK, response};
}

ObjectChildrenCall::Response
ObjectChildrenCall::children(const QObject *object) const {
  auto response = ObjectChildrenCall::Response{};

  for (const auto child : object->children()) {
    const auto child_query = searcher().getQuery(child);
    response.add_objects()->set_query(child_query.toString().toStdString());
  }

  return response;
}

/* --------------------------- ObjectCallMethodCall ----------------------- */

ObjectCallMethodCall::ObjectCallMethodCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::RequestCallMethod) {}

ObjectCallMethodCall::~ObjectCallMethodCall() = default;

std::unique_ptr<ObjectCallMethodCallData> ObjectCallMethodCall::clone() const {
  return std::make_unique<ObjectCallMethodCall>(getService(), getQueue());
}

ObjectCallMethodCall::ProcessResult
ObjectCallMethodCall::process(const Request &request) const {
  const auto query =
    ObjectQuery::fromString(QString::fromStdString(request.object().query()));

  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return {status, {}};

  return invoke(object, request.method(), request.arguments());
}

ObjectCallMethodCall::ProcessResult ObjectCallMethodCall::invoke(
  QObject *object, const std::string &method,
  const google::protobuf::RepeatedPtrField<google::protobuf::Value> &arguments)
  const {

  auto parameters = QVariantList{};
  for (const auto &argument : arguments) {
    parameters.push_back(convertIntoVariant(argument));
  }

  const auto meta_method = metaMethod(object, method, parameters);
  if (!meta_method.isValid()) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        QLatin1String("Method '%1' is unknown.")
          .arg(method.c_str())
          .toStdString()),
      {}};
  }

  for (auto i = 0; i < meta_method.parameterCount(); ++i) {
    const auto parameter_meta_type = meta_method.parameterMetaType(i);
    auto &parameter = parameters[i];

    parameter.convert(parameter_meta_type);
  }

  const auto _generic_arg = [&parameters](auto index) {
    return parameters.size() > index
             ? QGenericArgument(
                 parameters[index].typeName(), parameters[index].data())
             : QGenericArgument{};
  };

  meta_method.invoke(
    object, Qt::QueuedConnection, _generic_arg(0), _generic_arg(1),
    _generic_arg(2), _generic_arg(3), _generic_arg(4), _generic_arg(5),
    _generic_arg(6), _generic_arg(7), _generic_arg(8), _generic_arg(9));

  return {grpc::Status::OK, {}};
}

QMetaMethod ObjectCallMethodCall::metaMethod(
  const QObject *object, const std::string &name,
  const QVariantList &parameters) const {
  const auto meta_object = object->metaObject();

  for (auto i = 0; i < meta_object->methodCount(); ++i) {
    const auto method = meta_object->method(i);
    if (method.name() != QByteArray::fromStdString(name)) continue;
    if (method.parameterCount() != parameters.size()) continue;

    for (auto i = 0; i < method.parameterCount(); ++i) {
      const auto parameter_meta_type = method.parameterMetaType(i);
      if (!parameters[i].canConvert(parameter_meta_type)) continue;
    }

    return method;
  }

  return QMetaMethod{};
}

/* ------------------------- ObjectUpdatePropertyCall --------------------- */

ObjectUpdatePropertyCall::ObjectUpdatePropertyCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::RequestUpdateProperty) {}

ObjectUpdatePropertyCall::~ObjectUpdatePropertyCall() = default;

std::unique_ptr<ObjectUpdatePropertyCallData>
ObjectUpdatePropertyCall::clone() const {
  return std::make_unique<ObjectUpdatePropertyCall>(getService(), getQueue());
}

ObjectUpdatePropertyCall::ProcessResult
ObjectUpdatePropertyCall::process(const Request &request) const {
  const auto query =
    ObjectQuery::fromString(QString::fromStdString(request.object().query()));

  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return {status, {}};

  return setProperty(object, request.property(), request.value());
}

ObjectUpdatePropertyCall::ProcessResult ObjectUpdatePropertyCall::setProperty(
  QObject *object, const std::string &property,
  const google::protobuf::Value &value) const {

  const auto property_name = QByteArray::fromStdString(property);
  auto property_meta_type = object->property(property_name).metaType();

  if (property_meta_type.id() == QMetaType::UnknownType) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        QLatin1String("Property '%1' type is unknown.")
          .arg(property_name)
          .toStdString()),
      {}};
  }

  auto new_value = convertIntoVariant(value);
  if (!new_value.convert(property_meta_type)) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        QLatin1String("Property '%1' value '%2' is incorrect.")
          .arg(property_name)
          .arg(new_value.toString())
          .toStdString()),
      {}};
  }

  auto index = object->metaObject()->indexOfProperty(property_name);
  if (index >= 0) {
    QMetaProperty property = object->metaObject()->property(index);
    if (!property.isWritable()) {
      QString error_msg = QLatin1String(
                            "Property '%1' could not be set to '%2'. "
                            "The property may not exist or is not writable.")
                            .arg(property_name, new_value.toString());
      return {
        grpc::Status(
          grpc::StatusCode::INVALID_ARGUMENT, error_msg.toStdString()),
        {}};
    }
  }

  auto ret = object->setProperty(property_name, new_value);
  if (!ret) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        QLatin1String("Property '%1' set failed.")
          .arg(property_name)
          .toStdString()),
      {}};
  }

  return {grpc::Status::OK, {}};
}

/* --------------------------- ObjectGetMethodsCall ---------------------- */

ObjectGetMethodsCall::ObjectGetMethodsCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::RequestGetMethods) {}

ObjectGetMethodsCall::~ObjectGetMethodsCall() = default;

std::unique_ptr<ObjectGetMethodsCallData> ObjectGetMethodsCall::clone() const {
  return std::make_unique<ObjectGetMethodsCall>(getService(), getQueue());
}

ObjectGetMethodsCall::ProcessResult
ObjectGetMethodsCall::process(const Request &request) const {
  const auto query =
    ObjectQuery::fromString(QString::fromStdString(request.query()));

  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return {status, {}};

  return {grpc::Status::OK, methods(object)};
}

ObjectGetMethodsCall::Response
ObjectGetMethodsCall::methods(const QObject *object) const {
  auto response = ObjectGetMethodsCall::Response{};

  auto meta_object = object->metaObject();
  for (auto i = 0; i < meta_object->methodCount(); ++i) {
    const auto meta_method = meta_object->method(i);

    if (meta_method.access() != QMetaMethod::Access::Public) { continue; }
    if (
      meta_method.methodType() != QMetaMethod::Slot &&
      meta_method.methodType() != QMetaMethod::Method) {
      continue;
    }

    bool skip_current_method = false;
    std::vector<std::pair<std::string, QVariant>> collected_parameters_data;

    for (int j = 0; j < meta_method.parameterCount(); ++j) {
      auto parameter_name = meta_method.parameterNames().at(j).toStdString();
      if (parameter_name.empty()) {
        parameter_name = QString("arg%1").arg(QString::number(j)).toStdString();
      }

      auto parameter_default_value_qvariant =
        QVariant(QMetaType(meta_method.parameterType(j)));
      if (!parameter_default_value_qvariant.isValid()) {
        skip_current_method = true;
        break;
      }

      collected_parameters_data.emplace_back(
        parameter_name, parameter_default_value_qvariant);
    }

    if (skip_current_method) { continue; }
    auto proto_method = response.add_methods();
    proto_method->set_name(QString(meta_method.name()).toStdString());

    for (const auto &param_data : collected_parameters_data) {
      auto proto_parameter = proto_method->add_parameters();
      proto_parameter->set_name(param_data.first);
      *proto_parameter->mutable_default_value() =
        convertIntoValue(param_data.second);
    }
  }

  return response;
}

/* -------------------------- ObjectGetPropertiesCall ---------------------- */

ObjectGetPropertiesCall::ObjectGetPropertiesCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::RequestGetProperties) {}

ObjectGetPropertiesCall::~ObjectGetPropertiesCall() = default;

std::unique_ptr<ObjectGetPropertiesCallData>
ObjectGetPropertiesCall::clone() const {
  return std::make_unique<ObjectGetPropertiesCall>(getService(), getQueue());
}

ObjectGetPropertiesCall::ProcessResult
ObjectGetPropertiesCall::process(const Request &request) const {
  const auto query =
    ObjectQuery::fromString(QString::fromStdString(request.query()));

  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return {status, {}};

  return {grpc::Status::OK, properties(object)};
}

ObjectGetPropertiesCall::Response
ObjectGetPropertiesCall::properties(const QObject *object) const {
  auto response = ObjectGetPropertiesCall::Response{};

  auto unique_properties = std::set<std::string>{};
  auto meta_object = object->metaObject();
  for (auto i = 0; i < meta_object->propertyCount(); ++i) {
    const auto name = meta_object->property(i).name();
    const auto read_only = !meta_object->property(i).isWritable();
    unique_properties.insert(name);
  }

  for (auto unique_property : unique_properties) {
    const auto value = object->property(unique_property.c_str());

    auto property_index =
      object->metaObject()->indexOfProperty(unique_property.c_str());
    auto read_only =
      !object->metaObject()->property(property_index).isWritable();

    auto new_properties = response.add_properties();
    new_properties->set_name(unique_property);
    *new_properties->mutable_value() = convertIntoValue(value);
    new_properties->set_read_only(read_only);
  }

  return response;
}

/* ------------------------ ObjectListenTreeChangesCall ----------------- */

ObjectListenTreeChangesCall::ObjectListenTreeChangesCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : StreamCallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::RequestListenTreeChanges),
      m_observer(std::make_unique<TreeObserver>()),
      m_observer_queue(std::make_unique<TreeObserverQueue>()),
      m_mapper(std::make_unique<TreeObservedActionsMapper>()) {

  m_observer_queue->setObserver(m_observer.get());
}

ObjectListenTreeChangesCall::~ObjectListenTreeChangesCall() = default;

ObjectListenTreeChangesCall::ProcessResult
ObjectListenTreeChangesCall::process(const Request &request) const {
  if (!m_observer->isObserving()) { m_observer->start(); }
  if (m_observer_queue->isEmpty()) return {};

  const auto observer_action = m_observer_queue->popAction();
  const auto response = observer_action.visit(*m_mapper);

  return response;
}

std::unique_ptr<ObjectListenTreeChangesCallData>
ObjectListenTreeChangesCall::clone() const {
  return std::make_unique<ObjectListenTreeChangesCall>(
    getService(), getQueue());
}

/* ----------------------- ObjectListenPropertyChangesCall ---------------- */

ObjectListenPropertyChangesCall::ObjectListenPropertyChangesCall(
  specter_proto::ObjectService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : StreamCallData(
        service, queue, CallTag{this},
        &specter_proto::ObjectService::AsyncService::
          RequestListenPropertiesChanges),
      m_observer(std::make_unique<PropertyObserver>()),
      m_observer_queue(std::make_unique<PropertyObserverQueue>()),
      m_mapper(std::make_unique<PropertyObservedActionsMapper>()) {

  m_observer_queue->setObserver(m_observer.get());
}

ObjectListenPropertyChangesCall::~ObjectListenPropertyChangesCall() = default;

ObjectListenPropertyChangesCall::ProcessResult
ObjectListenPropertyChangesCall::process(const Request &request) const {
  const auto query =
    ObjectQuery::fromString(QString::fromStdString(request.query()));

  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return status;

  if (!m_observer->isObserving()) { m_observer->start(); }
  if (m_observer->getObject() != object) { m_observer->setObject(object); }

  if (m_observer_queue->isEmpty()) return {};

  const auto observer_action = m_observer_queue->popAction();
  const auto response = observer_action.visit(*m_mapper);

  return response;
}

std::unique_ptr<ObjectListenPropertyChangesCallData>
ObjectListenPropertyChangesCall::clone() const {
  return std::make_unique<ObjectListenPropertyChangesCall>(
    getService(), getQueue());
}

/* ------------------------------ ObjectService --------------------------- */

ObjectService::ObjectService() = default;

ObjectService::~ObjectService() = default;

void ObjectService::start(grpc::ServerCompletionQueue *queue) {
  auto get_tree_call = new ObjectGetTreeCall(this, queue);
  auto find_call = new ObjectFindCall(this, queue);
  auto parent_call = new ObjectParentCall(this, queue);
  auto children_call = new ObjectChildrenCall(this, queue);
  auto call_method_call = new ObjectCallMethodCall(this, queue);
  auto update_property_call = new ObjectUpdatePropertyCall(this, queue);
  auto get_methods_call = new ObjectGetMethodsCall(this, queue);
  auto get_properties_call = new ObjectGetPropertiesCall(this, queue);
  auto listen_tree_changes = new ObjectListenTreeChangesCall(this, queue);
  auto listen_property_changes =
    new ObjectListenPropertyChangesCall(this, queue);

  get_tree_call->proceed();
  find_call->proceed();
  parent_call->proceed();
  children_call->proceed();
  call_method_call->proceed();
  update_property_call->proceed();
  get_methods_call->proceed();
  get_properties_call->proceed();
  listen_tree_changes->proceed();
  listen_property_changes->proceed();
}

}// namespace specter
