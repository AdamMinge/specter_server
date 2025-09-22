#ifndef SPECTER_MARK_MARKER_H
#define SPECTER_MARK_MARKER_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QByteArray>
#include <QEvent>
#include <QObject>
#include <QQueue>
/* ---------------------------------- Standard ------------------------------ */
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/id.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ MarkerListener --------------------------- */

class LIB_SPECTER_API MarkerListener : public QObject {
  Q_OBJECT

public:
  explicit MarkerListener(QObject *parent = nullptr);
  ~MarkerListener() override;

Q_SIGNALS:
  void currentWidgetChanged(QWidget *widget);
  void mouseMoved(const QPoint &position);

protected Q_SLOTS:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  QWidget *m_current_widget;
};

/* ---------------------------------- Marker ------------------------------- */

class MarkerWidgetTooltip;
class MarkerWidgetMarker;

class LIB_SPECTER_API Marker : public QObject {
  Q_OBJECT

public:
  Marker(QObject *parent = nullptr);
  ~Marker();

  void start();
  void stop();

  [[nodiscard]] bool isMarking();

  [[nodiscard]] QColor getMarkerColor() const;
  void setMarkerColor(QColor color);

Q_SIGNALS:
  void currentWidgetChanged(QWidget *widget);

protected Q_SLOTS:
  void onCurrentWidgetChanged(QWidget *widget);
  void onMouseMoved(const QPoint &position);

private:
  bool m_marking;
  QScopedPointer<MarkerWidgetTooltip> m_tooltip;
  QScopedPointer<MarkerWidgetMarker> m_marker;
  QScopedPointer<MarkerListener> m_listener;
};

/* ---------------------------- MarkerObserverQueue ----------------------- */

class LIB_SPECTER_API MarkerObserverQueue {
public:
  explicit MarkerObserverQueue();
  ~MarkerObserverQueue();

  void setObserver(Marker *observer);

  [[nodiscard]] bool isEmpty() const;
  [[nodiscard]] ObjectId popPreview();
  [[nodiscard]] ObjectId waitPopPreview();

private:
  Marker *m_observer;

  QMetaObject::Connection m_on_selection_changed;
  QQueue<ObjectId> m_observed_selection;

  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};

}// namespace specter

#endif// SPECTER_MARK_MARKER_H