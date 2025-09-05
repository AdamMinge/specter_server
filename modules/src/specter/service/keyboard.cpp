/* ----------------------------------- Local -------------------------------- */
#include "specter/service/keyboard.h"

#include "specter/module.h"
#include "specter/search/utils.h"
#include "specter/service/utils.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ----------------------------- KeyboardKeyPressCall --------------------- */

KeyboardKeyPressCall::KeyboardKeyPressCall(
  specter_proto::KeyboardService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::KeyboardService::AsyncService::RequestKeyPress) {}

KeyboardKeyPressCall::~KeyboardKeyPressCall() = default;

std::unique_ptr<KeyboardKeyPressCallData> KeyboardKeyPressCall::clone() const {
  return std::make_unique<KeyboardKeyPressCall>(getService(), getQueue());
}

KeyboardKeyPressCall::ProcessResult
KeyboardKeyPressCall::process(const Request &request) const {
  Qt::KeyboardModifiers mods = Qt::NoModifier;
  if (request.ctrl()) mods |= Qt::ControlModifier;
  if (request.alt()) mods |= Qt::AltModifier;
  if (request.shift()) mods |= Qt::ShiftModifier;
  if (request.meta()) mods |= Qt::MetaModifier;

  auto key = static_cast<Qt::Key>(request.key_code());

  auto &controller = keyboardController();
  controller.keyPress(key, mods);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* --------------------------- KeyboardKeyReleaseCall --------------------- */

KeyboardKeyReleaseCall::KeyboardKeyReleaseCall(
  specter_proto::KeyboardService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::KeyboardService::AsyncService::RequestKeyRelease) {}

KeyboardKeyReleaseCall::~KeyboardKeyReleaseCall() = default;

std::unique_ptr<KeyboardKeyReleaseCallData>
KeyboardKeyReleaseCall::clone() const {
  return std::make_unique<KeyboardKeyReleaseCall>(getService(), getQueue());
}

KeyboardKeyReleaseCall::ProcessResult
KeyboardKeyReleaseCall::process(const Request &request) const {
  Qt::KeyboardModifiers mods = Qt::NoModifier;
  if (request.ctrl()) mods |= Qt::ControlModifier;
  if (request.alt()) mods |= Qt::AltModifier;
  if (request.shift()) mods |= Qt::ShiftModifier;
  if (request.meta()) mods |= Qt::MetaModifier;

  auto key = static_cast<Qt::Key>(request.key_code());

  auto &controller = keyboardController();
  controller.keyPress(key, mods);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ----------------------------- KeyboardTypeTextCall --------------------- */

KeyboardTypeTextCall::KeyboardTypeTextCall(
  specter_proto::KeyboardService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::KeyboardService::AsyncService::RequestTypeText) {}

KeyboardTypeTextCall::~KeyboardTypeTextCall() = default;

std::unique_ptr<KeyboardTypeTextCallData> KeyboardTypeTextCall::clone() const {
  return std::make_unique<KeyboardTypeTextCall>(getService(), getQueue());
}

KeyboardTypeTextCall::ProcessResult
KeyboardTypeTextCall::process(const Request &request) const {
  auto &controller = keyboardController();
  controller.typeText(QString::fromStdString(request.text()));

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* -------------------------- KeyboardTypeIntoObjectCall ------------------ */

KeyboardTypeIntoObjectCall::KeyboardTypeIntoObjectCall(
  specter_proto::KeyboardService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::KeyboardService::AsyncService::RequestTypeIntoObject) {}

KeyboardTypeIntoObjectCall::~KeyboardTypeIntoObjectCall() = default;

std::unique_ptr<KeyboardTypeIntoObjectCallData>
KeyboardTypeIntoObjectCall::clone() const {
  return std::make_unique<KeyboardTypeIntoObjectCall>(getService(), getQueue());
}

KeyboardTypeIntoObjectCall::ProcessResult
KeyboardTypeIntoObjectCall::process(const Request &request) const {
  const auto id =
    ObjectId::fromString(QString::fromStdString(request.object_id().id()));

  auto [status, widget] = tryGetSingleWidget(id);
  if (!status.ok()) return {status, {}};

  auto &controller = keyboardController();
  controller.typeTextIntoObject(widget, QString::fromStdString(request.text()));

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ------------------------------ KeyboardService ------------------------- */

KeyboardService::KeyboardService() = default;

KeyboardService::~KeyboardService() = default;

void KeyboardService::start(grpc::ServerCompletionQueue *queue) {
  auto key_press_call = new KeyboardKeyPressCall(this, queue);
  auto key_release_call = new KeyboardKeyReleaseCall(this, queue);
  auto type_text_call = new KeyboardTypeTextCall(this, queue);
  auto type_into_object_call = new KeyboardTypeIntoObjectCall(this, queue);

  key_press_call->proceed();
  key_release_call->proceed();
  type_text_call->proceed();
  type_into_object_call->proceed();
}

}// namespace specter
