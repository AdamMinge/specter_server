/* ----------------------------------- Local -------------------------------- */
#include "specter/observe/preview/observer.h"

#include "specter/module.h"
#include "specter/search/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
#include <QBuffer>
#include <QWidget>
/* --------------------------------- Standard ------------------------------- */
#include <queue>
#include <set>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ PreviewObserver --------------------------- */

PreviewObserver::PreviewObserver()
    : m_object(nullptr), m_observing(false), m_check_timer(new QTimer(this)) {
  m_check_timer->setInterval(100);
  connect(
    m_check_timer, &QTimer::timeout, this, &PreviewObserver::checkForChanges);
}

PreviewObserver::~PreviewObserver() { stop(); }

void PreviewObserver::setObject(QObject *object) {
  if (m_object != object) { m_object = object; }
}

QObject *PreviewObserver::getObject() const { return m_object; }

void PreviewObserver::start() {
  if (m_observing) return;

  m_observing = true;
  m_check_timer->start();
}

void PreviewObserver::stop() {
  if (!m_observing) return;

  m_observing = false;
  m_check_timer->stop();
}

bool PreviewObserver::isObserving() const { return m_observing; }

void PreviewObserver::checkForChanges() {
  auto getImageData = [this]() -> QByteArray {
    if (!m_object) { return {}; }

    QWidget *widget = qobject_cast<QWidget *>(m_object);
    if (!widget) { return {}; }

    if (!widget->isVisible() || widget->size().isEmpty()) { return {}; }

    QPixmap pixmap = widget->grab();
    if (pixmap.isNull()) { return {}; }

    QByteArray imageData;
    QBuffer buffer(&imageData);
    if (!buffer.open(QIODevice::WriteOnly)) { return {}; }

    bool success = pixmap.save(&buffer, "PNG");
    if (!success) { return {}; }

    return imageData;
  };

  Q_EMIT previewReported(getImageData());
}

/* ---------------------------- PreviewObserverQueue ----------------------- */

PreviewObserverQueue::PreviewObserverQueue() : m_observer(nullptr) {}

PreviewObserverQueue::~PreviewObserverQueue() = default;

void PreviewObserverQueue::setObserver(PreviewObserver *observer) {
  if (m_observer) {
    m_observer->disconnect(m_on_preview_reported);
    m_observed_previews.clear();
  }

  m_observer = observer;

  if (m_observer) {
    m_on_preview_reported = QObject::connect(
      m_observer, &PreviewObserver::previewReported,
      [this](const auto recorder_action) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_observed_previews.push_back(recorder_action);
        }

        m_cv.notify_one();
      });
  }
}

bool PreviewObserverQueue::isEmpty() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_observed_previews.empty();
}

QByteArray PreviewObserverQueue::popPreview() {
  std::lock_guard<std::mutex> lock(m_mutex);
  Q_ASSERT(!m_observed_previews.empty());
  return m_observed_previews.takeFirst();
}

QByteArray PreviewObserverQueue::waitPopPreview() {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_cv.wait(lock, [this] { return !m_observed_previews.empty(); });

  return m_observed_previews.takeFirst();
}

}// namespace specter
