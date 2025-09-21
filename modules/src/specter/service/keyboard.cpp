/* ----------------------------------- Local -------------------------------- */
#include "specter/service/keyboard.h"

#include "specter/input/utils.h"
#include "specter/module.h"
#include "specter/search/utils.h"
#include "specter/service/utils.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ----------------------------- KeyboardPressKeyCall --------------------- */

KeyboardPressKeyCall::KeyboardPressKeyCall(
  specter_proto::KeyboardService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::KeyboardService::AsyncService::RequestPressKey) {}

KeyboardPressKeyCall::~KeyboardPressKeyCall() = default;

std::unique_ptr<KeyboardPressKeyCallData> KeyboardPressKeyCall::clone() const {
  return std::make_unique<KeyboardPressKeyCall>(getService(), getQueue());
}

KeyboardPressKeyCall::ProcessResult
KeyboardPressKeyCall::process(const Request &request) const {
  Qt::KeyboardModifiers mods = Qt::NoModifier;
  if (request.ctrl()) mods |= Qt::ControlModifier;
  if (request.alt()) mods |= Qt::AltModifier;
  if (request.shift()) mods |= Qt::ShiftModifier;
  if (request.meta()) mods |= Qt::MetaModifier;

  auto key = static_cast<Qt::Key>(request.key_code());

  auto target = getTargetWidget();
  if (target) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "Failed to press key: no active or focused widget found"),
      {}};
  }

  auto &controller = keyboardController();
  controller.pressKey(target, key, mods);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* --------------------------- KeyboardReleaseKeyCall --------------------- */

KeyboardReleaseKeyCall::KeyboardReleaseKeyCall(
  specter_proto::KeyboardService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::KeyboardService::AsyncService::RequestReleaseKey) {}

KeyboardReleaseKeyCall::~KeyboardReleaseKeyCall() = default;

std::unique_ptr<KeyboardReleaseKeyCallData>
KeyboardReleaseKeyCall::clone() const {
  return std::make_unique<KeyboardReleaseKeyCall>(getService(), getQueue());
}

KeyboardReleaseKeyCall::ProcessResult
KeyboardReleaseKeyCall::process(const Request &request) const {
  Qt::KeyboardModifiers mods = Qt::NoModifier;
  if (request.ctrl()) mods |= Qt::ControlModifier;
  if (request.alt()) mods |= Qt::AltModifier;
  if (request.shift()) mods |= Qt::ShiftModifier;
  if (request.meta()) mods |= Qt::MetaModifier;

  auto key = static_cast<Qt::Key>(request.key_code());

  auto target = getTargetWidget();
  if (target) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "Failed to release key: no active or focused widget found"),
      {}};
  }

  auto &controller = keyboardController();
  controller.releaseKey(target, key, mods);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ----------------------------- KeyboardTapKeyCall ----------------------- */

KeyboardTapKeyCall::KeyboardTapKeyCall(
  specter_proto::KeyboardService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::KeyboardService::AsyncService::RequestTapKey) {}

KeyboardTapKeyCall::~KeyboardTapKeyCall() = default;

std::unique_ptr<KeyboardTapKeyCallData> KeyboardTapKeyCall::clone() const {
  return std::make_unique<KeyboardTapKeyCall>(getService(), getQueue());
}

KeyboardTapKeyCall::ProcessResult
KeyboardTapKeyCall::process(const Request &request) const {
  Qt::KeyboardModifiers mods = Qt::NoModifier;
  if (request.ctrl()) mods |= Qt::ControlModifier;
  if (request.alt()) mods |= Qt::AltModifier;
  if (request.shift()) mods |= Qt::ShiftModifier;
  if (request.meta()) mods |= Qt::MetaModifier;

  auto key = static_cast<Qt::Key>(request.key_code());

  auto target = getTargetWidget();
  if (target) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "Failed to tap key: no active or focused widget found"),
      {}};
  }

  auto &controller = keyboardController();
  controller.tapKey(target, key, mods);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ----------------------------- KeyboardEnterTextCall -------------------- */

KeyboardEnterTextCall::KeyboardEnterTextCall(
  specter_proto::KeyboardService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::KeyboardService::AsyncService::RequestEnterText) {}

KeyboardEnterTextCall::~KeyboardEnterTextCall() = default;

std::unique_ptr<KeyboardEnterTextCallData>
KeyboardEnterTextCall::clone() const {
  return std::make_unique<KeyboardEnterTextCall>(getService(), getQueue());
}

KeyboardEnterTextCall::ProcessResult
KeyboardEnterTextCall::process(const Request &request) const {
  auto target = getTargetWidget();
  if (target) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "Failed to enter text: no active or focused widget found"),
      {}};
  }

  auto &controller = keyboardController();
  controller.enterText(target, QString::fromStdString(request.text()));

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ------------------------ KeyboardEnterTextIntoObjectCall --------------- */

KeyboardEnterTextIntoObjectCall::KeyboardEnterTextIntoObjectCall(
  specter_proto::KeyboardService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::KeyboardService::AsyncService::
          RequestEnterTextIntoObject) {}

KeyboardEnterTextIntoObjectCall::~KeyboardEnterTextIntoObjectCall() = default;

std::unique_ptr<KeyboardEnterTextIntoObjectCallData>
KeyboardEnterTextIntoObjectCall::clone() const {
  return std::make_unique<KeyboardEnterTextIntoObjectCall>(
    getService(), getQueue());
}

KeyboardEnterTextIntoObjectCall::ProcessResult
KeyboardEnterTextIntoObjectCall::process(const Request &request) const {
  const auto id =
    ObjectId::fromString(QString::fromStdString(request.object_id().id()));

  auto [status, target] = tryGetSingleWidget(id);
  if (!status.ok()) return {status, {}};

  auto &controller = keyboardController();
  controller.enterText(target, QString::fromStdString(request.text()));

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ------------------------------ KeyboardService ------------------------- */

KeyboardService::KeyboardService() = default;

KeyboardService::~KeyboardService() = default;

void KeyboardService::start(grpc::ServerCompletionQueue *queue) {
  auto press_key_call = new KeyboardPressKeyCall(this, queue);
  auto release_key_call = new KeyboardReleaseKeyCall(this, queue);
  auto enter_text_call = new KeyboardEnterTextCall(this, queue);
  auto enter_text_into_object_call =
    new KeyboardEnterTextIntoObjectCall(this, queue);

  press_key_call->proceed();
  release_key_call->proceed();
  enter_text_call->proceed();
  enter_text_into_object_call->proceed();
}

}// namespace specter
