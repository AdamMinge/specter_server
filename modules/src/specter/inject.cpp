/* ----------------------------------- Local -------------------------------- */
#include "specter/config.h"
#include "specter/module.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QThread>
#include <QtGlobal>
/* -------------------------------------------------------------------------- */

class ServerThread : public QThread {
 public:
  ServerThread(specter::SpecterModule& specter, const QHostAddress& host,
               quint16 port)
      : m_specter(specter), m_host(host), m_port(port) {}

 protected:
  void run() override { m_specter.getServer().listen(m_host, m_port); }

 private:
  specter::SpecterModule& m_specter;
  QHostAddress m_host;
  quint16 m_port;
};

void startServer() {
  auto valid_port = false;
  auto valid_host = false;

  const auto str_host = qEnvironmentVariable("SPECTER_SERVER_HOST", "0.0.0.0");
  const auto str_port = qEnvironmentVariable("SPECTER_SERVER_PORT", "5010");

  const auto host = QHostAddress(str_host);
  valid_host = !host.isNull();

  const auto port = str_port.toUInt(&valid_port);

  if (!valid_host) return;
  if (!valid_port) return;

  auto server_thread =
      new ServerThread(specter::SpecterModule::getInstance(), host, port);
  server_thread->start();
}

#if defined(SPECTER_OS_WINDOWS)
#include <io.h>
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) {
    startServer();
  }
  return TRUE;
}
#elif defined(SPECTER_OS_LINUX)
__attribute__((constructor)) static void init(void) { startServer(); }
#else
#error This operating system does not support DLL injection
#endif
