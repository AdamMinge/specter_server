/* ----------------------------------- Local -------------------------------- */
#include "specter/record/recorder.h"

#include "specter/module.h"
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

/* ---------------------------- ActionStrategyManager ----------------------- */

StrategyManager::StrategyManager(QObject *parent) : QObject(parent) {
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
}

StrategyManager::~StrategyManager() = default;

void StrategyManager::handleEvent(QObject *object, QEvent *event) {
  if (auto strategy = findStrategy(object))
    strategy->handleEvent(object, event);
}

ActionRecordStrategy *StrategyManager::findStrategy(QObject *object) const {
  if (!object) return nullptr;

  auto meta_object = object->metaObject();
  while (meta_object) {
    const auto type_id = meta_object->metaType().id();
    auto it = m_strategies.find(type_id);
    if (it != m_strategies.end()) { return it->second; }
    meta_object = meta_object->superClass();
  }

  return nullptr;
}

/* ------------------------------- ActionRecorder --------------------------- */

ActionRecorder::ActionRecorder(QObject *parent)
    : QObject(parent), m_strategy_manager(new StrategyManager(this)),
      m_recording(false) {}

ActionRecorder::~ActionRecorder() { stop(); }

void ActionRecorder::start() {
  if (m_recording) return;
  m_recording = true;

  qApp->installEventFilter(this);
  connect(
    m_strategy_manager, &StrategyManager::actionRecorded, this,
    &ActionRecorder::actionReported);
}

void ActionRecorder::stop() {
  if (!m_recording) return;
  m_recording = false;

  qApp->removeEventFilter(this);
  disconnect(
    m_strategy_manager, &StrategyManager::actionRecorded, this,
    &ActionRecorder::actionReported);
}

bool ActionRecorder::isRecording() const { return m_recording; }

bool ActionRecorder::eventFilter(QObject *object, QEvent *event) {
  m_strategy_manager->handleEvent(object, event);
  return QObject::eventFilter(object, event);
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
