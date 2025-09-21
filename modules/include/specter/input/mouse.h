#ifndef SPECTER_INPUT_MOUSE_H
#define SPECTER_INPUT_MOUSE_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QObject>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace specter {

class LIB_SPECTER_API MouseController : public QObject {
  Q_OBJECT

public:
  explicit MouseController();
  ~MouseController() override;

  void pressButton(
    QWidget *target, const QPoint &pos, Qt::MouseButton button,
    bool doubleClick);
  void
  releaseButton(QWidget *target, const QPoint &pos, Qt::MouseButton button);
  void clickButton(
    QWidget *target, const QPoint &pos, Qt::MouseButton button,
    bool doubleClick);
  void moveCursor(const QPoint &pos);
  void scrollWheel(QWidget *target, const QPoint &delta);

  void hover(QWidget *target, const QPoint &pos);
};

}// namespace specter

#endif// SPECTER_INPUT_MOUSE_H