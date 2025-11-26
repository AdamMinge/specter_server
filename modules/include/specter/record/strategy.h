#ifndef SPECTER_RECORD_STRATEGY_H
#define SPECTER_RECORD_STRATEGY_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QPointer>
#include <QQueue>
#include <QWidget>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/id.h"
#include "specter/search/query.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSlider>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QSpinBox>
#include <QTabBar>
#include <QTextEdit>
#include <QToolBox>
#include <QToolButton>
/* -------------------------------------------------------------------------- */

namespace specter {

class RecordedAction;

template<typename T>
concept QObjectPtrConcept =
  std::is_base_of_v<QObject, std::remove_pointer_t<T>>;

/* ---------------------------- ActionRecordStrategy ------------------------ */

class LIB_SPECTER_API ActionRecordStrategy : public QObject {
  Q_OBJECT

public:
  explicit ActionRecordStrategy(QObject *parent = nullptr);
  ~ActionRecordStrategy() override;

  void handleEvent(QObject *object, QEvent *event);

  template<typename TYPE>
  [[nodiscard]] TYPE *getWidgetAs(QWidget *widget) const;

  [[nodiscard]] ObjectQuery getObjectAsQuery(QObject *object) const;
  [[nodiscard]] ObjectId getObjectAsId(QObject *object) const;

Q_SIGNALS:
  void actionReported(const RecordedAction &action);

protected:
  virtual void handleEvent(QWidget *widget, QEvent *event);
  virtual void installConnections(QWidget *widget);
  virtual void removeConnections(QWidget *widget);

  template<typename TYPE, QObjectPtrConcept OBJECT_PTR_TYPE, typename... ARGS>
  void reportAction(OBJECT_PTR_TYPE object, ARGS &&...args);

private:
  QVector<QWidget *> m_widgets;
};

template<typename TYPE>
TYPE *ActionRecordStrategy::getWidgetAs(QWidget *widget) const {
  const auto specific_widget = qobject_cast<TYPE *>(widget);
  Q_ASSERT(widget == specific_widget);
  return specific_widget;
}

template<typename TYPE, QObjectPtrConcept OBJECT_PTR_TYPE, typename... ARGS>
void ActionRecordStrategy::reportAction(
  OBJECT_PTR_TYPE object, ARGS &&...args) {
  Q_EMIT actionReported(TYPE{
    getObjectAsQuery(object), getObjectAsId(object),
    std::forward<ARGS>(args)...});
}

/* ------------------------- ActionRecordWidgetStrategy --------------------- */

class LIB_SPECTER_API ActionRecordWidgetStrategy : public ActionRecordStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordWidgetStrategy(QObject *parent = nullptr);
  ~ActionRecordWidgetStrategy() override;

protected:
  void handleEvent(QWidget *widget, QEvent *event) override;

private Q_SLOTS:
  void onOpenContextMenu(QWidget *widget);
  void onClosed(QWidget *widget);
  void onWindowStateChanged(QWidget *widget, Qt::WindowStates newStates);
};

/* ------------------------- ActionRecordButtonStrategy --------------------- */

class LIB_SPECTER_API ActionRecordButtonStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordButtonStrategy(QObject *parent = nullptr);
  ~ActionRecordButtonStrategy() override;

protected:
  void installConnections(QWidget *widget);

private Q_SLOTS:
  void onPressed(QAbstractButton *button);
  void onClicked(QAbstractButton *button);
  void onToggled(QAbstractButton *button, bool checked);
};

/* ------------------------ ActionRecordComboBoxStrategy -------------------- */

class LIB_SPECTER_API ActionRecordComboBoxStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordComboBoxStrategy(QObject *parent = nullptr);
  ~ActionRecordComboBoxStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onCurrentIndexChanged(QComboBox *combobox, int index);
};

/* ------------------------ ActionRecordSpinBoxStrategy --------------------- */

class LIB_SPECTER_API ActionRecordSpinBoxStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordSpinBoxStrategy(QObject *parent = nullptr);
  ~ActionRecordSpinBoxStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onValueChanged(QDoubleSpinBox *spinbox, double value);
  void onValueChanged(QSpinBox *spinbox, int value);
};

/* ------------------------- ActionRecordSliderStrategy --------------------- */

class LIB_SPECTER_API ActionRecordSliderStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordSliderStrategy(QObject *parent = nullptr);
  ~ActionRecordSliderStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onValueChanged(QAbstractSlider *slider, int value);
};

/* ------------------------- ActionRecordTabBarStrategy --------------------- */

class LIB_SPECTER_API ActionRecordTabBarStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordTabBarStrategy(QObject *parent = nullptr);
  ~ActionRecordTabBarStrategy() override;

protected:
  void installConnections(QWidget *widget) override;
  void removeConnections(QWidget *widget) override;

private Q_SLOTS:
  void onCurrentChanged(QTabBar *tabbar, int index);
  void onTabClosed(QTabBar *tabbar, int index);
  void onTabMoved(QTabBar *tabbar, int from, int to);

private:
  bool m_closing;
};

/* ------------------------- ActionRecordToolBoxStrategy -------------------- */

class LIB_SPECTER_API ActionRecordToolBoxStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordToolBoxStrategy(QObject *parent = nullptr);
  ~ActionRecordToolBoxStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onCurrentChanged(QToolBox *toolbox, int index);
};

/* -------------------------- ActionRecordMenuStrategy ---------------------- */

class LIB_SPECTER_API ActionRecordMenuStrategy : public ActionRecordStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordMenuStrategy(QObject *parent = nullptr);
  ~ActionRecordMenuStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onTriggered(QAction *action);
  void onHovered(QAction *action);

private:
  QAction *m_lastHovered;
};

/* ------------------------- ActionRecordMenuBarStrategy -------------------- */

class LIB_SPECTER_API ActionRecordMenuBarStrategy
    : public ActionRecordStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordMenuBarStrategy(QObject *parent = nullptr);
  ~ActionRecordMenuBarStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onTriggered(QAction *action);
  void onHovered(QAction *action);

private:
  QAction *m_lastHovered;
};

/* ------------------------ ActionRecordTextEditStrategy -------------------- */

class LIB_SPECTER_API ActionRecordTextEditStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordTextEditStrategy(QObject *parent = nullptr);
  ~ActionRecordTextEditStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onTextChanged(QTextEdit *textedit, const QString &text);
};

/* ------------------------ ActionRecordLineEditStrategy -------------------- */

class LIB_SPECTER_API ActionRecordLineEditStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordLineEditStrategy(QObject *parent = nullptr);
  ~ActionRecordLineEditStrategy() override;

protected:
  void handleEvent(QWidget *widget, QEvent *event) override;
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onTextChanged(QLineEdit *lineedit, const QString &text);
  void onReturnPressed(QLineEdit *lineedit);
};

/* ------------------------ ActionRecordItemViewStrategy -------------------- */

class LIB_SPECTER_API ActionRecordItemViewStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  [[nodiscard]] static int getType();

public:
  explicit ActionRecordItemViewStrategy(QObject *parent = nullptr);
  ~ActionRecordItemViewStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onDataChanged(
    QAbstractItemView *itemview, const QModelIndex &topLeft,
    const QModelIndex &bottomRight, const QList<int> &roles);
};

}// namespace specter

#endif// SPECTER_RECORD_STRATEGY_H