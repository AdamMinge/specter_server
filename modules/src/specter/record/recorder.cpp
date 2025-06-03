/* ----------------------------------- Local -------------------------------- */
#include "specter/record/recorder.h"

#include "specter/module.h"
#include "specter/record/strategy.h"
#include "specter/thread/invoke.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QTabBar>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------ Find Exceptions --------------------------- */

[[nodiscard]] QWidget *findScrollAreaException(QWidget *widget) {
  const auto is_scrollarea_viewport =
    widget->objectName() == QLatin1String("qt_scrollarea_viewport");
  if (is_scrollarea_viewport) widget = widget->parentWidget();

  return widget;
}

[[nodiscard]] QWidget *findComboBoxException(QWidget *widget) {
  const auto class_name = widget->metaObject()->className();
  const auto is_combobox_internal =
    class_name == QLatin1String("QComboBoxPrivateContainer") ||
    class_name == QLatin1String("QComboBoxListView");

  while (is_combobox_internal) {
    if (auto combobox = qobject_cast<QComboBox *>(widget); combobox) break;

    widget = widget->parentWidget();
    Q_ASSERT(widget);
  };

  return widget;
}

[[nodiscard]] QWidget *findTabBarException(QWidget *widget) {
  const auto is_close_button =
    widget->metaObject()->className() == QLatin1String("CloseButton");

  if (is_close_button) {
    auto tabbar = qobject_cast<QTabBar *>(widget->parentWidget());
    widget = tabbar ? tabbar : widget;
  }

  return widget;
}

/* ------------------------ ActionRecorderWidgetListener -------------------- */

ActionRecorderWidgetListener::ActionRecorderWidgetListener(QObject *parent)
    : QObject(parent), m_current_widget(nullptr) {}

ActionRecorderWidgetListener::~ActionRecorderWidgetListener() = default;

bool ActionRecorderWidgetListener::eventFilter(QObject *obj, QEvent *event) {
  switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
      auto widget = findWidget(qobject_cast<QWidget *>(obj));
      if (widget && widget->hasFocus()) setWidget(widget);
      break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick: {
      auto widget = findWidget(qobject_cast<QWidget *>(obj));
      setWidget(widget);
      break;
    }
    case QEvent::Destroy: {
      auto widget = findWidget(qobject_cast<QWidget *>(obj));
      if (widget == m_current_widget) setWidget(nullptr);
      break;
    }
  }

  return QObject::eventFilter(obj, event);
}

void ActionRecorderWidgetListener::setWidget(QWidget *widget) {
  if (m_current_widget != widget) {
    m_current_widget = widget;
    Q_EMIT currentWidgetChanged(widget);
  }
}

QWidget *ActionRecorderWidgetListener::findWidget(QWidget *widget) const {
  static const auto find_exceptions = {
    &findScrollAreaException, &findComboBoxException, &findTabBarException};

  if (widget) {
    for (const auto find_exception : find_exceptions) {
      widget = find_exception(widget);
    }
  }

  return widget;
}

/* ------------------------------- ActionRecorder --------------------------- */

ActionRecorder::ActionRecorder(QObject *parent)
    : QObject(parent), m_current_strategy(nullptr),
      m_widget_listener(new ActionRecorderWidgetListener(this)),
      m_recording(false) {
  connect(
    m_widget_listener, &ActionRecorderWidgetListener::currentWidgetChanged,
    this, &ActionRecorder::onCurrentWidgetChanged);
}

ActionRecorder::~ActionRecorder() { stop(); }

void ActionRecorder::start() {
  InvokeInObjectThread(qApp, [this]() {
    if (m_recording) return;
    m_recording = true;

    qApp->installEventFilter(m_widget_listener);
  });
}

void ActionRecorder::stop() {
  InvokeInObjectThread(qApp, [this]() {
    if (!m_recording) return;
    m_recording = false;

    qApp->removeEventFilter(m_widget_listener);
    onCurrentWidgetChanged(nullptr);
  });
}

bool ActionRecorder::isRecording() const { return m_recording; }

bool ActionRecorder::addStrategy(ActionRecordStrategy *strategy) {
  if (m_strategies.contains(strategy->getType())) return false;

  strategy->setParent(this);
  m_strategies.insert(std::make_pair(strategy->getType(), strategy));
  return true;
}

void ActionRecorder::onCurrentWidgetChanged(QWidget *widget) {
  auto strategy = findStrategy(widget);

  if (m_current_strategy) {
    m_current_strategy->setWidget(nullptr);
    disconnect(
      m_current_strategy, &ActionRecordStrategy::actionRecorded, this,
      &ActionRecorder::actionReported);
  }

  m_current_strategy = strategy;

  if (m_current_strategy) {
    m_current_strategy->setWidget(widget);
    connect(
      m_current_strategy, &ActionRecordStrategy::actionRecorded, this,
      &ActionRecorder::actionReported);
  }
}

ActionRecordStrategy *ActionRecorder::findStrategy(QWidget *widget) const {
  auto meta_object = widget ? widget->metaObject() : nullptr;
  while (meta_object) {
    const auto type_id = meta_object->metaType().id();
    auto found_strategy = m_strategies.find(type_id);
    if (found_strategy != m_strategies.end()) { return found_strategy->second; }

    meta_object = meta_object->superClass();
  }

  return nullptr;
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
