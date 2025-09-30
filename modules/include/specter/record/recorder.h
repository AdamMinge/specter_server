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

/* ------------------------------- ActionRecorder --------------------------- */

class LIB_SPECTER_API ActionRecorder : public QObject {
  Q_OBJECT

public:
  explicit ActionRecorder(QObject *parent = nullptr);
  ~ActionRecorder();

  void start();
  void stop();

  [[nodiscard]] bool isRecording() const;

Q_SIGNALS:
  void actionReported(const RecordedAction &action);

protected:
  bool eventFilter(QObject *object, QEvent *event) override;

private Q_SLOTS:
  void cleanupStaleStrategies();

private:
  template<typename STRATEGY>
  void registerStrategy();

  [[nodiscard]] bool hadRecentUserEvent(QWidget *widget) const;

  [[nodiscard]] ActionRecordStrategy *createStrategy(QWidget *widget);
  void removeStrategy(QWidget *widget);

private:
  using StrategyFactory = std::function<ActionRecordStrategy *()>;

private:
  std::unordered_map<int, StrategyFactory> m_strategies_factories;
  std::unordered_map<QWidget *, ActionRecordStrategy *> m_strategies;
  std::unordered_map<QWidget *, qint64> m_last_user_events;
  QTimer *m_cleanup_timer;
  bool m_recording;
};

template<typename STRATEGY>
void ActionRecorder::registerStrategy() {
  m_strategies_factories.insert(
    std::make_pair(STRATEGY::getType(), [this]() -> ActionRecordStrategy * {
      return new STRATEGY(this);
    }));
}

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