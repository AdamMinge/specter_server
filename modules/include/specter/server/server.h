#ifndef SPECTER_SERVER_SERVER_H
#define SPECTER_SERVER_SERVER_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QHostAddress>
#include <QObject>
#include <QTimer>
/* --------------------------------- Standard ------------------------------- */
#include <memory>
#include <thread>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace grpc {

class Alarm;
class Server;
class Service;
class ServerCompletionQueue;

}// namespace grpc

namespace specter {

class Service;

/* ------------------------------- IsValidService --------------------------- */

template<typename TYPE>
concept IsValidService = std::is_base_of_v<specter::Service, TYPE> &&
                         std::is_base_of_v<grpc::Service, TYPE>;

/* ----------------------------------- Server ------------------------------- */

class LIB_SPECTER_API Server : public QObject {
  Q_OBJECT

public:
  explicit Server();
  ~Server();

  void listen(const QHostAddress &host, quint16 port);

  template<IsValidService SERVICE, typename... ARGS>
  void registerService(ARGS &&...args);

private:
  void startLoop();
  void processQueue();

  std::list<std::unique_ptr<Service>> m_services;
  std::unique_ptr<grpc::Server> m_server;
  std::unique_ptr<grpc::ServerCompletionQueue> m_queue;
  std::unique_ptr<std::thread> m_process_queue_thread;
};

template<IsValidService SERVICE, typename... ARGS>
void Server::registerService(ARGS &&...args) {
  m_services.emplace_back(
    std::make_unique<SERVICE>(std::forward<ARGS>(args)...));
}

}// namespace specter

#endif// SPECTER_SERVER_SERVER_H