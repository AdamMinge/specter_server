/* ----------------------------------- Local -------------------------------- */
#include "specter/record/strategy.h"

#include "specter/module.h"
#include "specter/record/action.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ----------------------------------- Utils -------------------------------- */

[[nodiscard]] int findTabIndex(QTabBar *tabbar, QAbstractButton *button) {
  if (tabbar->tabsClosable()) {
    for (auto i = 0; i < tabbar->count(); ++i) {
      const auto current_button = qobject_cast<QAbstractButton *>(
        tabbar->tabButton(i, QTabBar::RightSide));

      if (button == current_button) return i;
    }
  }

  return -1;
}

/* ---------------------------- ActionRecordStrategy ------------------------ */

ActionRecordStrategy::ActionRecordStrategy(QObject *parent) : QObject(parent) {}

ActionRecordStrategy::~ActionRecordStrategy() = default;

void ActionRecordStrategy::handleEvent(QObject *object, QEvent *event) {
  auto widget = qobject_cast<QWidget *>(object);
  Q_ASSERT(widget);

  if (!m_widgets.contains(object)) {
    m_widgets.push_back(widget);
    installConnections(widget);

    connect(widget, &QObject::destroyed, this, [this, widget]() {
      m_widgets.removeOne(widget);
      removeConnections(widget);
    });
  }

  switch (event->type()) {
    case QEvent::MouseButtonPress:
      setState(widget, InteractionState::UserPressed);
      break;

    case QEvent::KeyPress:
    case QEvent::ShortcutOverride:
      setState(widget, InteractionState::UserTyping);
      break;

    case QEvent::Wheel:
      setState(widget, InteractionState::UserScrolling);
      break;

    case QEvent::InputMethod:
      setState(widget, InteractionState::UserTyping);
      break;

    case QEvent::FocusOut:
      setState(widget, InteractionState::None);
      break;

    default:
      break;
  }

  processEvent(widget, event);
}

ObjectQuery ActionRecordStrategy::getObjectAsQuery(QObject *object) const {
  const auto query = searcher().getQuery(object);
  return query;
}

void ActionRecordStrategy::processEvent(QWidget *widget, QEvent *event) {
  Q_UNUSED(widget);
  Q_UNUSED(event);
}

void ActionRecordStrategy::installConnections(QWidget *widget) {
  Q_UNUSED(widget);
}

void ActionRecordStrategy::removeConnections(QWidget *widget) {
  widget->disconnect(this);
}

bool ActionRecordStrategy::userInitiated(QWidget *widget) {
  const bool ok = (state(widget) != InteractionState::None);
  if (ok) setState(widget, InteractionState::None);
  return ok;
}

ActionRecordStrategy::InteractionState
ActionRecordStrategy::state(QWidget *widget) const {
  return m_state.value(widget, InteractionState::None);
}

void ActionRecordStrategy::setState(QWidget *widget, InteractionState state) {
  if (state == InteractionState::None) m_state.remove(widget);
  else
    m_state[widget] = state;
}

/* ------------------------- ActionRecordWidgetStrategy --------------------- */

int ActionRecordWidgetStrategy::getType() { return qMetaTypeId<QWidget>(); }

ActionRecordWidgetStrategy::ActionRecordWidgetStrategy(QObject *parent)
    : ActionRecordStrategy(parent) {}

ActionRecordWidgetStrategy::~ActionRecordWidgetStrategy() = default;

void ActionRecordWidgetStrategy::processEvent(QWidget *widget, QEvent *event) {
  ActionRecordStrategy::processEvent(widget, event);

  switch (event->type()) {
    case QEvent::ContextMenu:
      onOpenContextMenu(widget);
      break;
    case QEvent::Close:
      onClosed(widget);
      break;
    case QEvent::WindowStateChange: {
      auto stateEvent = static_cast<QWindowStateChangeEvent *>(event);
      onWindowStateChanged(widget, widget->windowState());
      break;
    }
  }
}

void ActionRecordWidgetStrategy::onOpenContextMenu(QWidget *widget) {
  tryReportAction<RecordedAction::ContextMenuOpened>(widget);
}

void ActionRecordWidgetStrategy::onClosed(QWidget *widget) {
  tryReportAction<RecordedAction::WindowClosed>(widget);
}

void ActionRecordWidgetStrategy::onWindowStateChanged(
  QWidget *widget, Qt::WindowStates newState) {
  if (newState == Qt::WindowMinimized) {
    tryReportAction<RecordedAction::WindowMinimized>(widget);
  } else if (newState == Qt::WindowMaximized) {
    tryReportAction<RecordedAction::WindowMaximized>(widget);
  }
}

