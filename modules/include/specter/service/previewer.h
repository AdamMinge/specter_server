#ifndef SPECTER_SERVICE_PREVIEWER_H
#define SPECTER_SERVICE_PREVIEWER_H

/* ----------------------------------- Proto -------------------------------- */
#include <specter_proto/specter.grpc.pb.h>
#include <specter_proto/specter.pb.h>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/server/call.h"
#include "specter/server/service.h"
/* -------------------------------------------------------------------------- */

namespace specter {

class PreviewObserver;
class PreviewObserverQueue;

/* ------------------------- PreviewerListenCommandsCall -------------------- */

using PreviewerListenCommandsCallData = StreamCallData<
  specter_proto::PreviewerService::AsyncService, specter_proto::Object,
  specter_proto::PreviewImage>;

class LIB_SPECTER_API PreviewerListenCommandsCall
    : public PreviewerListenCommandsCallData {
public:
  explicit PreviewerListenCommandsCall(
    specter_proto::PreviewerService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~PreviewerListenCommandsCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<PreviewerListenCommandsCallData> clone() const override;

private:
  std::unique_ptr<PreviewObserver> m_observer;
  std::unique_ptr<PreviewObserverQueue> m_observer_queue;
};

/* ------------------------------ PreviewerService -------------------------- */

class PreviewerService
    : public ServiceWrapper<specter_proto::PreviewerService::AsyncService> {
public:
  explicit PreviewerService();
  ~PreviewerService() override;

  void start(grpc::ServerCompletionQueue *queue) override;
};

}// namespace specter

#endif// SPECTER_SERVICE_PREVIEWER_H
