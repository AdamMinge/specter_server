/* ----------------------------------- Local -------------------------------- */
#include "specter/service/recorder.h"

#include "specter/module.h"
#include "specter/record/action.h"
#include "specter/record/recorder.h"
#include "specter/record/strategy.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
/* -------------------------------------------------------------------------- */

namespace specter {

/* --------------------------- RecordedActionsMapper ------------------------ */

class RecordedActionsMapper {
public:
  specter_proto::RecorderCommand
  operator()(const RecordedAction::ContextMenuOpened &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"ContextMenuOpened %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ButtonClicked &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"ButtonClicked %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ButtonToggled &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"ButtonToggled %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ComboBoxCurrentChanged &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"ComboBoxCurrentChanged %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::SpinBoxValueChanged &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"SpinBoxValueChanged %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::DoubleSpinBoxValueChanged &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"DoubleSpinBoxValueChanged %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::SliderValueChanged &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"SliderValueChanged %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::TabCurrentChanged &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"TabCurrentChanged %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::TabClosed &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"TabClosed %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::TabMoved &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"TabMoved %1"}.arg(action.object.toString()).toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ToolBoxCurrentChanged &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"ToolBoxCurrentChanged %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ActionTriggered &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"ActionTriggered %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::TextEditTextChanged &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"TextEditTextChanged %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::LineEditTextChanged &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"LineEditTextChanged %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::LineEditReturnPressed &action) const {
    auto response = specter_proto::RecorderCommand{};
    response.set_command(
      QLatin1String{"LineEditReturnPressed %1"}
        .arg(action.object.toString())
        .toStdString());
    return response;
  }
};

/* ----------------------------- RecorderListenCommandsCall ------------------------ */

RecorderListenCommandsCall::RecorderListenCommandsCall(
  specter_proto::RecorderService::AsyncService *service,
  grpc::ServerCompletionQueue *queue)
    : StreamCallData(
        service, queue, CallTag{this},
        &specter_proto::RecorderService::AsyncService::RequestListenCommands),
      m_recorder(std::make_unique<ActionRecorder>()),
      m_recorder_queue(std::make_unique<ActionRecorderQueue>()),
      m_mapper(std::make_unique<RecordedActionsMapper>()) {
  m_recorder->addStrategy(new ActionRecordWidgetStrategy());
  m_recorder->addStrategy(new ActionRecordButtonStrategy());
  m_recorder->addStrategy(new ActionRecordComboBoxStrategy());
  m_recorder->addStrategy(new ActionRecordSpinBoxStrategy());
  m_recorder->addStrategy(new ActionRecordSliderStrategy());
  m_recorder->addStrategy(new ActionRecordTabBarStrategy());
  m_recorder->addStrategy(new ActionRecordToolBoxStrategy());
  m_recorder->addStrategy(new ActionRecordMenuStrategy());
  m_recorder->addStrategy(new ActionRecordTextEditStrategy());
  m_recorder->addStrategy(new ActionRecordLineEditStrategy());
  m_recorder->addStrategy(new ActionRecordItemViewStrategy());
  m_recorder->addStrategy(new ActionRecordButtonStrategy());

  m_recorder_queue->setRecorder(m_recorder.get());
}

RecorderListenCommandsCall::~RecorderListenCommandsCall() = default;

std::unique_ptr<RecorderListenCommandsCallData>
RecorderListenCommandsCall::clone() const {
  return std::make_unique<RecorderListenCommandsCall>(getService(), getQueue());
}

RecorderListenCommandsCall::ProcessResult
RecorderListenCommandsCall::process(const Request &request) const {
  if (!m_recorder->isRecording()) { m_recorder->start(); }
  if (m_recorder_queue->isEmpty()) return {};

  const auto recorded_action = m_recorder_queue->popAction();
  const auto response = recorded_action.visit(*m_mapper);
  return response;
}

/* ------------------------------- RecorderService -------------------------- */

RecorderService::RecorderService() = default;

RecorderService::~RecorderService() = default;

void RecorderService::start(grpc::ServerCompletionQueue *queue) {
  auto listen_call = new RecorderListenCommandsCall(this, queue);
  listen_call->proceed();
}

}// namespace specter
