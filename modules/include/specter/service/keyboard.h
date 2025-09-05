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

/* ----------------------------- KeyboardPressKeyCall --------------------- */

using KeyboardPressKeyCallData = CallData<
  specter_proto::KeyboardService::AsyncService, specter_proto::KeyEvent,
  google::protobuf::Empty>;

class LIB_SPECTER_API KeyboardPressKeyCall : public KeyboardPressKeyCallData {
public:
  explicit KeyboardPressKeyCall(
    specter_proto::KeyboardService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~KeyboardPressKeyCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<KeyboardPressKeyCallData> clone() const override;
};

/* --------------------------- KeyboardReleaseKeyCall --------------------- */

using KeyboardReleaseKeyCallData = CallData<
  specter_proto::KeyboardService::AsyncService, specter_proto::KeyEvent,
  google::protobuf::Empty>;

class LIB_SPECTER_API KeyboardReleaseKeyCall
    : public KeyboardReleaseKeyCallData {
public:
  explicit KeyboardReleaseKeyCall(
    specter_proto::KeyboardService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~KeyboardReleaseKeyCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<KeyboardReleaseKeyCallData> clone() const override;
};

/* ----------------------------- KeyboardTapKeyCall ----------------------- */

using KeyboardTapKeyCallData = CallData<
  specter_proto::KeyboardService::AsyncService, specter_proto::KeyEvent,
  google::protobuf::Empty>;

class LIB_SPECTER_API KeyboardTapKeyCall : public KeyboardTapKeyCallData {
public:
  explicit KeyboardTapKeyCall(
    specter_proto::KeyboardService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~KeyboardTapKeyCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<KeyboardTapKeyCallData> clone() const override;
};

/* ----------------------------- KeyboardEnterTextCall -------------------- */

using KeyboardEnterTextCallData = CallData<
  specter_proto::KeyboardService::AsyncService, specter_proto::TextInput,
  google::protobuf::Empty>;

class LIB_SPECTER_API KeyboardEnterTextCall : public KeyboardEnterTextCallData {
public:
  explicit KeyboardEnterTextCall(
    specter_proto::KeyboardService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~KeyboardEnterTextCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<KeyboardEnterTextCallData> clone() const override;
};

/* ------------------------ KeyboardEnterTextIntoObjectCall --------------- */

using KeyboardEnterTextIntoObjectCallData = CallData<
  specter_proto::KeyboardService::AsyncService, specter_proto::ObjectTextInput,
  google::protobuf::Empty>;

class LIB_SPECTER_API KeyboardEnterTextIntoObjectCall
    : public KeyboardEnterTextIntoObjectCallData {
public:
  explicit KeyboardEnterTextIntoObjectCall(
    specter_proto::KeyboardService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~KeyboardEnterTextIntoObjectCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<KeyboardEnterTextIntoObjectCallData> clone() const override;
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
