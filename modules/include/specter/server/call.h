#ifndef SPECTER_SERVER_CALL_H
#define SPECTER_SERVER_CALL_H

/* ----------------------------------- GRPC --------------------------------- */
#include <grpc++/grpc++.h>
/* --------------------------------- Standard ------------------------------- */
#include <memory>
#include <optional>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ---------------------------------- CallTag ------------------------------- */

struct LIB_SPECTER_API CallTag {
  void *callable;
};

/* ---------------------------------- Callable ------------------------------ */

class LIB_SPECTER_API Callable {
public:
  explicit Callable();
  virtual ~Callable();

  virtual void proceed() = 0;
};

/* ---------------------------------- CallData ------------------------------ */

template<typename SERVICE, typename REQUEST, typename RESPONSE>
class CallData : public Callable {
protected:
  enum class CallStatus { Create, Process, Finish };

  using Request = REQUEST;
  using Response = RESPONSE;
  using Service = SERVICE;

  using ProcessResult = std::pair<grpc::Status, Response>;
  using RequestMethod = void (Service::*)(
    grpc::ServerContext *, Request *,
    grpc::ServerAsyncResponseWriter<Response> *, grpc::CompletionQueue *,
    grpc::ServerCompletionQueue *, void *);

public:
  explicit CallData(
    Service *service, grpc::ServerCompletionQueue *queue, CallTag tag,
    RequestMethod request_method);
  ~CallData() override;

  void proceed() override;

  [[nodiscard]] Service *getService() const;
  [[nodiscard]] grpc::ServerCompletionQueue *getQueue() const;

protected:
  virtual ProcessResult process(const Request &request) const = 0;

  virtual std::unique_ptr<CallData> clone() const = 0;

protected:
  Service *m_service;
  grpc::ServerCompletionQueue *m_queue;

  CallTag m_tag;
  RequestMethod m_request_method;
  CallStatus m_status;
  grpc::ServerContext m_context;
  Request m_request;
  grpc::ServerAsyncResponseWriter<Response> m_responder;
};

template<typename SERVICE, typename REQUEST, typename RESPONSE>
CallData<SERVICE, REQUEST, RESPONSE>::CallData(
  Service *service, grpc::ServerCompletionQueue *queue, CallTag tag,
  RequestMethod request_method)
    : m_service(service), m_queue(queue), m_tag(tag),
      m_request_method(request_method), m_status(CallStatus::Create),
      m_responder(&m_context) {}

template<typename SERVICE, typename REQUEST, typename RESPONSE>
CallData<SERVICE, REQUEST, RESPONSE>::~CallData() = default;

template<typename SERVICE, typename REQUEST, typename RESPONSE>
void CallData<SERVICE, REQUEST, RESPONSE>::proceed() {
  switch (m_status) {
    case CallStatus::Create: {
      m_status = CallStatus::Process;
      (m_service->*m_request_method)(
        &m_context, &m_request, &m_responder, m_queue, m_queue,
        static_cast<void *>(&m_tag));
      break;
    }
    case CallStatus::Process: {
      auto cell_data = clone().release();
      cell_data->proceed();

      const auto [status, response] = process(m_request);
      if (status.ok()) {
        m_responder.Finish(response, status, static_cast<void *>(&m_tag));
      } else {
        m_responder.FinishWithError(status, static_cast<void *>(&m_tag));
      }

      m_status = CallStatus::Finish;
      break;
    }
    default: {
      delete this;
    }
  }
}

template<typename SERVICE, typename REQUEST, typename RESPONSE>
CallData<SERVICE, REQUEST, RESPONSE>::Service *
CallData<SERVICE, REQUEST, RESPONSE>::getService() const {
  return m_service;
}

template<typename SERVICE, typename REQUEST, typename RESPONSE>
grpc::ServerCompletionQueue *
CallData<SERVICE, REQUEST, RESPONSE>::getQueue() const {
  return m_queue;
}

/* ------------------------------- StreamCallData --------------------------- */

template<typename SERVICE, typename REQUEST, typename RESPONSE>
class StreamCallData : public Callable {
protected:
  enum class CallStatus { Create, Process, Processing, Finish };

  using Request = REQUEST;
  using Response = RESPONSE;
  using Service = SERVICE;

  using ProcessResult = std::optional<std::variant<grpc::Status, Response>>;
  using RequestMethod = void (Service::*)(
    grpc::ServerContext *, Request *, grpc::ServerAsyncWriter<Response> *,
    grpc::CompletionQueue *, grpc::ServerCompletionQueue *, void *);

public:
  explicit StreamCallData(
    Service *service, grpc::ServerCompletionQueue *queue, CallTag tag,
    RequestMethod request_method);
  ~StreamCallData() override;

  void proceed() override;

  [[nodiscard]] Service *getService() const;
  [[nodiscard]] grpc::ServerCompletionQueue *getQueue() const;

protected:
  [[nodiscard]] virtual ProcessResult process(const Request &request) const = 0;

  virtual std::unique_ptr<StreamCallData> clone() const = 0;

protected:
  Service *m_service;
  grpc::ServerCompletionQueue *m_queue;

  CallTag m_tag;
  RequestMethod m_request_method;
  CallStatus m_status;
  grpc::ServerContext m_context;
  Request m_request;
  grpc::ServerAsyncWriter<Response> m_responder;
};

template<typename SERVICE, typename REQUEST, typename RESPONSE>
StreamCallData<SERVICE, REQUEST, RESPONSE>::StreamCallData(
  Service *service, grpc::ServerCompletionQueue *queue, CallTag tag,
  RequestMethod request_method)
    : m_service(service), m_queue(queue), m_tag(tag),
      m_request_method(request_method), m_status(CallStatus::Create),
      m_responder(&m_context) {}

template<typename SERVICE, typename REQUEST, typename RESPONSE>
StreamCallData<SERVICE, REQUEST, RESPONSE>::~StreamCallData() = default;

template<typename SERVICE, typename REQUEST, typename RESPONSE>
void StreamCallData<SERVICE, REQUEST, RESPONSE>::proceed() {
  const auto _process = [this]() {
    const auto result = process(m_request);
    if (result.has_value()) {
      if (const auto status = std::get_if<grpc::Status>(&*result); status) {
        m_responder.Finish(*status, static_cast<void *>(&m_tag));
        m_status = CallStatus::Finish;
        return;
      }

      if (const auto response = std::get_if<Response>(&*result); response) {
        m_responder.Write(*response, static_cast<void *>(&m_tag));
      }
    }

    m_status = CallStatus::Processing;
  };

  switch (m_status) {
    case CallStatus::Create: {
      m_status = CallStatus::Process;
      (m_service->*m_request_method)(
        &m_context, &m_request, &m_responder, m_queue, m_queue,
        static_cast<void *>(&m_tag));
      break;
    }
    case CallStatus::Process: {
      auto cell_data = clone().release();
      cell_data->proceed();

      _process();
      break;
    }
    case CallStatus::Processing: {
      _process();
      break;
    }
    default: {
      delete this;
    }
  }
}

template<typename SERVICE, typename REQUEST, typename RESPONSE>
StreamCallData<SERVICE, REQUEST, RESPONSE>::Service *
StreamCallData<SERVICE, REQUEST, RESPONSE>::getService() const {
  return m_service;
}

template<typename SERVICE, typename REQUEST, typename RESPONSE>
grpc::ServerCompletionQueue *
StreamCallData<SERVICE, REQUEST, RESPONSE>::getQueue() const {
  return m_queue;
}

}// namespace specter

#endif// SPECTER_SERVER_CALL_H