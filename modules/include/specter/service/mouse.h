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

/* --------------------------- MousePressButtonCall ----------------------- */

using MousePressButtonCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::MouseEvent,
  google::protobuf::Empty>;

class LIB_SPECTER_API MousePressButtonCall : public MousePressButtonCallData {
public:
  explicit MousePressButtonCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MousePressButtonCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MousePressButtonCallData> clone() const override;
};

/* -------------------------- MouseReleaseButtonCall ---------------------- */

using MouseReleaseButtonCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::MouseEvent,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseReleaseButtonCall
    : public MouseReleaseButtonCallData {
public:
  explicit MouseReleaseButtonCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseReleaseButtonCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseReleaseButtonCallData> clone() const override;
};

/* --------------------------- MouseClickButtonCall ----------------------- */

using MouseClickButtonCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::MouseEvent,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseClickButtonCall : public MouseClickButtonCallData {
public:
  explicit MouseClickButtonCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseClickButtonCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseClickButtonCallData> clone() const override;
};

/* --------------------------- MouseMoveCursorCall ------------------------ */

using MouseMoveCursorCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::CursorMove,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseMoveCursorCall : public MouseMoveCursorCallData {
public:
  explicit MouseMoveCursorCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseMoveCursorCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseMoveCursorCallData> clone() const override;
};

/* --------------------------- MouseScrollWheelCall ----------------------- */

using MouseScrollWheelCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::WheelScroll,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseScrollWheelCall : public MouseScrollWheelCallData {
public:
  explicit MouseScrollWheelCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseScrollWheelCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseScrollWheelCallData> clone() const override;
};

/* ---------------------------- MouseClickOnObject ------------------------ */

using MouseClickOnObjectData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::ObjectClick,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseClickOnObject : public MouseClickOnObjectData {
public:
  explicit MouseClickOnObject(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseClickOnObject() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseClickOnObjectData> clone() const override;
};

/* ------------------------- MouseHoverOverObjectCall --------------------- */

using MouseHoverOverObjectCallData = CallData<
  specter_proto::MouseService::AsyncService, specter_proto::ObjectHover,
  google::protobuf::Empty>;

class LIB_SPECTER_API MouseHoverOverObjectCall
    : public MouseHoverOverObjectCallData {
public:
  explicit MouseHoverOverObjectCall(
    specter_proto::MouseService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MouseHoverOverObjectCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MouseHoverOverObjectCallData> clone() const override;
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
