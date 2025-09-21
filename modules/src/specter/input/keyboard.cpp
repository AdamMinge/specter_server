/* ----------------------------------- Local -------------------------------- */
#include "specter/input/keyboard.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QKeyEvent>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QWidget>
/* -------------------------------------------------------------------------- */

namespace specter {

KeyboardController::KeyboardController() = default;

KeyboardController::~KeyboardController() = default;

void KeyboardController::pressKey(
  QWidget *target, Qt::Key key, Qt::KeyboardModifiers mods) {
  if (!target) return;

  auto press = new QKeyEvent(QEvent::KeyPress, key, mods);
  QCoreApplication::postEvent(target, press);
}

void KeyboardController::releaseKey(
  QWidget *target, Qt::Key key, Qt::KeyboardModifiers mods) {
  if (!target) return;

  auto release = new QKeyEvent(QEvent::KeyRelease, key, mods);
  QCoreApplication::postEvent(target, release);
}

void KeyboardController::tapKey(
  QWidget *target, Qt::Key key, Qt::KeyboardModifiers mods) {
  if (!target) return;

  pressKey(target, key, mods);
  releaseKey(target, key, mods);
}

void KeyboardController::enterText(QWidget *target, const QString &text) {
  if (!target) return;

  if (auto line_edit = qobject_cast<QLineEdit *>(target)) {
    line_edit->setText(text);
  } else if (auto text_edit = qobject_cast<QTextEdit *>(target)) {
    text_edit->setPlainText(text);
  } else if (auto plain_edit = qobject_cast<QPlainTextEdit *>(target)) {
    plain_edit->setPlainText(text);
  } else {
    for (auto c : text) {
      auto key_press = new QKeyEvent(
        QEvent::KeyPress, c.unicode(), Qt::NoModifier, QString(c));
      QCoreApplication::postEvent(target, key_press);

      auto key_release = new QKeyEvent(
        QEvent::KeyRelease, c.unicode(), Qt::NoModifier, QString(c));
      QCoreApplication::postEvent(target, key_release);
    }
  }
}

}// namespace specter
