#ifndef SPECTER_OBSERVE_PROPERTY_OBSERVER_H
#define SPECTER_OBSERVE_PROPERTY_OBSERVER_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QQueue>
#include <QTimer>
/* ---------------------------------- Standard ------------------------------ */
#include <condition_variable>
#include <mutex>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/observe/property/action.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ PropertyObserver -------------------------- */

class LIB_SPECTER_API PropertyObserver : public QObject {
  Q_OBJECT

public:
  explicit PropertyObserver();
  ~PropertyObserver() override;

  void setObject(QObject *object);
  QObject *getObject() const;

  void start();
  void stop();

  [[nodiscard]] bool isObserving() const;

Q_SIGNALS:
  void actionReported(const PropertyObservedAction &action);

private:
  void startChangesTracker();
  void stopChangesTracker();
  void checkForChanges();

  [[nodiscard]] std::map<QString, QVariant> getTrackedProperties() const;
  [[nodiscard]] bool variantEqual(const QVariant &a, const QVariant &b) const;

private:
  QPointer<QObject> m_object;
  bool m_observing;
  QTimer *m_check_timer;
  std::map<QString, QVariant> m_tracked_properties;
};

/* --------------------------- PropertyObserverQueue ---------------------- */

class LIB_SPECTER_API PropertyObserverQueue {
public:
  explicit PropertyObserverQueue();
  ~PropertyObserverQueue();

  void setObserver(PropertyObserver *observer);

  [[nodiscard]] bool isEmpty() const;
  [[nodiscard]] PropertyObservedAction popAction();
  [[nodiscard]] PropertyObservedAction waitPopAction();

private:
  PropertyObserver *m_observer;
  QMetaObject::Connection m_on_action_reported;
  QQueue<PropertyObservedAction> m_observed_actions;

  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};

}// namespace specter

#endif// SPECTER_OBSERVE_PROPERTY_OBSERVER_H