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

  void keyPress(Qt::Key key, Qt::KeyboardModifiers mods = Qt::NoModifier);
  void keyRelease(Qt::Key key, Qt::KeyboardModifiers mods = Qt::NoModifier);
  void typeText(const QString &text);

  void typeTextIntoObject(QWidget *widget, const QString &text);
};

}// namespace specter

#endif// SPECTER_INPUT_KEYBOARD_H