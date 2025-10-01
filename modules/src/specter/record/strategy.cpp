/* ----------------------------------- Local -------------------------------- */
#include "specter/record/strategy.h"

#include "specter/module.h"
#include "specter/record/action.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSlider>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QSpinBox>
#include <QTabBar>
#include <QTextEdit>
#include <QToolBox>
#include <QToolButton>
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

ActionRecordStrategy::ActionRecordStrategy(QObject *parent)
    : QObject(parent), m_widget(nullptr) {}

ActionRecordStrategy::~ActionRecordStrategy() = default;

void ActionRecordStrategy::setWidget(QWidget *widget) {
  if (m_widget == widget) return;

  if (m_widget) {
    removeConnections(m_widget);
    m_widget->removeEventFilter(this);
  }

  m_widget = widget;

  if (m_widget) {
    installConnections(m_widget);
    m_widget->installEventFilter(this);
  }
}

QWidget *ActionRecordStrategy::getWidget() const { return m_widget; }

ObjectQuery ActionRecordStrategy::getObjectAsQuery(QObject *object) const {
  const auto query = searcher().getQuery(object);
  return query;
}

ObjectId ActionRecordStrategy::getObjectAsId(QObject *object) const {
  const auto id = searcher().getId(object);
  return id;
}

void ActionRecordStrategy::installConnections(QWidget *widget) {
  Q_UNUSED(widget);
}

void ActionRecordStrategy::removeConnections(QWidget *widget) {
  m_widget->disconnect(this);
}

/* ------------------------- ActionRecordWidgetStrategy --------------------- */

int ActionRecordWidgetStrategy::getType() { return qMetaTypeId<QWidget>(); }

ActionRecordWidgetStrategy::ActionRecordWidgetStrategy(QObject *parent)
    : ActionRecordStrategy(parent) {}

ActionRecordWidgetStrategy::~ActionRecordWidgetStrategy() = default;

bool ActionRecordWidgetStrategy::eventFilter(QObject *obj, QEvent *event) {
  if (auto widget = getWidget(); widget == obj) {
    switch (event->type()) {
      case QEvent::ContextMenu:
        onOpenContextMenu();
        break;
      case QEvent::Close:
        onClosed();
        break;
      case QEvent::WindowStateChange: {
        auto stateEvent = static_cast<QWindowStateChangeEvent *>(event);
        onWindowStateChanged(widget->windowState());
        break;
      }
    }
  }

  return ActionRecordStrategy::eventFilter(obj, event);
}

void ActionRecordWidgetStrategy::onOpenContextMenu() {
  reportAction<RecordedAction::ContextMenuOpened>();
}

void ActionRecordWidgetStrategy::onClosed() {
  reportAction<RecordedAction::WindowClosed>();
}

void ActionRecordWidgetStrategy::onWindowStateChanged(
  Qt::WindowStates newState) {
  if (newState == Qt::WindowMinimized) {
    reportAction<RecordedAction::WindowMinimized>();
  } else if (newState == Qt::WindowMaximized) {
    reportAction<RecordedAction::WindowMaximized>();
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
  auto button = qobject_cast<QAbstractButton *>(widget);
  Q_ASSERT(button);

  connect(
    button, &QAbstractButton::pressed, this,
    &ActionRecordButtonStrategy::onPressed);
}

void ActionRecordButtonStrategy::onPressed() {
  const auto button = getWidgetAs<QAbstractButton>();

  if (button->isCheckable()) {
    onToggled(!button->isChecked());
  } else {
    onClicked();
  }
}

void ActionRecordButtonStrategy::onClicked() {
  reportAction<RecordedAction::ButtonClicked>();
}

void ActionRecordButtonStrategy::onToggled(bool checked) {
  reportAction<RecordedAction::ButtonToggled>(checked);
}

/* ------------------------ ActionRecordComboBoxStrategy -------------------- */

int ActionRecordComboBoxStrategy::getType() { return qMetaTypeId<QComboBox>(); }

ActionRecordComboBoxStrategy::ActionRecordComboBoxStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordComboBoxStrategy::~ActionRecordComboBoxStrategy() = default;

void ActionRecordComboBoxStrategy::installConnections(QWidget *widget) {
  auto combobox = qobject_cast<QComboBox *>(widget);
  Q_ASSERT(combobox);

  connect(
    combobox, &QComboBox::currentIndexChanged, this,
    &ActionRecordComboBoxStrategy::onCurrentIndexChanged);
}

void ActionRecordComboBoxStrategy::onCurrentIndexChanged(int index) {
  reportAction<RecordedAction::ComboBoxCurrentChanged>(index);
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
      spinbox, &QSpinBox::valueChanged, this,
      QOverload<int>::of(&ActionRecordSpinBoxStrategy::onValueChanged));
  } else if (auto spinbox = qobject_cast<QDoubleSpinBox *>(widget); spinbox) {
    connect(
      spinbox, &QDoubleSpinBox::valueChanged, this,
      QOverload<double>::of(&ActionRecordSpinBoxStrategy::onValueChanged));
  } else {
    Q_ASSERT(false);
  }
}

