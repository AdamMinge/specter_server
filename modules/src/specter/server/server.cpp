/* ----------------------------------- Local -------------------------------- */
#include "specter/server/server.h"

#include "specter/server/call.h"
#include "specter/server/service.h"
/* ----------------------------------- GRPC --------------------------------- */
#include <grpc++/grpc++.h>
#include <grpc++/impl/service_type.h>
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ----------------------------------- Server ------------------------------- */

const int Server::poll_batch_size = 10;
const int Server::poll_interval_ms = 10;

Server::Server() = default;

Server::~Server() {
  m_server->Shutdown();
  m_queue->Shutdown();
}

void Server::listen(const QHostAddress &host, quint16 port) {
  const auto address =
    QLatin1String("%1:%2").arg(host.toString()).arg(port).toStdString();

  grpc::ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  for (const auto &service : m_services) {
    auto grpc_service = dynamic_cast<grpc::Service *>(service.get());
    Q_ASSERT(grpc_service);

    builder.RegisterService(grpc_service);
  }

  m_queue = builder.AddCompletionQueue();
  m_server = builder.BuildAndStart();

  startLoop();
}

void Server::startLoop() {
  for (const auto &service : m_services) { service->start(m_queue.get()); }

  QTimer *timer = new QTimer(this);
  QObject::connect(timer, &QTimer::timeout, [timer, this]() {
    void *tag = nullptr;
    bool ok = false;

    for (int i = 0; i < poll_batch_size; ++i) {
      auto status =
        m_queue->AsyncNext(&tag, &ok, std::chrono::system_clock::now());
      if (status == grpc::CompletionQueue::GOT_EVENT) {
        auto call_tag = static_cast<CallTag *>(tag);
        auto callable = static_cast<Callable *>(call_tag->callable);
        if (callable) callable->proceed(ok);
      } else if (status != grpc::CompletionQueue::GOT_EVENT) {
        return;
      }
    }
  });

  timer->start(poll_interval_ms);
}

}// namespace specter
