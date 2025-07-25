#ifndef SPECTER_OBSERVE_PREVIEW_OBSERVER_H
#define SPECTER_OBSERVE_PREVIEW_OBSERVER_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QByteArray>
#include <QObject>
#include <QPointer>
#include <QQueue>
#include <QTimer>
/* ---------------------------------- Standard ------------------------------ */
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------- PreviewObserver -------------------------- */

class LIB_SPECTER_API PreviewObserver : public QObject {
  Q_OBJECT

public:
  explicit PreviewObserver();
  ~PreviewObserver() override;

  void setObject(QObject *object);
  QObject *getObject() const;

  void start();
  void stop();

  [[nodiscard]] bool isObserving() const;

Q_SIGNALS:
  void previewReported(const QByteArray &preview);

private:
  void checkForChanges();

private:
  QPointer<QObject> m_object;
  bool m_observing;
  QTimer *m_check_timer;
};

/* ---------------------------- PreviewObserverQueue ---------------------- */

class LIB_SPECTER_API PreviewObserverQueue {
public:
  explicit PreviewObserverQueue();
  ~PreviewObserverQueue();

  void setObserver(PreviewObserver *observer);

  [[nodiscard]] bool isEmpty() const;
  [[nodiscard]] QByteArray popPreview();
  [[nodiscard]] QByteArray waitPopPreview();

private:
  PreviewObserver *m_observer;
  QMetaObject::Connection m_on_preview_reported;
  QQueue<QByteArray> m_observed_previews;

  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};

}// namespace specter

#endif// SPECTER_OBSERVE_PREVIEW_OBSERVER_H