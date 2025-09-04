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

void MouseController::move(int x, int y) {
  auto widget = QApplication::widgetAt(x, y);
  if (!widget) return;

  auto globalPos = QPoint(x, y);
  auto pos = QPoint(
    x - widget->mapToGlobal(QPoint(0, 0)).x(),
    y - widget->mapToGlobal(QPoint(0, 0)).y());

  auto moveEvent = new QMouseEvent(
    QEvent::MouseMove, pos, pos, globalPos, Qt::NoButton, Qt::NoButton,
    Qt::NoModifier);
  QCoreApplication::postEvent(widget, moveEvent);
}

void MouseController::click(
  int x, int y, Qt::MouseButton button, bool doubleClick) {
  QWidget *widget = QApplication::widgetAt(x, y);
  if (!widget) return;

  auto globalPos = QPoint(x, y);
  auto pos = widget->mapFromGlobal(globalPos);
  auto type =
    doubleClick ? QEvent::MouseButtonDblClick : QEvent::MouseButtonPress;

  auto press =
    new QMouseEvent(type, pos, pos, globalPos, button, button, Qt::NoModifier);
  QCoreApplication::postEvent(widget, press);

  auto release = new QMouseEvent(
    QEvent::MouseButtonRelease, pos, pos, globalPos, button, Qt::NoButton,
    Qt::NoModifier);
  QCoreApplication::postEvent(widget, release);
}

void MouseController::scroll(int deltaX, int deltaY) {
  auto globalPos = QCursor::pos();
  auto widget = QApplication::widgetAt(globalPos);
  if (!widget) return;

  auto pos = widget->mapFromGlobal(globalPos);

  auto wheel = new QWheelEvent(
    pos, globalPos, QPoint(), QPoint(deltaX, deltaY), Qt::NoButton,
    Qt::NoModifier, Qt::NoScrollPhase, false);
  QCoreApplication::postEvent(widget, wheel);
}

void MouseController::clickOnObject(
  QWidget *widget, Qt::MouseButton button, bool doubleClick) {
  QPoint pos = widget->rect().center();
  QPoint globalPos = widget->mapToGlobal(pos);

  click(globalPos.x(), globalPos.y(), button, doubleClick);
}

}// namespace specter
