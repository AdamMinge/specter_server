#ifndef SPECTER_MARK_MARKER_H
#define SPECTER_MARK_MARKER_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QEvent>
#include <QObject>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
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

 protected Q_SLOTS:
  void onCurrentWidgetChanged(QWidget *widget);
  void onMouseMoved(const QPoint &position);

 private:
  bool m_marking;
  QScopedPointer<MarkerWidgetTooltip> m_tooltip;
  QScopedPointer<MarkerWidgetMarker> m_marker;
  QScopedPointer<MarkerListener> m_listener;
};

}  // namespace specter

#endif  // SPECTER_MARK_MARKER_H