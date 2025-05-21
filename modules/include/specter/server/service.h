#ifndef SPECTER_SERVER_SERVICE_H
#define SPECTER_SERVER_SERVICE_H

/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace grpc {

class Service;
class ServerCompletionQueue;

}  // namespace grpc

namespace specter {

/* ----------------------------------- Service ------------------------------ */

class LIB_SPECTER_API Service {
 public:
  explicit Service() = default;
  virtual ~Service() = default;

  virtual void start(grpc::ServerCompletionQueue* queue) = 0;
};

/* ------------------------------- ServiceWrapper --------------------------- */

template <typename GRPC_SERVICE>
class ServiceWrapper : public GRPC_SERVICE, public Service {
 public:
  explicit ServiceWrapper() = default;
  ~ServiceWrapper() override = default;
};

}  // namespace specter

#endif  // SPECTER_SERVER_SERVICE_H