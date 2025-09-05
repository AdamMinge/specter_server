/* ----------------------------------- Local -------------------------------- */
#include "specter/service/mouse.h"

#include "specter/module.h"
#include "specter/search/utils.h"
#include "specter/service/utils.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ MouseClickCall -------------------------- */

MouseClickCall::MouseClickCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestClick) {}

MouseClickCall::~MouseClickCall() = default;

std::unique_ptr<MouseClickCallData> MouseClickCall::clone() const {
  return std::make_unique<MouseClickCall>(getService(), getQueue());
}

MouseClickCall::ProcessResult
MouseClickCall::process(const Request &request) const {
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
  controller.click(
    QPoint(request.offset().x(), request.offset().y()), button, double_click);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ------------------------------ MouseMoveCall --------------------------- */

MouseMoveCall::MouseMoveCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestMove) {}

MouseMoveCall::~MouseMoveCall() = default;

std::unique_ptr<MouseMoveCallData> MouseMoveCall::clone() const {
  return std::make_unique<MouseMoveCall>(getService(), getQueue());
}

MouseMoveCall::ProcessResult
MouseMoveCall::process(const Request &request) const {
  auto &controller = mouseController();
  controller.move(QPoint(request.offset().x(), request.offset().y()));

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ------------------------------ MouseScrollCall ------------------------- */

MouseScrollCall::MouseScrollCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestScroll) {}

MouseScrollCall::~MouseScrollCall() = default;

std::unique_ptr<MouseScrollCallData> MouseScrollCall::clone() const {
  return std::make_unique<MouseScrollCall>(getService(), getQueue());
}

MouseScrollCall::ProcessResult
MouseScrollCall::process(const Request &request) const {
  auto &controller = mouseController();
  controller.scroll(QPoint(request.delta_x(), request.delta_y()));

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ---------------------------- MouseClickObjectCall ---------------------- */

MouseClickObjectCall::MouseClickObjectCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestClickObject) {}

MouseClickObjectCall::~MouseClickObjectCall() = default;

std::unique_ptr<MouseClickObjectCallData> MouseClickObjectCall::clone() const {
  return std::make_unique<MouseClickObjectCall>(getService(), getQueue());
}

MouseClickObjectCall::ProcessResult
MouseClickObjectCall::process(const Request &request) const {
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

/* --------------------------- MouseMoveOverObjectCall -------------------- */

MouseMoveOverObjectCall::MouseMoveOverObjectCall(
  specter_proto::MouseService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MouseService::AsyncService::RequestMoveOverObject) {}

MouseMoveOverObjectCall::~MouseMoveOverObjectCall() = default;

std::unique_ptr<MouseMoveOverObjectCallData>
MouseMoveOverObjectCall::clone() const {
  return std::make_unique<MouseMoveOverObjectCall>(getService(), getQueue());
}

MouseMoveOverObjectCall::ProcessResult
MouseMoveOverObjectCall::process(const Request &request) const {
  const auto id =
    ObjectId::fromString(QString::fromStdString(request.object_id().id()));

  auto [status, widget] = tryGetSingleWidget(id);
  if (!status.ok()) return {status, {}};

  auto pos = request.has_anchor() ? resolvePosition(widget, request.anchor())
                                  : resolvePosition(widget, request.offset());

  auto &controller = mouseController();
  controller.moveOverObject(widget, pos);

  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ------------------------------ MouseService ---------------------------- */

MouseService::MouseService() = default;

MouseService::~MouseService() = default;

void MouseService::start(grpc::ServerCompletionQueue *queue) {
  auto mouse_click_call = new MouseClickCall(this, queue);
  auto mouse_move_call = new MouseMoveCall(this, queue);
  auto mouse_scroll_call = new MouseScrollCall(this, queue);
  auto mouse_click_object_call = new MouseClickObjectCall(this, queue);
  auto mouse_move_over_object_call = new MouseMoveOverObjectCall(this, queue);

  mouse_click_call->proceed();
  mouse_move_call->proceed();
  mouse_scroll_call->proceed();
  mouse_click_object_call->proceed();
  mouse_move_over_object_call->proceed();
}

}// namespace specter
