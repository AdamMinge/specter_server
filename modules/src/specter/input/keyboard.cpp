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

void KeyboardController::keyPress(Qt::Key key, Qt::KeyboardModifiers mods) {
  auto press = new QKeyEvent(QEvent::KeyPress, key, mods);
  QCoreApplication::postEvent(QApplication::focusWidget(), press);
}

void KeyboardController::keyRelease(Qt::Key key, Qt::KeyboardModifiers mods) {
  auto release = new QKeyEvent(QEvent::KeyRelease, key, mods);
  QCoreApplication::postEvent(QApplication::focusWidget(), release);
}

void KeyboardController::typeText(const QString &text) {
  auto target = QApplication::focusWidget();
  if (!target) return;

  for (auto c : text) {
    auto keyPress =
      new QKeyEvent(QEvent::KeyPress, c.unicode(), Qt::NoModifier, QString(c));
    QCoreApplication::postEvent(target, keyPress);

    auto keyRelease = new QKeyEvent(
      QEvent::KeyRelease, c.unicode(), Qt::NoModifier, QString(c));
    QCoreApplication::postEvent(target, keyRelease);
  }
}

void KeyboardController::typeIntoObject(QWidget *widget, const QString &text) {
  if (auto lineEdit = qobject_cast<QLineEdit *>(widget)) {
    lineEdit->setText(text);
  } else if (auto textEdit = qobject_cast<QTextEdit *>(widget)) {
    textEdit->setPlainText(text);
  } else if (auto plainEdit = qobject_cast<QPlainTextEdit *>(widget)) {
    plainEdit->setPlainText(text);
  } else {
    widget->setFocus();
    typeText(text);
  }
}

}// namespace specter
