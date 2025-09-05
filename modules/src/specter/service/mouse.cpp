/* ----------------------------------- Local -------------------------------- */
#include "specter/service/mouse.h"

#include "specter/module.h"
#include "specter/search/utils.h"
#include "specter/service/utils.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* --------------------------- MousePressButtonCall ----------------------- */

MousePressButtonCall::MousePressButtonCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestPressButton) {}

MousePressButtonCall::~MousePressButtonCall() = default;

std::unique_ptr<MousePressButtonCallData> MousePressButtonCall::clone() const {
  return std::make_unique<MousePressButtonCall>(getService(), getQueue());
}

MousePressButtonCall::ProcessResult
MousePressButtonCall::process(const Request &request) const {
  auto double_click =
    request.has_double_click() ? request.double_click() : false;

  Qt::MouseButton button = Qt::LeftButton;
  switch (request.button()) {
    case specter_proto::MouseButton::RIGHT:
      button = Qt::RightButton;
      break;
    case specter_proto::MouseButton::MIDDLE:
      button = Qt::MiddleButton;
      break;
    default:
      break;
  }


  auto &controller = mouseController();
  controller.pressButton(
    QPoint(request.offset().x(), request.offset().y()), button, double_click);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* -------------------------- MouseReleaseButtonCall ---------------------- */

MouseReleaseButtonCall::MouseReleaseButtonCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestReleaseButton) {}

MouseReleaseButtonCall::~MouseReleaseButtonCall() = default;

std::unique_ptr<MouseReleaseButtonCallData>
MouseReleaseButtonCall::clone() const {
  return std::make_unique<MouseReleaseButtonCall>(getService(), getQueue());
}

MouseReleaseButtonCall::ProcessResult
MouseReleaseButtonCall::process(const Request &request) const {
  Qt::MouseButton button = Qt::LeftButton;
  switch (request.button()) {
    case specter_proto::MouseButton::RIGHT:
      button = Qt::RightButton;
      break;
    case specter_proto::MouseButton::MIDDLE:
      button = Qt::MiddleButton;
      break;
    default:
      break;
  }


  auto &controller = mouseController();
  controller.releaseButton(
    QPoint(request.offset().x(), request.offset().y()), button);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* --------------------------- MouseClickButtonCall ----------------------- */

MouseClickButtonCall::MouseClickButtonCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestClickButton) {}

MouseClickButtonCall::~MouseClickButtonCall() = default;

std::unique_ptr<MouseClickButtonCallData> MouseClickButtonCall::clone() const {
  return std::make_unique<MouseClickButtonCall>(getService(), getQueue());
}

MouseClickButtonCall::ProcessResult
MouseClickButtonCall::process(const Request &request) const {
  auto double_click =
    request.has_double_click() ? request.double_click() : false;

  Qt::MouseButton button = Qt::LeftButton;
  switch (request.button()) {
    case specter_proto::MouseButton::RIGHT:
      button = Qt::RightButton;
      break;
    case specter_proto::MouseButton::MIDDLE:
      button = Qt::MiddleButton;
      break;
    default:
      break;
  }


  auto &controller = mouseController();
  controller.clickButton(
    QPoint(request.offset().x(), request.offset().y()), button, double_click);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* --------------------------- MouseMoveCursorCall ------------------------ */

MouseMoveCursorCall::MouseMoveCursorCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestMoveCursor) {}

MouseMoveCursorCall::~MouseMoveCursorCall() = default;

std::unique_ptr<MouseMoveCursorCallData> MouseMoveCursorCall::clone() const {
  return std::make_unique<MouseMoveCursorCall>(getService(), getQueue());
}

MouseMoveCursorCall::ProcessResult
MouseMoveCursorCall::process(const Request &request) const {
  auto &controller = mouseController();
  controller.moveCursor(QPoint(request.offset().x(), request.offset().y()));

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* --------------------------- MouseScrollWheelCall ----------------------- */

MouseScrollWheelCall::MouseScrollWheelCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestScrollWheel) {}

