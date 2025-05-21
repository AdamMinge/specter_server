#ifndef SPECTER_RECORD_RECORDER_H
#define SPECTER_RECORD_RECORDER_H

/* --------------------------------- Standard ------------------------------- */
#include <unordered_map>
/* ------------------------------------ Qt ---------------------------------- */
#include <QQueue>
#include <QScopedPointer>
#include <QThread>
#include <QWidget>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/record/action.h"
/* -------------------------------------------------------------------------- */

namespace specter {

class ActionRecordStrategy;

/* ------------------------ ActionRecorderWidgetListener -------------------- */

class LIB_SPECTER_API ActionRecorderWidgetListener : public QObject {
  Q_OBJECT

public:
  explicit ActionRecorderWidgetListener(QObject *parent = nullptr);
  ~ActionRecorderWidgetListener() override;

Q_SIGNALS:
  void currentWidgetChanged(QWidget *widget);

protected Q_SLOTS:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void setWidget(QWidget *widget);
  [[nodiscard]] QWidget *findWidget(QWidget *widget) const;

private:
  QWidget *m_current_widget;
};

/* ------------------------------- ActionRecorder --------------------------- */

class LIB_SPECTER_API ActionRecorder : public QObject {
  Q_OBJECT

public:
  explicit ActionRecorder(QObject *parent = nullptr);
  ~ActionRecorder();

  void start();
  void stop();

  [[nodiscard]] bool isRecording() const;

  bool addStrategy(ActionRecordStrategy *strategy);

Q_SIGNALS:
  void actionReported(const RecordedAction &action);

protected Q_SLOTS:
  void onCurrentWidgetChanged(QWidget *widget);

private:
  [[nodiscard]] ActionRecordStrategy *findStrategy(QWidget *widget) const;

private:
  std::unordered_map<int, ActionRecordStrategy *> m_strategies;
  ActionRecordStrategy *m_current_strategy;
  ActionRecorderWidgetListener *m_widget_listener;
  bool m_recording;
};

/* ----------------------------- ActionRecorderQueue ------------------------ */

class LIB_SPECTER_API ActionRecorderQueue {
public:
  explicit ActionRecorderQueue();
  ~ActionRecorderQueue();

  void setRecorder(ActionRecorder *recorder);

  [[nodiscard]] bool isEmpty() const;
  [[nodiscard]] RecordedAction popAction();
  [[nodiscard]] RecordedAction waitPopAction();

private:
  ActionRecorder *m_recorder;
  QMetaObject::Connection m_on_action_reported;
  QQueue<RecordedAction> m_recorded_actions;

  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};

}// namespace specter

#endif// SPECTER_RECORD_RECORDER_H