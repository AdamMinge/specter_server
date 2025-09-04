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

  void move(int x, int y);
  void click(int x, int y, Qt::MouseButton button, bool doubleClick = false);
  void scroll(int deltaX, int deltaY);

  void clickOnObject(
    QWidget *widget, Qt::MouseButton button, bool doubleClick = false);
};

}// namespace specter

#endif// SPECTER_INPUT_MOUSE_H