/* ------------------------- ActionRecordButtonStrategy --------------------- */

int ActionRecordButtonStrategy::getType() {
  return qMetaTypeId<QAbstractButton>();
}

ActionRecordButtonStrategy::ActionRecordButtonStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordButtonStrategy::~ActionRecordButtonStrategy() = default;

void ActionRecordButtonStrategy::installConnections(QWidget *widget) {
  auto button = getWidgetAs<QAbstractButton>(widget);

  connect(button, &QAbstractButton::pressed, this, [this, button]() {
    onPressed(button);
  });
}

void ActionRecordButtonStrategy::onPressed(QAbstractButton *button) {
  if (button->isCheckable()) {
    onToggled(button, !button->isChecked());
  } else {
    onClicked(button);
  }
}

void ActionRecordButtonStrategy::onClicked(QAbstractButton *button) {
  tryReportAction<RecordedAction::ButtonClicked>(button);
}

void ActionRecordButtonStrategy::onToggled(
  QAbstractButton *button, bool checked) {
  tryReportAction<RecordedAction::ButtonToggled>(button, checked);
}

/* ------------------------ ActionRecordComboBoxStrategy -------------------- */

int ActionRecordComboBoxStrategy::getType() { return qMetaTypeId<QComboBox>(); }

ActionRecordComboBoxStrategy::ActionRecordComboBoxStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordComboBoxStrategy::~ActionRecordComboBoxStrategy() = default;

void ActionRecordComboBoxStrategy::installConnections(QWidget *widget) {
  auto combobox = getWidgetAs<QComboBox>(widget);

  connect(
    combobox, &QComboBox::currentIndexChanged, this,
    [this, combobox](auto &&index) {
      onCurrentIndexChanged(combobox, std::forward<decltype(index)>(index));
    });
}

void ActionRecordComboBoxStrategy::onCurrentIndexChanged(
  QComboBox *combobox, int index) {
  tryReportAction<RecordedAction::ComboBoxCurrentChanged>(combobox, index);
}

/* ------------------------ ActionRecordSpinBoxStrategy --------------------- */

int ActionRecordSpinBoxStrategy::getType() {
  return qMetaTypeId<QAbstractSpinBox>();
}

ActionRecordSpinBoxStrategy::ActionRecordSpinBoxStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordSpinBoxStrategy::~ActionRecordSpinBoxStrategy() = default;

void ActionRecordSpinBoxStrategy::installConnections(QWidget *widget) {
  if (auto spinbox = qobject_cast<QSpinBox *>(widget); spinbox) {
    connect(
      spinbox, &QSpinBox::valueChanged, this, [this, spinbox](auto &&value) {
        onValueChanged(spinbox, std::forward<decltype(value)>(value));
      });
  } else if (auto spinbox = qobject_cast<QDoubleSpinBox *>(widget); spinbox) {
    connect(
      spinbox, &QDoubleSpinBox::valueChanged, this,
      [this, spinbox](auto &&value) {
        onValueChanged(spinbox, std::forward<decltype(value)>(value));
      });
  } else {
    Q_ASSERT(false);
  }
}

void ActionRecordSpinBoxStrategy::onValueChanged(
  QDoubleSpinBox *spinbox, double value) {
  tryReportAction<RecordedAction::DoubleSpinBoxValueChanged>(spinbox, value);
}

void ActionRecordSpinBoxStrategy::onValueChanged(QSpinBox *spinbox, int value) {
  tryReportAction<RecordedAction::SpinBoxValueChanged>(spinbox, value);
}

/* ------------------------- ActionRecordSliderStrategy --------------------- */

int ActionRecordSliderStrategy::getType() {
  return qMetaTypeId<QAbstractSlider>();
}

ActionRecordSliderStrategy::ActionRecordSliderStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordSliderStrategy::~ActionRecordSliderStrategy() = default;

void ActionRecordSliderStrategy::installConnections(QWidget *widget) {
  auto slider = getWidgetAs<QAbstractSlider>(widget);

  connect(
    slider, &QAbstractSlider::valueChanged, this, [this, slider](auto &&value) {
      onValueChanged(slider, std::forward<decltype(value)>(value));
    });
}

void ActionRecordSliderStrategy::onValueChanged(
  QAbstractSlider *slider, int value) {
  tryReportAction<RecordedAction::SliderValueChanged>(slider, value);
}

/* ------------------------- ActionRecordTabBarStrategy --------------------- */

int ActionRecordTabBarStrategy::getType() { return qMetaTypeId<QTabBar>(); }

