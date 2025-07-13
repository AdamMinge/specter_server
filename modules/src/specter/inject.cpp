/* ----------------------------------- Local -------------------------------- */
#include "specter/config.h"
#include "specter/module.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
#include <QThread>
#include <QtGlobal>
/* -------------------------------------------------------------------------- */

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

  QMetaObject::invokeMethod(
    qApp,
    [host, port]() {
      auto &specter = specter::SpecterModule::getInstance();
      specter.getServer().listen(host, port);
    },
    Qt::QueuedConnection);
}

#if defined(SPECTER_OS_WINDOWS)
#include <io.h>
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) { startServer(); }
  return TRUE;
}
#elif defined(SPECTER_OS_LINUX)
__attribute__((constructor)) static void init(void) { startServer(); }
#else
#error This operating system does not support DLL injection
#endif
