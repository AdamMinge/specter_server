/* ----------------------------------- Local -------------------------------- */
#include "specter/service/previewer.h"

#include "specter/module.h"
#include "specter/observe/preview/observer.h"
#include "specter/search/utils.h"
#include "specter/service/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------- PreviewerListenCommandsCall -------------------- */

PreviewerListenCommandsCall::PreviewerListenCommandsCall(
  specter_proto::PreviewerService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : StreamCallData(
        service, queue, CallTag{this},
        &specter_proto::PreviewerService::AsyncService::RequestListenPreview),
      m_observer(std::make_unique<PreviewObserver>()),
      m_observer_queue(std::make_unique<PreviewObserverQueue>()) {

  m_observer_queue->setObserver(m_observer.get());
}

PreviewerListenCommandsCall::~PreviewerListenCommandsCall() = default;

std::unique_ptr<PreviewerListenCommandsCallData>
PreviewerListenCommandsCall::clone() const {
  return std::make_unique<PreviewerListenCommandsCall>(
    getService(), getQueue());
}

PreviewerListenCommandsCall::ProcessResult
PreviewerListenCommandsCall::process(const Request &request) const {
  const auto query =
    ObjectQuery::fromString(QString::fromStdString(request.query()));

  auto [status, object] = tryGetSingleObject(query);
  if (!status.ok()) return status;

  if (!m_observer->isObserving()) { m_observer->start(); }
  if (m_observer->getObject() != object) { m_observer->setObject(object); }

  if (m_observer_queue->isEmpty()) return {};

  const auto observer_preview = m_observer_queue->popPreview();

  specter_proto::PreviewImage response;
  response.set_image(observer_preview.constData(), observer_preview.size());

  return response;
}

/* ------------------------------ PreviewerService -------------------------- */

PreviewerService::PreviewerService() = default;

PreviewerService::~PreviewerService() = default;

void PreviewerService::start(grpc::ServerCompletionQueue *queue) {
  auto listen_call = new PreviewerListenCommandsCall(this, queue);
  listen_call->proceed();
}

}// namespace specter
