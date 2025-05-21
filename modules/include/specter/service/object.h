#ifndef SPECTER_SERVICE_OBJECT_H
#define SPECTER_SERVICE_OBJECT_H

/* ----------------------------------- Proto -------------------------------- */
#include <specter_proto/specter.grpc.pb.h>
#include <specter_proto/specter.pb.h>
/* ------------------------------------ Qt ---------------------------------- */
#include <QMetaMethod>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/server/call.h"
#include "specter/server/service.h"
/* -------------------------------------------------------------------------- */

namespace specter {

class ObjectQuery;
class ObjectObserver;
class ObjectObserverQueue;
class ObservedActionsMapper;

/* ------------------------------- ObjectGetTreeCall ------------------------ */

using ObjectGetTreeCallData = CallData<
  specter_proto::ObjectService::AsyncService, specter_proto::OptionalObject,
  specter_proto::ObjectTree>;

class LIB_SPECTER_API ObjectGetTreeCall : public ObjectGetTreeCallData {
public:
  explicit ObjectGetTreeCall(
    specter_proto::ObjectService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~ObjectGetTreeCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<ObjectGetTreeCallData> clone() const override;

private:
  [[nodiscard]] Response tree(const QObjectList &objects) const;
};

/* -------------------------------- ObjectFindCall -------------------------- */

using ObjectFindCallData = CallData<
  specter_proto::ObjectService::AsyncService, specter_proto::Object,
  specter_proto::Objects>;

class LIB_SPECTER_API ObjectFindCall : public ObjectFindCallData {
public:
  explicit ObjectFindCall(
    specter_proto::ObjectService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~ObjectFindCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<ObjectFindCallData> clone() const override;

private:
  [[nodiscard]] Response find(const QObjectList &objects) const;
};

/* ------------------------------ ObjectParentCall ------------------------ */

using ObjectParentCallData = CallData<
  specter_proto::ObjectService::AsyncService, specter_proto::Object,
  specter_proto::Object>;

class LIB_SPECTER_API ObjectParentCall : public ObjectParentCallData {
public:
  explicit ObjectParentCall(
    specter_proto::ObjectService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~ObjectParentCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<ObjectParentCallData> clone() const override;

private:
  [[nodiscard]] Response parent(const QObject *object) const;
};

/* ----------------------------- ObjectChildrenCall ----------------------- */

using ObjectChildrenCallData = CallData<
  specter_proto::ObjectService::AsyncService, specter_proto::Object,
  specter_proto::Objects>;

class LIB_SPECTER_API ObjectChildrenCall : public ObjectChildrenCallData {
public:
  explicit ObjectChildrenCall(
    specter_proto::ObjectService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~ObjectChildrenCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<ObjectChildrenCallData> clone() const override;

private:
  [[nodiscard]] Response children(const QObject *object) const;
};

/* ---------------------------- ObjectCallMethodCall ---------------------- */

using ObjectCallMethodCallData = CallData<
  specter_proto::ObjectService::AsyncService, specter_proto::MethodCall,
  google::protobuf::Empty>;

class LIB_SPECTER_API ObjectCallMethodCall : public ObjectCallMethodCallData {
public:
  explicit ObjectCallMethodCall(
    specter_proto::ObjectService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~ObjectCallMethodCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<ObjectCallMethodCallData> clone() const override;

private:
  [[nodiscard]] ProcessResult invoke(
    QObject *object, const std::string &method,
    const google::protobuf::RepeatedPtrField<google::protobuf::Value>
      &arguments) const;
  [[nodiscard]] QMetaMethod
  metaMethod(const QObject *object, const std::string &name) const;
};

/* -------------------------- ObjectUpdatePropertyCall -------------------- */

using ObjectUpdatePropertyCallData = CallData<
  specter_proto::ObjectService::AsyncService, specter_proto::PropertyUpdate,
  google::protobuf::Empty>;

class LIB_SPECTER_API ObjectUpdatePropertyCall
    : public ObjectUpdatePropertyCallData {
public:
  explicit ObjectUpdatePropertyCall(
    specter_proto::ObjectService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~ObjectUpdatePropertyCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<ObjectUpdatePropertyCallData> clone() const override;

private:
  [[nodiscard]] ProcessResult setProperty(
    QObject *object, const std::string &property,
    const google::protobuf::Value &value) const;
};

/* ---------------------------- ObjectGetMethodsCall ---------------------- */

using ObjectGetMethodsCallData = CallData<
  specter_proto::ObjectService::AsyncService, specter_proto::Object,
  specter_proto::Methods>;

class LIB_SPECTER_API ObjectGetMethodsCall : public ObjectGetMethodsCallData {
public:
  explicit ObjectGetMethodsCall(
    specter_proto::ObjectService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~ObjectGetMethodsCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<ObjectGetMethodsCallData> clone() const override;

private:
  [[nodiscard]] Response methods(const QObject *object) const;
};

/* --------------------------- ObjectGetPropertiesCall -------------------- */

using ObjectGetPropertiesCallData = CallData<
  specter_proto::ObjectService::AsyncService, specter_proto::Object,
  specter_proto::Properties>;

class LIB_SPECTER_API ObjectGetPropertiesCall
    : public ObjectGetPropertiesCallData {
public:
  explicit ObjectGetPropertiesCall(
    specter_proto::ObjectService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~ObjectGetPropertiesCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<ObjectGetPropertiesCallData> clone() const override;

private:
  [[nodiscard]] Response properties(const QObject *object) const;
};

/* ------------------------- ObjectListenObjectChangesCall ------------------ */

using ObjectListenObjectChangesCallData = StreamCallData<
  specter_proto::ObjectService::AsyncService, google::protobuf::Empty,
  specter_proto::ObjectChange>;

class LIB_SPECTER_API ObjectListenObjectChangesCall
    : public ObjectListenObjectChangesCallData {
public:
  explicit ObjectListenObjectChangesCall(
    specter_proto::ObjectService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~ObjectListenObjectChangesCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<ObjectListenObjectChangesCallData> clone() const override;

private:
  std::unique_ptr<ObjectObserver> m_observer;
  std::unique_ptr<ObjectObserverQueue> m_observer_queue;
  std::unique_ptr<ObservedActionsMapper> m_mapper;
};

/* ------------------------------- ObjectService -------------------------- */

class ObjectService
    : public ServiceWrapper<specter_proto::ObjectService::AsyncService> {
public:
  explicit ObjectService();
  ~ObjectService() override;

  void start(grpc::ServerCompletionQueue *queue) override;
};

}// namespace specter

#endif// SPECTER_SERVICE_OBJECT_H
