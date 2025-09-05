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

  void move(const QPoint &pos);
  void click(const QPoint &pos, Qt::MouseButton button, bool doubleClick);
  void scroll(const QPoint &delta);

  void clickOnObject(
    QWidget *widget, const QPoint &pos, Qt::MouseButton button,
    bool doubleClick);
  void moveOverObject(QWidget *widget, const QPoint &pos);
};

}// namespace specter

#endif// SPECTER_INPUT_MOUSE_H