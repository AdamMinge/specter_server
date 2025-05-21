#ifndef SPECTER_SERVICE_RECORDER_H
#define SPECTER_SERVICE_RECORDER_H

/* ----------------------------------- Proto -------------------------------- */
#include <specter_proto/specter.grpc.pb.h>
#include <specter_proto/specter.pb.h>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/server/call.h"
#include "specter/server/service.h"
/* -------------------------------------------------------------------------- */

namespace specter {

class ActionRecorder;
class ActionRecorderQueue;
class RecordedActionsMapper;

/* ------------------------- RecorderListenCommandsCall -------------------- */

using RecorderListenCommandsCallData = StreamCallData<
  specter_proto::RecorderService::AsyncService, google::protobuf::Empty,
  specter_proto::RecorderCommand>;

class LIB_SPECTER_API RecorderListenCommandsCall
    : public RecorderListenCommandsCallData {
public:
  explicit RecorderListenCommandsCall(
    specter_proto::RecorderService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~RecorderListenCommandsCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<RecorderListenCommandsCallData> clone() const override;

private:
  std::unique_ptr<ActionRecorder> m_recorder;
  std::unique_ptr<ActionRecorderQueue> m_recorder_queue;
  std::unique_ptr<RecordedActionsMapper> m_mapper;
};

/* ------------------------------ RecorderService --------------------------- */

class RecorderService
    : public ServiceWrapper<specter_proto::RecorderService::AsyncService> {
public:
  explicit RecorderService();
  ~RecorderService() override;

  void start(grpc::ServerCompletionQueue *queue) override;
};

}// namespace specter

#endif// SPECTER_SERVICE_RECORDER_H
