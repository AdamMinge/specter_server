/* ----------------------------------- Local -------------------------------- */
#include "specter/input/keyboard.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QWidget>
/* -------------------------------------------------------------------------- */

namespace specter {

KeyboardController::KeyboardController() = default;

KeyboardController::~KeyboardController() = default;

void KeyboardController::pressKey(Qt::Key key, Qt::KeyboardModifiers mods) {
  auto press = new QKeyEvent(QEvent::KeyPress, key, mods);
  QCoreApplication::postEvent(QApplication::focusWidget(), press);
}

void KeyboardController::releaseKey(Qt::Key key, Qt::KeyboardModifiers mods) {
  auto release = new QKeyEvent(QEvent::KeyRelease, key, mods);
  QCoreApplication::postEvent(QApplication::focusWidget(), release);
}

void KeyboardController::tapKey(Qt::Key key, Qt::KeyboardModifiers mods) {
  pressKey(key, mods);
  releaseKey(key, mods);
}

void KeyboardController::enterText(const QString &text) {
  auto widget = QApplication::focusWidget();
  enterTextIntoObject(widget, text);
}

void KeyboardController::enterTextIntoObject(
  QWidget *widget, const QString &text) {
  if (!widget) return;

  if (auto line_edit = qobject_cast<QLineEdit *>(widget)) {
    line_edit->setText(text);
  } else if (auto text_edit = qobject_cast<QTextEdit *>(widget)) {
    text_edit->setPlainText(text);
  } else if (auto plain_edit = qobject_cast<QPlainTextEdit *>(widget)) {
    plain_edit->setPlainText(text);
  } else {
    for (auto c : text) {
      auto key_press = new QKeyEvent(
        QEvent::KeyPress, c.unicode(), Qt::NoModifier, QString(c));
      QCoreApplication::postEvent(widget, key_press);

      auto key_release = new QKeyEvent(
        QEvent::KeyRelease, c.unicode(), Qt::NoModifier, QString(c));
      QCoreApplication::postEvent(widget, key_release);
    }
  }
}

}// namespace specter
