/* ----------------------------------- Local -------------------------------- */
#include "specter/input/mouse.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
#include <QMouseEvent>
#include <QWidget>
/* -------------------------------------------------------------------------- */

namespace specter {

MouseController::MouseController() = default;

MouseController::~MouseController() = default;

void MouseController::pressButton(
  QWidget *target, const QPoint &pos, Qt::MouseButton button,
  bool doubleClick) {

  auto local_pos = target->mapFromGlobal(pos);
  auto type =
    doubleClick ? QEvent::MouseButtonDblClick : QEvent::MouseButtonPress;

  auto press =
    new QMouseEvent(type, local_pos, pos, button, button, Qt::NoModifier);
  QCoreApplication::postEvent(target, press);
}

void MouseController::releaseButton(
  QWidget *target, const QPoint &pos, Qt::MouseButton button) {
  auto local_pos = target->mapFromGlobal(pos);
  auto release = new QMouseEvent(
    QEvent::MouseButtonRelease, local_pos, pos, button, Qt::NoButton,
    Qt::NoModifier);
  QCoreApplication::postEvent(target, release);
}

void MouseController::clickButton(
  QWidget *target, const QPoint &pos, Qt::MouseButton button,
  bool doubleClick) {
  hover(target, pos);
  pressButton(target, pos, button, doubleClick);
  releaseButton(target, pos, button);
}

void MouseController::moveCursor(const QPoint &pos) { QCursor::setPos(pos); }

void MouseController::scrollWheel(QWidget *target, const QPoint &delta) {
  auto global_pos = QCursor::pos();

  auto localPos = target->mapFromGlobal(global_pos);

  auto wheel = new QWheelEvent(
    localPos, global_pos, QPoint(), delta, Qt::NoButton, Qt::NoModifier,
    Qt::NoScrollPhase, false);
  QCoreApplication::postEvent(target, wheel);
}

void MouseController::hover(QWidget *target, const QPoint &pos) {
  auto global_pos = target->mapToGlobal(pos);
  moveCursor(global_pos);
}

}// namespace specter
