/* ----------------------------------- Local -------------------------------- */
#include "specter/service/marker.h"

#include "specter/module.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ MarkerStartCall -------------------------- */

MarkerStartCall::MarkerStartCall(
  specter_proto::MarkerService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MarkerService::AsyncService::RequestStart) {}

MarkerStartCall::~MarkerStartCall() = default;

std::unique_ptr<MarkerStartCallData> MarkerStartCall::clone() const {
  return std::make_unique<MarkerStartCall>(getService(), getQueue());
}

MarkerStartCall::ProcessResult
MarkerStartCall::process(const Request &request) const {
  if (marker().isMarking()) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "The start cannot be triggered, the marker is already working"),
      {}};
  }

  marker().start();
  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ------------------------------ MarkerStopCall --------------------------- */

MarkerStopCall::MarkerStopCall(
  specter_proto::MarkerService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : CallData(
        service, queue, CallTag{this},
        &specter_proto::MarkerService::AsyncService::RequestStop) {}

MarkerStopCall::~MarkerStopCall() = default;

std::unique_ptr<MarkerStartCallData> MarkerStopCall::clone() const {
  return std::make_unique<MarkerStopCall>(getService(), getQueue());
}

MarkerStopCall::ProcessResult
MarkerStopCall::process(const Request &request) const {
  if (!marker().isMarking()) {
    return {
      grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT,
        "The stop cannot be triggered, the marker is already stopped"),
      {}};
  }

  marker().stop();
  return {grpc::Status::OK, google::protobuf::Empty{}};
}

/* ---------------------- MarkerListenSelectionChangesCall ----------------- */

MarkerListenSelectionChangesCall::MarkerListenSelectionChangesCall(
  specter_proto::MarkerService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : StreamCallData(
        service, queue, CallTag{this},
        &specter_proto::MarkerService::AsyncService::
          RequestListenSelectionChanges),
      m_observer_queue(std::make_unique<MarkerObserverQueue>()) {}

MarkerListenSelectionChangesCall::~MarkerListenSelectionChangesCall() = default;

std::unique_ptr<MarkerListenSelectionChangesCallData>
MarkerListenSelectionChangesCall::clone() const {
  return std::make_unique<MarkerListenSelectionChangesCall>(
    getService(), getQueue());
}

MarkerListenSelectionChangesCall::StartResult
MarkerListenSelectionChangesCall::start(const Request &request) const {
  m_observer_queue->setObserver(&marker());
  return {};
}

MarkerListenSelectionChangesCall::ProcessResult
MarkerListenSelectionChangesCall::process() const {
  if (m_observer_queue->isEmpty()) return {};

  const auto selected_object = m_observer_queue->popPreview();

  specter_proto::ObjectId response;
  response.set_id(selected_object.toString().toStdString());

  return response;
}

/* ------------------------------- MarkerService --------------------------- */

MarkerService::MarkerService() = default;

MarkerService::~MarkerService() = default;

void MarkerService::start(grpc::ServerCompletionQueue *queue) {
  auto start_call = new MarkerStartCall(this, queue);
  auto stop_call = new MarkerStopCall(this, queue);
  auto listen_selection_changes_call =
    new MarkerListenSelectionChangesCall(this, queue);

  start_call->proceed();
  stop_call->proceed();
  listen_selection_changes_call->proceed();
}

}// namespace specter