ActionRecordTabBarStrategy::ActionRecordTabBarStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent), m_closing(false) {}

ActionRecordTabBarStrategy::~ActionRecordTabBarStrategy() = default;

void ActionRecordTabBarStrategy::installConnections(QWidget *widget) {
  auto tabbar = getWidgetAs<QTabBar>(widget);

  connect(tabbar, &QTabBar::tabMoved, this, [this, tabbar](auto from, auto to) {
    onTabMoved(tabbar, from, to);
  });

  connect(tabbar, &QTabBar::currentChanged, this, [this, tabbar](auto i) {
    if (!m_closing) onCurrentChanged(tabbar, i);
    m_closing = false;
  });

  if (tabbar->tabsClosable()) {
    for (auto i = 0; i < tabbar->count(); ++i) {
      const auto button = qobject_cast<QAbstractButton *>(
        tabbar->tabButton(i, QTabBar::RightSide));

      connect(
        button, &QAbstractButton::released, this, [this, tabbar, button]() {
          m_closing = true;

          const auto i = findTabIndex(tabbar, button);
          Q_ASSERT(i >= 0);
          onTabClosed(tabbar, i);
        });
    }
  }
}

void ActionRecordTabBarStrategy::removeConnections(QWidget *widget) {
  ActionRecordWidgetStrategy::removeConnections(widget);

  auto tabbar = getWidgetAs<QTabBar>(widget);
  if (tabbar->tabsClosable()) {
    for (auto i = 0; i < tabbar->count(); ++i) {
      const auto button = qobject_cast<QAbstractButton *>(
        tabbar->tabButton(i, QTabBar::RightSide));

      button->disconnect(this);
    }
  }
}

void ActionRecordTabBarStrategy::onCurrentChanged(QTabBar *tabbar, int index) {
  tryReportAction<RecordedAction::TabCurrentChanged>(tabbar, index);
}

void ActionRecordTabBarStrategy::onTabClosed(QTabBar *tabbar, int index) {
  tryReportAction<RecordedAction::TabClosed>(tabbar, index);
}

void ActionRecordTabBarStrategy::onTabMoved(QTabBar *tabbar, int from, int to) {
  tryReportAction<RecordedAction::TabMoved>(tabbar, from, to);
}

/* ------------------------- ActionRecordToolBoxStrategy -------------------- */

int ActionRecordToolBoxStrategy::getType() { return qMetaTypeId<QToolBox>(); }

ActionRecordToolBoxStrategy::ActionRecordToolBoxStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordToolBoxStrategy::~ActionRecordToolBoxStrategy() = default;

void ActionRecordToolBoxStrategy::installConnections(QWidget *widget) {
  auto toolbox = getWidgetAs<QToolBox>(widget);

  connect(
    toolbox, &QToolBox::currentChanged, this,
    [this, toolbox](auto index) { onCurrentChanged(toolbox, index); });
}

void ActionRecordToolBoxStrategy::onCurrentChanged(
  QToolBox *toolbox, int index) {
  tryReportAction<RecordedAction::ToolBoxCurrentChanged>(toolbox, index);
}

/* ------------------------- ActionRecordMenuStrategy ---------------------- */

int ActionRecordMenuStrategy::getType() { return qMetaTypeId<QMenu>(); }

ActionRecordMenuStrategy::ActionRecordMenuStrategy(QObject *parent)
    : ActionRecordStrategy(parent), m_lastHovered(nullptr) {}

ActionRecordMenuStrategy::~ActionRecordMenuStrategy() = default;

void ActionRecordMenuStrategy::installConnections(QWidget *widget) {
  auto menu = getWidgetAs<QMenu>(widget);

  connect(menu, &QMenu::triggered, this, [this, menu](auto action) {
    onTriggered(action);
  });

  connect(menu, &QMenu::hovered, this, [this, menu](auto action) {
    onHovered(action);
  });
}

void ActionRecordMenuStrategy::onTriggered(QAction *action) {
  if (!action || !action->isEnabled()) return;
  reportAction<RecordedAction::ActionTriggered>(action);
}

void ActionRecordMenuStrategy::onHovered(QAction *action) {
  if (!action || !action->isEnabled()) return;

  if (action == m_lastHovered) return;
  m_lastHovered = action;

  reportAction<RecordedAction::ActionHovered>(action);
}

/* ------------------------- ActionRecordMenuBarStrategy -------------------- */

int ActionRecordMenuBarStrategy::getType() { return qMetaTypeId<QMenuBar>(); }

ActionRecordMenuBarStrategy::ActionRecordMenuBarStrategy(QObject *parent)
    : ActionRecordStrategy(parent) {}

