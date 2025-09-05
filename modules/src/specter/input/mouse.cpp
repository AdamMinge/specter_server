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

void MouseController::move(const QPoint &pos) {
  auto widget = QApplication::widgetAt(pos);
  if (!widget) return;

  auto local_pos = QPoint(
    pos.x() - widget->mapToGlobal(QPoint(0, 0)).x(),
    pos.y() - widget->mapToGlobal(QPoint(0, 0)).y());

  auto move_event = new QMouseEvent(
    QEvent::MouseMove, local_pos, local_pos, pos, Qt::NoButton, Qt::NoButton,
    Qt::NoModifier);
  QCoreApplication::postEvent(widget, move_event);
}

void MouseController::click(
  const QPoint &pos, Qt::MouseButton button, bool doubleClick) {
  auto widget = QApplication::widgetAt(pos);
  if (!widget) return;

  auto local_pos = widget->mapFromGlobal(pos);
  auto type =
    doubleClick ? QEvent::MouseButtonDblClick : QEvent::MouseButtonPress;

  auto press = new QMouseEvent(
    type, local_pos, local_pos, pos, button, button, Qt::NoModifier);
  QCoreApplication::postEvent(widget, press);

  auto release = new QMouseEvent(
    QEvent::MouseButtonRelease, local_pos, local_pos, pos, button, Qt::NoButton,
    Qt::NoModifier);
  QCoreApplication::postEvent(widget, release);
}

void MouseController::scroll(const QPoint &delta) {
  auto global_pos = QCursor::pos();
  auto widget = QApplication::widgetAt(global_pos);
  if (!widget) return;

  auto local_pos = widget->mapFromGlobal(global_pos);

  auto wheel = new QWheelEvent(
    local_pos, global_pos, QPoint(), delta, Qt::NoButton, Qt::NoModifier,
    Qt::NoScrollPhase, false);
  QCoreApplication::postEvent(widget, wheel);
}

void MouseController::clickOnObject(
  QWidget *widget, const QPoint &pos, Qt::MouseButton button,
  bool doubleClick) {
  auto global_pos = widget->mapToGlobal(pos);
  click(global_pos, button, doubleClick);
}

void MouseController::moveOverObject(QWidget *widget, const QPoint &pos) {
  auto global_pos = widget->mapToGlobal(pos);
  move(global_pos);
}

}// namespace specter