void ActionRecordSpinBoxStrategy::onValueChanged(double value) {
  reportAction<RecordedAction::DoubleSpinBoxValueChanged>(value);
}

void ActionRecordSpinBoxStrategy::onValueChanged(int value) {
  reportAction<RecordedAction::SpinBoxValueChanged>(value);
}

/* ------------------------- ActionRecordSliderStrategy --------------------- */

int ActionRecordSliderStrategy::getType() {
  return qMetaTypeId<QAbstractSlider>();
}

ActionRecordSliderStrategy::ActionRecordSliderStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordSliderStrategy::~ActionRecordSliderStrategy() = default;

void ActionRecordSliderStrategy::installConnections(QWidget *widget) {
  auto slider = qobject_cast<QAbstractSlider *>(widget);
  Q_ASSERT(slider);

  connect(
    slider, &QAbstractSlider::valueChanged, this,
    &ActionRecordSliderStrategy::onValueChanged);
}

void ActionRecordSliderStrategy::onValueChanged(int value) {
  reportAction<RecordedAction::SliderValueChanged>(value);
}

/* ------------------------- ActionRecordTabBarStrategy --------------------- */

int ActionRecordTabBarStrategy::getType() { return qMetaTypeId<QTabBar>(); }

ActionRecordTabBarStrategy::ActionRecordTabBarStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent), m_closing(false) {}

ActionRecordTabBarStrategy::~ActionRecordTabBarStrategy() = default;

void ActionRecordTabBarStrategy::installConnections(QWidget *widget) {
  auto tabbar = qobject_cast<QTabBar *>(widget);
  Q_ASSERT(tabbar);

  connect(
    tabbar, &QTabBar::tabMoved, this, &ActionRecordTabBarStrategy::onTabMoved);

  connect(tabbar, &QTabBar::currentChanged, this, [this](auto i) {
    if (!m_closing) onCurrentChanged(i);
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
          onTabClosed(i);
        });
    }
  }
}

void ActionRecordTabBarStrategy::removeConnections(QWidget *widget) {
  ActionRecordWidgetStrategy::removeConnections(widget);

  auto tabbar = qobject_cast<QTabBar *>(widget);
  Q_ASSERT(tabbar);

  if (tabbar->tabsClosable()) {
    for (auto i = 0; i < tabbar->count(); ++i) {
      const auto button = qobject_cast<QAbstractButton *>(
        tabbar->tabButton(i, QTabBar::RightSide));

      button->disconnect(this);
    }
  }
}

void ActionRecordTabBarStrategy::onCurrentChanged(int index) {
  reportAction<RecordedAction::TabCurrentChanged>(index);
}

void ActionRecordTabBarStrategy::onTabClosed(int index) {
  reportAction<RecordedAction::TabClosed>(index);
}

void ActionRecordTabBarStrategy::onTabMoved(int from, int to) {
  reportAction<RecordedAction::TabMoved>(from, to);
}

/* ------------------------- ActionRecordToolBoxStrategy -------------------- */

int ActionRecordToolBoxStrategy::getType() { return qMetaTypeId<QToolBox>(); }

ActionRecordToolBoxStrategy::ActionRecordToolBoxStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordToolBoxStrategy::~ActionRecordToolBoxStrategy() = default;

void ActionRecordToolBoxStrategy::installConnections(QWidget *widget) {
  auto toolbox = qobject_cast<QToolBox *>(widget);
  Q_ASSERT(toolbox);

  connect(
    toolbox, &QToolBox::currentChanged, this,
    &ActionRecordToolBoxStrategy::onCurrentChanged);
}