MouseScrollWheelCall::~MouseScrollWheelCall() = default;

std::unique_ptr<MouseScrollWheelCallData> MouseScrollWheelCall::clone() const {
  return std::make_unique<MouseScrollWheelCall>(getService(), getQueue());
}

MouseScrollWheelCall::ProcessResult
MouseScrollWheelCall::process(const Request &request) const {
  auto &controller = mouseController();
  controller.scrollWheel(QPoint(request.delta_x(), request.delta_y()));

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ---------------------------- MouseClickOnObject ------------------------ */

MouseClickOnObject::MouseClickOnObject(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestClickOnObject) {}

MouseClickOnObject::~MouseClickOnObject() = default;

std::unique_ptr<MouseClickOnObjectData> MouseClickOnObject::clone() const {
  return std::make_unique<MouseClickOnObject>(getService(), getQueue());
}

MouseClickOnObject::ProcessResult
MouseClickOnObject::process(const Request &request) const {
  const auto id =
    ObjectId::fromString(QString::fromStdString(request.object_id().id()));

  auto [status, widget] = tryGetSingleWidget(id);
  if (!status.ok()) return {status, {}};

  auto double_click =
    request.has_double_click() ? request.double_click() : false;

  Qt::MouseButton button = Qt::LeftButton;
  switch (request.button()) {
    case specter_proto::MouseButton::RIGHT:
      button = Qt::RightButton;
      break;
    case specter_proto::MouseButton::MIDDLE:
      button = Qt::MiddleButton;
      break;
    default:
      break;
  }

  auto pos = request.has_anchor() ? resolvePosition(widget, request.anchor())
                                  : resolvePosition(widget, request.offset());

  auto &controller = mouseController();
  controller.clickOnObject(widget, pos, button, double_click);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ------------------------- MouseHoverOverObjectCall --------------------- */

MouseHoverOverObjectCall::MouseHoverOverObjectCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestHoverOverObject) {}

MouseHoverOverObjectCall::~MouseHoverOverObjectCall() = default;

std::unique_ptr<MouseHoverOverObjectCallData>
MouseHoverOverObjectCall::clone() const {
  return std::make_unique<MouseHoverOverObjectCall>(getService(), getQueue());
}

MouseHoverOverObjectCall::ProcessResult
MouseHoverOverObjectCall::process(const Request &request) const {
  const auto id =
    ObjectId::fromString(QString::fromStdString(request.object_id().id()));

  auto [status, widget] = tryGetSingleWidget(id);
  if (!status.ok()) return {status, {}};

  auto pos = request.has_anchor() ? resolvePosition(widget, request.anchor())
                                  : resolvePosition(widget, request.offset());

  auto &controller = mouseController();
  controller.hoverOverObject(widget, pos);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ------------------------------ MouseService ---------------------------- */

MouseService::MouseService() = default;

MouseService::~MouseService() = default;

void MouseService::start(grpc::ServerCompletionQueue *queue) {
  auto mouse_press_button_call = new MousePressButtonCall(this, queue);
  auto mouse_release_button_call = new MouseReleaseButtonCall(this, queue);
  auto mouse_click_button_call = new MouseClickButtonCall(this, queue);
  auto mouse_move_cursor_call = new MouseMoveCursorCall(this, queue);
  auto mouse_scroll_wheel_call = new MouseScrollWheelCall(this, queue);
  auto mouse_click_on_object_call = new MouseClickOnObject(this, queue);
  auto mouse_hover_over_object_call = new MouseHoverOverObjectCall(this, queue);

  mouse_press_button_call->proceed();
  mouse_release_button_call->proceed();
  mouse_click_button_call->proceed();
  mouse_move_cursor_call->proceed();
  mouse_scroll_wheel_call->proceed();
  mouse_click_on_object_call->proceed();
  mouse_hover_over_object_call->proceed();
}

}// namespace specter
