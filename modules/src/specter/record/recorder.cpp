/* ----------------------------------- Local -------------------------------- */
#include "specter/record/recorder.h"

#include "specter/module.h"
#include "specter/record/strategy.h"
#include "specter/search/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QMouseEvent>
#include <QPushButton>
#include <QTabBar>
/* -------------------------------------------------------------------------- */

namespace specter {

static constexpr qint64 USER_EVENT_WINDOW_MS = 500;
static constexpr qint64 STALE_STRATEGY_TIMEOUT_MS = 30'000;

/* ------------------------------- ActionRecorder --------------------------- */

ActionRecorder::ActionRecorder(QObject *parent)
    : QObject(parent), m_recording(false), m_cleanup_timer(new QTimer(this)) {
  registerStrategy<ActionRecordWidgetStrategy>();
  registerStrategy<ActionRecordButtonStrategy>();
  registerStrategy<ActionRecordComboBoxStrategy>();
  registerStrategy<ActionRecordSpinBoxStrategy>();
  registerStrategy<ActionRecordSliderStrategy>();
  registerStrategy<ActionRecordTabBarStrategy>();
  registerStrategy<ActionRecordToolBoxStrategy>();
  registerStrategy<ActionRecordMenuStrategy>();
  registerStrategy<ActionRecordMenuBarStrategy>();
  registerStrategy<ActionRecordTextEditStrategy>();
  registerStrategy<ActionRecordLineEditStrategy>();
  registerStrategy<ActionRecordItemViewStrategy>();

  connect(
    m_cleanup_timer, &QTimer::timeout, this,
    &ActionRecorder::cleanupStaleStrategies);
}

ActionRecorder::~ActionRecorder() { stop(); }

void ActionRecorder::start() {
  if (m_recording) return;
  m_recording = true;

  qApp->installEventFilter(this);
  m_cleanup_timer->start();
}

void ActionRecorder::stop() {
  if (!m_recording) return;
  m_recording = false;

  qApp->removeEventFilter(this);
  m_cleanup_timer->stop();
}

bool ActionRecorder::isRecording() const { return m_recording; }

bool ActionRecorder::eventFilter(QObject *object, QEvent *event) {
  switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::Wheel:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::InputMethod: {
      if (event->spontaneous()) {
        if (auto widget = qobject_cast<QWidget *>(object)) {
          m_last_user_events[widget] = QDateTime::currentMSecsSinceEpoch();

          if (!m_strategies.contains(widget)) {
            if (auto strategy = createStrategy(widget)) {
              strategy->setWidget(widget);
              m_strategies[widget] = strategy;

              connect(
                strategy, &ActionRecordStrategy::actionReported, this,
                [this, strategy](auto action) {
                  if (hadRecentUserEvent(strategy->getWidget())) {
                    Q_EMIT actionReported(action);
                  }
                });
            }
          }
        }
      }
      break;
    }
    case QEvent::Destroy: {
      if (auto widget = qobject_cast<QWidget *>(object); widget) {
        removeStrategy(widget);
        m_last_user_events.erase(widget);
      }
      break;
    }
  }

  return QObject::eventFilter(object, event);
}

void ActionRecorder::cleanupStaleStrategies() {
  const auto now = QDateTime::currentMSecsSinceEpoch();

  for (auto it = m_last_user_events.begin(); it != m_last_user_events.end();) {
    QWidget *widget = it->first;
    qint64 last_event = it->second;

    if ((now - last_event) > STALE_STRATEGY_TIMEOUT_MS) {
      removeStrategy(widget);
      it = m_last_user_events.erase(it);
    } else {
      ++it;
    }
  }
}

bool ActionRecorder::hadRecentUserEvent(QWidget *widget) const {
  if (!widget) return false;
  auto it = m_last_user_events.find(widget);
  if (it == m_last_user_events.end()) return false;
  return (QDateTime::currentMSecsSinceEpoch() - it->second) <=
         USER_EVENT_WINDOW_MS;
}

ActionRecordStrategy *ActionRecorder::createStrategy(QWidget *widget) {
  if (!widget) return nullptr;

  auto meta_object = widget->metaObject();
  while (meta_object) {
    const auto type_id = meta_object->metaType().id();
    auto it = m_strategies_factories.find(type_id);
    if (it != m_strategies_factories.end()) { return (it->second)(); }
    meta_object = meta_object->superClass();
  }

  return nullptr;
}

void ActionRecorder::removeStrategy(QWidget *widget) {
  if (m_strategies.contains(widget)) {
    m_strategies.at(widget)->deleteLater();
    m_strategies.erase(widget);
  }
}

/* ----------------------------- ActionRecorderQueue ------------------------ */

ActionRecorderQueue::ActionRecorderQueue() : m_recorder(nullptr) {}

ActionRecorderQueue::~ActionRecorderQueue() = default;

void ActionRecorderQueue::setRecorder(ActionRecorder *recorder) {
  if (m_recorder) {
    m_recorder->disconnect(m_on_action_reported);
    m_recorded_actions.clear();
  }

  m_recorder = recorder;

  if (m_recorder) {
    m_on_action_reported = QObject::connect(
      m_recorder, &ActionRecorder::actionReported,
      [this](const auto recorder_action) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_recorded_actions.push_back(recorder_action);
        }

        m_cv.notify_one();
      });
  }
}

bool ActionRecorderQueue::isEmpty() const { return m_recorded_actions.empty(); }

RecordedAction ActionRecorderQueue::popAction() {
  std::lock_guard<std::mutex> lock(m_mutex);
  Q_ASSERT(!m_recorded_actions.empty());
  return m_recorded_actions.takeFirst();
}

RecordedAction ActionRecorderQueue::waitPopAction() {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_cv.wait(lock, [this] { return !m_recorded_actions.empty(); });

  return m_recorded_actions.takeFirst();
}

}// namespace specter
