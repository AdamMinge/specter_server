/* ----------------------------------- Local -------------------------------- */
#include "specter/mark/marker.h"

#include "specter/mark/widget_marker.h"
#include "specter/mark/widget_tooltip.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
#include <QMouseEvent>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ MarkerListener --------------------------- */

MarkerListener::MarkerListener(QObject *parent)
    : QObject(parent), m_current_widget(nullptr) {}

MarkerListener::~MarkerListener() = default;

bool MarkerListener::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseMove) {
    const auto mouse_event = static_cast<QMouseEvent *>(event);

    const auto global_pos = mouse_event->globalPosition().toPoint();
    const auto widget = qApp->widgetAt(global_pos);

    Q_EMIT mouseMoved(global_pos);

    if (m_current_widget != widget) {
      m_current_widget = widget;
      Q_EMIT currentWidgetChanged(widget);
    }
  }

  return QObject::eventFilter(obj, event);
}

/* ---------------------------------- Marker ------------------------------- */

Marker::Marker(QObject *parent)
    : QObject(parent), m_marking(false), m_tooltip(new MarkerWidgetTooltip),
      m_marker(new MarkerWidgetMarker), m_listener(new MarkerListener) {
  connect(
    m_listener.get(), &MarkerListener::currentWidgetChanged, this,
    &Marker::onCurrentWidgetChanged);
  connect(
    m_listener.get(), &MarkerListener::mouseMoved, this, &Marker::onMouseMoved);
}

Marker::~Marker() = default;

void Marker::start() {
  if (m_marking) return;
  m_marking = true;

  qApp->installEventFilter(m_listener.get());

  m_tooltip->show();
  m_marker->show();
}

void Marker::stop() {
  if (!m_marking) return;
  m_marking = false;

  qApp->removeEventFilter(m_listener.get());

  m_tooltip->hide();
  m_marker->hide();
}

bool Marker::isMarking() { return m_marking; }

QColor Marker::getMarkerColor() const { return m_marker->getColor(); }

void Marker::setMarkerColor(QColor color) { m_marker->setColor(color); }

void Marker::onCurrentWidgetChanged(QWidget *widget) {
  m_tooltip->setWidget(widget);
  m_marker->setWidget(widget);
}

void Marker::onMouseMoved(const QPoint &position) {
  m_tooltip->move(position + QPoint(10, 10));
}

}// namespace specter
