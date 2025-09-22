/* ----------------------------------- Local -------------------------------- */
#include "specter/mark/marker.h"

#include "specter/mark/widget_marker.h"
#include "specter/mark/widget_tooltip.h"
#include "specter/module.h"
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

  Q_EMIT currentWidgetChanged(widget);
}

void Marker::onMouseMoved(const QPoint &position) {
  m_tooltip->move(position + QPoint(10, 10));
}

/* ---------------------------- MarkerObserverQueue ----------------------- */

MarkerObserverQueue::MarkerObserverQueue() {}

MarkerObserverQueue::~MarkerObserverQueue() = default;

void MarkerObserverQueue::setObserver(Marker *observer) {
  if (m_observer) {
    m_observer->disconnect(m_on_selection_changed);
    m_observed_selection.clear();
  }

  m_observer = observer;

  if (m_observer) {
    m_on_selection_changed = QObject::connect(
      m_observer, &Marker::currentWidgetChanged, [this](const auto widget) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_observed_selection.push_back(searcher().getId(widget));
        }

        m_cv.notify_one();
      });
  }
}

bool MarkerObserverQueue::isEmpty() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_observed_selection.empty();
}

ObjectId MarkerObserverQueue::popPreview() {
  std::lock_guard<std::mutex> lock(m_mutex);
  Q_ASSERT(!m_observed_selection.empty());
  return m_observed_selection.takeFirst();
}

ObjectId MarkerObserverQueue::waitPopPreview() {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_cv.wait(lock, [this] { return !m_observed_selection.empty(); });

  return m_observed_selection.takeFirst();
}

}// namespace specter
