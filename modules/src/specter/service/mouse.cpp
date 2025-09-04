/* ----------------------------------- Local -------------------------------- */
#include "specter/service/mouse.h"
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
MouseClickCall::process(const Request &request) const {}

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
MouseMoveCall::process(const Request &request) const {}

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
MouseScrollCall::process(const Request &request) const {}

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
MouseClickObjectCall::process(const Request &request) const {}

/* ------------------------------ MouseService ---------------------------- */

MouseService::MouseService() = default;

MouseService::~MouseService() = default;

void MouseService::start(grpc::ServerCompletionQueue *queue) {
  auto mouse_click_call = new MouseClickCall(this, queue);
  auto mouse_move_call = new MouseMoveCall(this, queue);
  auto mouse_scroll_call = new MouseScrollCall(this, queue);
  auto mouse_click_object_call = new MouseClickObjectCall(this, queue);

  mouse_click_call->proceed();
  mouse_move_call->proceed();
  mouse_scroll_call->proceed();
  mouse_click_object_call->proceed();
}

}// namespace specter
