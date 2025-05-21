#ifndef SPECTER_SERVICE_MARKER_H
#define SPECTER_SERVICE_MARKER_H

/* ----------------------------------- Proto -------------------------------- */
#include <specter_proto/specter.grpc.pb.h>
#include <specter_proto/specter.pb.h>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/server/call.h"
#include "specter/server/service.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ MarkerStartCall -------------------------- */

using MarkerStartCallData = CallData<
  specter_proto::MarkerService::AsyncService, google::protobuf::Empty,
  google::protobuf::Empty>;

class LIB_SPECTER_API MarkerStartCall : public MarkerStartCallData {
public:
  explicit MarkerStartCall(
    specter_proto::MarkerService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MarkerStartCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MarkerStartCallData> clone() const override;
};

/* ------------------------------ MarkerStartCall -------------------------- */

using MarkerStartCallData = CallData<
  specter_proto::MarkerService::AsyncService, google::protobuf::Empty,
  google::protobuf::Empty>;

class LIB_SPECTER_API MarkerStopCall : public MarkerStartCallData {
public:
  explicit MarkerStopCall(
    specter_proto::MarkerService::AsyncService *service,
    grpc::ServerCompletionQueue *queue);
  ~MarkerStopCall() override;

  ProcessResult process(const Request &request) const override;

  std::unique_ptr<MarkerStartCallData> clone() const override;
};

/* ------------------------------- MarkerService --------------------------- */

class MarkerService
    : public ServiceWrapper<specter_proto::MarkerService::AsyncService> {
public:
  explicit MarkerService();
  ~MarkerService() override;

  void start(grpc::ServerCompletionQueue *queue) override;
};

}// namespace specter

#endif// SPECTER_SERVICE_MARKER_H
