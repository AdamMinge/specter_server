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
#include "specter/record/strategy.h"
/* -------------------------------------------------------------------------- */

namespace specter {

class ActionRecordStrategy;

/* ---------------------------- ActionStrategyManager ----------------------- */

class LIB_SPECTER_API StrategyManager : public QObject {
  Q_OBJECT

public:
  explicit StrategyManager(QObject *parent = nullptr);
  ~StrategyManager();

  void handleEvent(QObject *object, QEvent *event);

Q_SIGNALS:
  void actionRecorded(const RecordedAction &action);

private:
  template<typename STRATEGY>
  void registerStrategy();

  [[nodiscard]] ActionRecordStrategy *findStrategy(QObject *object) const;

private:
  std::unordered_map<int, ActionRecordStrategy *> m_strategies;
};

template<typename STRATEGY>
void StrategyManager::registerStrategy() {
  auto [iter, inserted] = m_strategies.insert(
    std::make_pair(STRATEGY::getType(), new STRATEGY(this)));
  Q_ASSERT(inserted);

  connect(
    iter->second, &ActionRecordStrategy::actionReported, this,
    &StrategyManager::actionRecorded);
}

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

private:
  StrategyManager *m_strategy_manager;
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