#ifndef SPECTER_SERVICE_KEYBOARD_H
#define SPECTER_SERVICE_KEYBOARD_H

/* ----------------------------------- Proto -------------------------------- */
#include <specter_proto/specter.grpc.pb.h>
#include <specter_proto/specter.pb.h>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/server/call.h"
#include "specter/server/service.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ----------------------------- KeyboardKeyPressCall --------------------- */

using KeyboardKeyPressCallData = CallData<
  specter_proto::KeyboardService::AsyncService, specter_proto::KeyEvent,
  google::protobuf::Empty>;

class LIB_SPECTER_API KeyboardKeyPressCall : public KeyboardKeyPressCallData {
public:
  explicit KeyboardKeyPressCall(
    specter_proto::KeyboardService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~KeyboardKeyPressCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<KeyboardKeyPressCallData> clone() const override;
};

/* --------------------------- KeyboardKeyReleaseCall --------------------- */

using KeyboardKeyReleaseCallData = CallData<
  specter_proto::KeyboardService::AsyncService, specter_proto::KeyEvent,
  google::protobuf::Empty>;

class LIB_SPECTER_API KeyboardKeyReleaseCall
    : public KeyboardKeyReleaseCallData {
public:
  explicit KeyboardKeyReleaseCall(
    specter_proto::KeyboardService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~KeyboardKeyReleaseCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<KeyboardKeyReleaseCallData> clone() const override;
};

/* ----------------------------- KeyboardTypeTextCall --------------------- */

using KeyboardTypeTextCallData = CallData<
  specter_proto::KeyboardService::AsyncService, specter_proto::TextInput,
  google::protobuf::Empty>;

class LIB_SPECTER_API KeyboardTypeTextCall : public KeyboardTypeTextCallData {
public:
  explicit KeyboardTypeTextCall(
    specter_proto::KeyboardService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~KeyboardTypeTextCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<KeyboardTypeTextCallData> clone() const override;
};

/* -------------------------- KeyboardTypeIntoObjectCall ------------------ */

using KeyboardTypeIntoObjectCallData = CallData<
  specter_proto::KeyboardService::AsyncService, specter_proto::ObjectTextInput,
  google::protobuf::Empty>;

class LIB_SPECTER_API KeyboardTypeIntoObjectCall
    : public KeyboardTypeIntoObjectCallData {
public:
  explicit KeyboardTypeIntoObjectCall(
    specter_proto::KeyboardService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~KeyboardTypeIntoObjectCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<KeyboardTypeIntoObjectCallData> clone() const override;
};

/* ------------------------------ KeyboardService ------------------------- */

class KeyboardService
    : public ServiceWrapper<specter_proto::KeyboardService::AsyncService> {
public:
  explicit KeyboardService();
  ~KeyboardService() override;

  void start(grpc::ServerCompletionQueue *queue) override;
};

}// namespace specter

#endif// SPECTER_SERVICE_KEYBOARD_H
