/* ----------------------------------- Local -------------------------------- */
#include "specter/service/recorder.h"

#include "specter/module.h"
#include "specter/record/action.h"
#include "specter/record/recorder.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
/* -------------------------------------------------------------------------- */

namespace specter {

/* --------------------------- RecordedActionsMapper ------------------------ */

class RecordedActionsMapper {
public:
  specter_proto::RecorderCommand
  operator()(const RecordedAction::ContextMenuOpened &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_context_menu_opened();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ButtonClicked &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_button_clicked();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ButtonToggled &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_button_toggled();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_checked(action.checked);
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ComboBoxCurrentChanged &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_combo_box_current_changed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_index(action.index);
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::SpinBoxValueChanged &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_spin_box_value_changed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_value(action.value);
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::DoubleSpinBoxValueChanged &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_double_spin_box_value_changed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_value(action.value);
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::SliderValueChanged &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_slider_value_changed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_value(action.value);
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::TabCurrentChanged &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_tab_current_changed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_index(action.index);
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::TabClosed &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_tab_closed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_index(action.index);
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::TabMoved &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_tab_moved();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_from(action.from);
    ev->set_to(action.to);
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ToolBoxCurrentChanged &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_tool_box_current_changed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_index(action.index);
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ActionTriggered &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_action_triggered();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::ActionHovered &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_action_hovered();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::TextEditTextChanged &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_text_edit_text_changed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_value(action.value.toStdString());
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::LineEditTextChanged &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_line_edit_text_changed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    ev->set_value(action.value.toStdString());
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::LineEditReturnPressed &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_line_edit_return_pressed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::WindowClosed &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_window_closed();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::WindowMinimized &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_window_minimized();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    return cmd;
  }

  specter_proto::RecorderCommand
  operator()(const RecordedAction::WindowMaximized &action) const {
    auto cmd = specter_proto::RecorderCommand{};
    auto ev = cmd.mutable_window_maximized();
    ev->mutable_object_query()->set_query(
      action.object.toString().toStdString());
    return cmd;
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
  m_recorder_queue->setRecorder(m_recorder.get());
}

RecorderListenCommandsCall::~RecorderListenCommandsCall() = default;

std::unique_ptr<RecorderListenCommandsCallData>
RecorderListenCommandsCall::clone() const {
  return std::make_unique<RecorderListenCommandsCall>(getService(), getQueue());
}

RecorderListenCommandsCall::StartResult
RecorderListenCommandsCall::start(const Request &request) const {
  m_recorder->start();
  return {};
}

RecorderListenCommandsCall::ProcessResult
RecorderListenCommandsCall::process() const {
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