void ActionRecordToolBoxStrategy::onCurrentChanged(int index) {
  reportAction<RecordedAction::ToolBoxCurrentChanged>(index);
}

/* ------------------------- ActionRecordMenuStrategy ---------------------- */

int ActionRecordMenuStrategy::getType() { return qMetaTypeId<QMenu>(); }

ActionRecordMenuStrategy::ActionRecordMenuStrategy(QObject *parent)
    : ActionRecordStrategy(parent), m_lastHovered(nullptr) {}

ActionRecordMenuStrategy::~ActionRecordMenuStrategy() = default;

void ActionRecordMenuStrategy::installConnections(QWidget *widget) {
  auto menu = qobject_cast<QMenu *>(widget);
  Q_ASSERT(menu);

  connect(
    menu, &QMenu::triggered, this, &ActionRecordMenuStrategy::onTriggered);

  connect(menu, &QMenu::hovered, this, &ActionRecordMenuStrategy::onHovered);
}

void ActionRecordMenuStrategy::onTriggered(QAction *action) {
  if (!action || !action->isEnabled()) return;
  if (action->menu()) return;

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
  auto menubar = qobject_cast<QMenuBar *>(widget);
  Q_ASSERT(menubar);

  connect(
    menubar, &QMenuBar::triggered, this,
    &ActionRecordMenuBarStrategy::onTriggered);

  connect(
    menubar, &QMenuBar::hovered, this, &ActionRecordMenuBarStrategy::onHovered);
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
  auto textedit = qobject_cast<QTextEdit *>(widget);
  Q_ASSERT(textedit);

  connect(textedit, &QTextEdit::textChanged, this, [this, textedit]() {
    onTextChanged(textedit->toPlainText());
  });
}

void ActionRecordTextEditStrategy::onTextChanged(const QString &text) {
  reportAction<RecordedAction::TextEditTextChanged>(text);
}

/* ------------------------ ActionRecordLineEditStrategy -------------------- */

int ActionRecordLineEditStrategy::getType() { return qMetaTypeId<QLineEdit>(); }

ActionRecordLineEditStrategy::ActionRecordLineEditStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordLineEditStrategy::~ActionRecordLineEditStrategy() = default;

void ActionRecordLineEditStrategy::installConnections(QWidget *widget) {
  auto lineedit = qobject_cast<QLineEdit *>(widget);
  Q_ASSERT(lineedit);

  connect(
    lineedit, &QLineEdit::textChanged, this,
    &ActionRecordLineEditStrategy::onTextChanged);
}

bool ActionRecordLineEditStrategy::eventFilter(QObject *obj, QEvent *event) {
  if (auto widget = getWidget(); widget == obj) {
    switch (event->type()) {
      case QEvent::KeyPress: {
        const auto key_event = static_cast<QKeyEvent *>(event);
        const auto pressed_key = key_event->key();
        if (pressed_key != Qt::Key_Return && pressed_key != Qt::Key_Enter)
          break;

        onReturnPressed();

        break;
      }
    }
  }

  return ActionRecordWidgetStrategy::eventFilter(obj, event);
}

void ActionRecordLineEditStrategy::onTextChanged(const QString &text) {
  reportAction<RecordedAction::LineEditTextChanged>(text);
}

void ActionRecordLineEditStrategy::onReturnPressed() {
  reportAction<RecordedAction::LineEditReturnPressed>();
}

/* ------------------------ ActionRecordItemViewStrategy -------------------- */

int ActionRecordItemViewStrategy::getType() {
  return qMetaTypeId<QAbstractItemView>();
}

ActionRecordItemViewStrategy::ActionRecordItemViewStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(parent) {}

ActionRecordItemViewStrategy::~ActionRecordItemViewStrategy() = default;

void ActionRecordItemViewStrategy::installConnections(QWidget *widget) {
  auto itemview = qobject_cast<QAbstractItemView *>(widget);
  Q_ASSERT(itemview);

  auto model = itemview->model();
  if (model) {
    connect(
      model, &QAbstractItemModel::dataChanged, this,
      &ActionRecordItemViewStrategy::onDataChanged);
  }
}

void ActionRecordItemViewStrategy::onDataChanged(
  const QModelIndex &topLeft, const QModelIndex &bottomRight,
  const QList<int> &roles) {
  /* TODO */
}

}// namespace specter
