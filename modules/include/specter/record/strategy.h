#ifndef SPECTER_RECORD_STRATEGY_H
#define SPECTER_RECORD_STRATEGY_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QPointer>
#include <QQueue>
#include <QWidget>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

class RecordedAction;

/* ---------------------------- ActionRecordStrategy ------------------------ */

class LIB_SPECTER_API ActionRecordStrategy : public QObject {
  Q_OBJECT

public:
  explicit ActionRecordStrategy(int type, QObject *parent = nullptr);
  ~ActionRecordStrategy() override;

  void setWidget(QWidget *widget);
  [[nodiscard]] QWidget *getWidget() const;

  template<typename TYPE>
  [[nodiscard]] TYPE *getWidgetAs() const;
  [[nodiscard]] ObjectQuery getWidgetAsQuery() const;

  [[nodiscard]] int getType() const;

Q_SIGNALS:
  void actionRecorded(const RecordedAction &action);

protected:
  virtual void installConnections(QWidget *widget);
  virtual void removeConnections(QWidget *widget);

  template<typename TYPE, typename... ARGS>
  void recordAction(ARGS &&...args);

private:
  int m_type;
  QPointer<QWidget> m_widget;
};

template<typename TYPE>
TYPE *ActionRecordStrategy::getWidgetAs() const {
  const auto widget = getWidget();
  const auto specific_widget = qobject_cast<TYPE *>(widget);
  Q_ASSERT(widget == specific_widget);

  return specific_widget;
}

template<typename TYPE, typename... ARGS>
void ActionRecordStrategy::recordAction(ARGS &&...args) {
  Q_EMIT actionRecorded(TYPE{getWidgetAsQuery(), std::forward<ARGS>(args)...});
}

/* ------------------------- ActionRecordWidgetStrategy --------------------- */

class LIB_SPECTER_API ActionRecordWidgetStrategy : public ActionRecordStrategy {
  Q_OBJECT

public:
  explicit ActionRecordWidgetStrategy(QObject *parent = nullptr);
  ~ActionRecordWidgetStrategy() override;

protected:
  explicit ActionRecordWidgetStrategy(int type, QObject *parent = nullptr);

  bool eventFilter(QObject *obj, QEvent *event) override;

private Q_SLOTS:
  void onOpenContextMenu();
};

/* ------------------------- ActionRecordButtonStrategy --------------------- */

class LIB_SPECTER_API ActionRecordButtonStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  explicit ActionRecordButtonStrategy(QObject *parent = nullptr);
  ~ActionRecordButtonStrategy() override;

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private Q_SLOTS:
  void onPressed();
  void onClicked();
  void onToggled(bool checked);
};

/* ------------------------ ActionRecordComboBoxStrategy -------------------- */

class LIB_SPECTER_API ActionRecordComboBoxStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  explicit ActionRecordComboBoxStrategy(QObject *parent = nullptr);
  ~ActionRecordComboBoxStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onCurrentIndexChanged(int index);
};

/* ------------------------ ActionRecordSpinBoxStrategy --------------------- */

class LIB_SPECTER_API ActionRecordSpinBoxStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  explicit ActionRecordSpinBoxStrategy(QObject *parent = nullptr);
  ~ActionRecordSpinBoxStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onValueChanged(double value);
  void onValueChanged(int value);
};

/* ------------------------- ActionRecordSliderStrategy --------------------- */

class LIB_SPECTER_API ActionRecordSliderStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  explicit ActionRecordSliderStrategy(QObject *parent = nullptr);
  ~ActionRecordSliderStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onValueChanged(int value);
};

/* ------------------------- ActionRecordTabBarStrategy --------------------- */

class LIB_SPECTER_API ActionRecordTabBarStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  explicit ActionRecordTabBarStrategy(QObject *parent = nullptr);
  ~ActionRecordTabBarStrategy() override;

protected:
  void installConnections(QWidget *widget) override;
  void removeConnections(QWidget *widget) override;

private Q_SLOTS:
  void onCurrentChanged(int index);
  void onTabClosed(int index);
  void onTabMoved(int from, int to);

private:
  bool m_closing;
};

/* ------------------------- ActionRecordToolBoxStrategy -------------------- */

class LIB_SPECTER_API ActionRecordToolBoxStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  explicit ActionRecordToolBoxStrategy(QObject *parent = nullptr);
  ~ActionRecordToolBoxStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onCurrentChanged(int index);
};

/* -------------------------- ActionRecordMenuStrategy ---------------------- */

class LIB_SPECTER_API ActionRecordMenuStrategy : public ActionRecordStrategy {
  Q_OBJECT

public:
  explicit ActionRecordMenuStrategy(QObject *parent = nullptr);
  ~ActionRecordMenuStrategy() override;

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private Q_SLOTS:
  void onTriggered(QAction *action);
};

/* ------------------------ ActionRecordTextEditStrategy -------------------- */

class LIB_SPECTER_API ActionRecordTextEditStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  explicit ActionRecordTextEditStrategy(QObject *parent = nullptr);
  ~ActionRecordTextEditStrategy() override;

protected:
  void installConnections(QWidget *widget) override;

private Q_SLOTS:
  void onTextChanged(const QString &text);
};

/* ------------------------ ActionRecordLineEditStrategy -------------------- */

class LIB_SPECTER_API ActionRecordLineEditStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  explicit ActionRecordLineEditStrategy(QObject *parent = nullptr);
  ~ActionRecordLineEditStrategy() override;

protected:
  void installConnections(QWidget *widget) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

private Q_SLOTS:
  void onTextChanged(const QString &text);
  void onReturnPressed();
};

/* ------------------------ ActionRecordItemViewStrategy -------------------- */

class LIB_SPECTER_API ActionRecordItemViewStrategy
    : public ActionRecordWidgetStrategy {
  Q_OBJECT

public:
  explicit ActionRecordItemViewStrategy(QObject *parent = nullptr);
  ~ActionRecordItemViewStrategy() override;

protected:
  void installConnections(QWidget *widget) override;
  void removeConnections(QWidget *widget) override;

private Q_SLOTS:
  void onDataChanged(
    const QModelIndex &topLeft, const QModelIndex &bottomRight,
    const QList<int> &roles);
};

}// namespace specter

#endif// SPECTER_RECORD_STRATEGY_H