ActionRecordMenuBarStrategy::~ActionRecordMenuBarStrategy() = default;

void ActionRecordMenuBarStrategy::installConnections(QWidget *widget) {
  auto menubar = getWidgetAs<QMenuBar>(widget);

  connect(menubar, &QMenuBar::triggered, this, [this](auto action) {
    onTriggered(action);
  });

  connect(menubar, &QMenuBar::hovered, this, [this](auto action) {
    onHovered(action);
  });
}

void ActionRecordMenuBarStrategy::onTriggered(QAction *action) {
  if (!action || !action->isEnabled()) return;
  reportAction<RecordedAction::ActionTriggered>(action);
}

void ActionRecordMenuBarStrategy::onHovered(QAction *action) {
  if (!action || !action->isEnabled()) return;

  if (action == m_lastHovered) return;
  m_lastHovered = action;

  reportAction<RecordedAction::ActionHovered>(action);
}

/* ------------------------ ActionRecordTextEditStrategy -------------------- */

int ActionRecordTextEditStrategy::getType() { return qMetaTypeId<QTextEdit>(); }

ActionRecordTextEditStrategy::ActionRecordTextEditStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordTextEditStrategy::~ActionRecordTextEditStrategy() = default;

void ActionRecordTextEditStrategy::installConnections(QWidget *widget) {
  auto textedit = getWidgetAs<QTextEdit>(widget);

  connect(textedit, &QTextEdit::textChanged, this, [this, textedit]() {
    onTextChanged(textedit, textedit->toPlainText());
  });
}

void ActionRecordTextEditStrategy::onTextChanged(
  QTextEdit *textedit, const QString &text) {
  tryReportAction<RecordedAction::TextEditTextChanged>(textedit, text);
}

/* ------------------------ ActionRecordLineEditStrategy -------------------- */

int ActionRecordLineEditStrategy::getType() { return qMetaTypeId<QLineEdit>(); }

ActionRecordLineEditStrategy::ActionRecordLineEditStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordLineEditStrategy::~ActionRecordLineEditStrategy() = default;

void ActionRecordLineEditStrategy::installConnections(QWidget *widget) {
  auto lineedit = getWidgetAs<QLineEdit>(widget);

  connect(
    lineedit, &QLineEdit::textChanged, this, [this, lineedit](auto &&text) {
      onTextChanged(lineedit, std::forward<decltype(text)>(text));
    });
}

void ActionRecordLineEditStrategy::processEvent(
  QWidget *widget, QEvent *event) {
  ActionRecordWidgetStrategy::processEvent(widget, event);

  switch (event->type()) {
    case QEvent::KeyPress: {
      const auto key_event = static_cast<QKeyEvent *>(event);
      const auto pressed_key = key_event->key();
      if (pressed_key != Qt::Key_Return && pressed_key != Qt::Key_Enter) break;

      auto lineedit = getWidgetAs<QLineEdit>(widget);
      onReturnPressed(lineedit);

      break;
    }
  }
}

void ActionRecordLineEditStrategy::onTextChanged(
  QLineEdit *lineedit, const QString &text) {
  tryReportAction<RecordedAction::LineEditTextChanged>(lineedit, text);
}

void ActionRecordLineEditStrategy::onReturnPressed(QLineEdit *lineedit) {
  tryReportAction<RecordedAction::LineEditReturnPressed>(lineedit);
}

/* ------------------------ ActionRecordItemViewStrategy -------------------- */

int ActionRecordItemViewStrategy::getType() {
  return qMetaTypeId<QAbstractItemView>();
}

ActionRecordItemViewStrategy::ActionRecordItemViewStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordItemViewStrategy::~ActionRecordItemViewStrategy() = default;

void ActionRecordItemViewStrategy::installConnections(QWidget *widget) {
  auto itemview = getWidgetAs<QAbstractItemView>(widget);

  auto model = itemview->model();
  if (model) {
    connect(
      model, &QAbstractItemModel::dataChanged, this,
      [this, itemview](auto &&topLeft, auto &&bottomRight, auto &&roles) {
        onDataChanged(
          itemview, std::forward<decltype(topLeft)>(topLeft),
          std::forward<decltype(bottomRight)>(bottomRight),
          std::forward<decltype(roles)>(roles));
      });
  }
}

void ActionRecordItemViewStrategy::onDataChanged(
  QAbstractItemView *itemview, const QModelIndex &topLeft,
  const QModelIndex &bottomRight, const QList<int> &roles) {
  /* TODO */
}

}// namespace specter
