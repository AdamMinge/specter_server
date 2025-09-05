#ifndef SPECTER_SERVICE_MOUSE_H
#define SPECTER_SERVICE_MOUSE_H

/* ----------------------------------- Proto -------------------------------- */
#include <specter_proto/specter.grpc.pb.h>
#include <specter_proto/specter.pb.h>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/server/call.h"
#include "specter/server/service.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ MouseClickCall -------------------------- */

using MouseClickCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::MouseClick,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseClickCall : public MouseClickCallData {
public:
  explicit MouseClickCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseClickCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseClickCallData> clone() const override;
};

/* ------------------------------ MouseMoveCall --------------------------- */

using MouseMoveCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::MouseMove,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseMoveCall : public MouseMoveCallData {
public:
  explicit MouseMoveCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseMoveCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseMoveCallData> clone() const override;
};

/* ------------------------------ MouseScrollCall ------------------------- */

using MouseScrollCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::MouseScroll,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseScrollCall : public MouseScrollCallData {
public:
  explicit MouseScrollCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseScrollCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseScrollCallData> clone() const override;
};

/* ---------------------------- MouseClickObjectCall ---------------------- */

using MouseClickObjectCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::ObjectMouseClick,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseClickObjectCall : public MouseClickObjectCallData {
public:
  explicit MouseClickObjectCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseClickObjectCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseClickObjectCallData> clone() const override;
};

/* --------------------------- MouseMoveOverObjectCall -------------------- */

using MouseMoveOverObjectCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::ObjectMouseMove,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseMoveOverObjectCall
    : public MouseMoveOverObjectCallData {
public:
  explicit MouseMoveOverObjectCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseMoveOverObjectCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseMoveOverObjectCallData> clone() const override;
};

/* ------------------------------- MouseService --------------------------- */

class MouseService
    : public ServiceWrapper<specter_proto::MouseService::AsyncService> {
public:
  explicit MouseService();
  ~MouseService() override;

  void start(grpc::ServerCompletionQueue *queue) override;
};

}// namespace specter

#endif// SPECTER_SERVICE_MOUSE_H
