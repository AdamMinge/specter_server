/* ----------------------------------- Local -------------------------------- */
#include "specter/input/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
/* -------------------------------------------------------------------------- */

namespace specter {

QWidget *getTargetWidget() {
  auto target = QApplication::focusWidget();
  if (!target) target = QApplication::activeWindow();

  return target;
}

QWidget *getTargetWidget(const QPoint &pos) {
  auto target = QApplication::widgetAt(pos);
  if (!target) target = QApplication::focusWidget();
  if (!target) target = QApplication::activeWindow();

  return target;
}

}// namespace specter
