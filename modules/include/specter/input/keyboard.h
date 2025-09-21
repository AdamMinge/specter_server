#ifndef SPECTER_INPUT_KEYBOARD_H
#define SPECTER_INPUT_KEYBOARD_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QObject>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace specter {

class LIB_SPECTER_API KeyboardController : public QObject {
  Q_OBJECT

public:
  explicit KeyboardController();
  ~KeyboardController() override;

  void pressKey(
    QWidget *target, Qt::Key key, Qt::KeyboardModifiers mods = Qt::NoModifier);
  void releaseKey(
    QWidget *target, Qt::Key key, Qt::KeyboardModifiers mods = Qt::NoModifier);
  void tapKey(
    QWidget *target, Qt::Key key, Qt::KeyboardModifiers mods = Qt::NoModifier);

  void enterText(QWidget *target, const QString &text);
};

}// namespace specter

#endif// SPECTER_INPUT_KEYBOARD_H