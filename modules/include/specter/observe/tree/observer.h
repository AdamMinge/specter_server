#ifndef SPECTER_OBSERVE_OBSERVER_H
#define SPECTER_OBSERVE_OBSERVER_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QHash>
#include <QObject>
#include <QTimer>
/* ---------------------------------- Standard ------------------------------ */
#include <condition_variable>
#include <mutex>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/observe/tree/action.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* -------------------------------- TreeObserver ---------------------------- */

class LIB_SPECTER_API TreeObserver : public QObject {
  Q_OBJECT

public:
  explicit TreeObserver();
  ~TreeObserver() override;

  void start();
  void stop();

  [[nodiscard]] bool isObserving() const;

Q_SIGNALS:
  void actionReported(const TreeObservedAction &action);

protected:
  [[nodiscard]] bool eventFilter(QObject *object, QEvent *event) override;

private:
  void startRenameTracker();
  void stopRenameTracker();
  void checkForRenames();

private:
  bool m_observing;
  QTimer *m_check_timer;
  mutable std::mutex m_mutex;
  QMap<QObject *, ObjectQuery> m_tracked_objects;
};

/* ----------------------------- TreeObserverQueue ------------------------ */

class LIB_SPECTER_API TreeObserverQueue {
public:
  explicit TreeObserverQueue();
  ~TreeObserverQueue();

  void setObserver(TreeObserver *observer);

  [[nodiscard]] bool isEmpty() const;
  [[nodiscard]] TreeObservedAction popAction();
  [[nodiscard]] TreeObservedAction waitPopAction();

private:
  TreeObserver *m_observer;
  QMetaObject::Connection m_on_action_reported;
  QQueue<TreeObservedAction> m_observed_actions;

  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};

}// namespace specter

#endif// SPECTER_OBSERVE_OBSERVER_H