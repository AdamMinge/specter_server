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

ActionRecordStrategy::ActionRecordStrategy(int type, QObject *parent)
    : QObject(parent), m_type(type), m_widget(nullptr) {}

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

ObjectQuery ActionRecordStrategy::getWidgetAsQuery() const {
  const auto query = searcher().getQueryUsingKinds(
    getWidget(),
    {SearchStrategy::Kind::OrderIndex, SearchStrategy::Kind::Path});
  return query;
}

ObjectId ActionRecordStrategy::getWidgetAsId() const {
  const auto id = searcher().getId(getWidget());
  return id;
}

int ActionRecordStrategy::getType() const { return m_type; }

void ActionRecordStrategy::installConnections(QWidget *widget) {
  Q_UNUSED(widget);
}

void ActionRecordStrategy::removeConnections(QWidget *widget) {
  m_widget->disconnect(this);
}

/* ------------------------- ActionRecordWidgetStrategy --------------------- */

ActionRecordWidgetStrategy::ActionRecordWidgetStrategy(QObject *parent)
    : ActionRecordStrategy(qMetaTypeId<QWidget>(), parent) {}

ActionRecordWidgetStrategy::ActionRecordWidgetStrategy(
  int type, QObject *parent)
    : ActionRecordStrategy(type, parent) {}

ActionRecordWidgetStrategy::~ActionRecordWidgetStrategy() = default;

bool ActionRecordWidgetStrategy::eventFilter(QObject *obj, QEvent *event) {
  if (auto widget = getWidget(); widget == obj) {
    switch (event->type()) {
      case QEvent::ContextMenu:
        onOpenContextMenu();
        break;
    }
  }

  return ActionRecordStrategy::eventFilter(obj, event);
}

void ActionRecordWidgetStrategy::onOpenContextMenu() {
  recordAction<RecordedAction::ContextMenuOpened>();
}

/* ------------------------- ActionRecordButtonStrategy --------------------- */

ActionRecordButtonStrategy::ActionRecordButtonStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(qMetaTypeId<QAbstractButton>(), parent) {}

ActionRecordButtonStrategy::~ActionRecordButtonStrategy() = default;

bool ActionRecordButtonStrategy::eventFilter(QObject *obj, QEvent *event) {
  if (auto button = getWidgetAs<QAbstractButton>(); button == obj) {
    switch (event->type()) {
      case QEvent::MouseButtonRelease: {
        const auto mouse_event = static_cast<QMouseEvent *>(event);
        const auto mouse_position = mouse_event->position().toPoint();
        const auto button_rect = button->rect();
        if (!button_rect.contains(mouse_position)) break;

        onPressed();

        break;
      }

      case QEvent::KeyPress: {
        const auto key_event = static_cast<QKeyEvent *>(event);
        if (
          key_event->key() == Qt::Key_Space ||
          key_event->key() == Qt::Key_Return ||
          key_event->key() == Qt::Key_Enter) {
          onPressed();
        }

        break;
      }
    }
  }

  return ActionRecordWidgetStrategy::eventFilter(obj, event);
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
  recordAction<RecordedAction::ButtonClicked>();
}

void ActionRecordButtonStrategy::onToggled(bool checked) {
  recordAction<RecordedAction::ButtonToggled>(checked);
}

/* ------------------------ ActionRecordComboBoxStrategy -------------------- */

ActionRecordComboBoxStrategy::ActionRecordComboBoxStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(qMetaTypeId<QComboBox>(), parent) {}

ActionRecordComboBoxStrategy::~ActionRecordComboBoxStrategy() = default;

void ActionRecordComboBoxStrategy::installConnections(QWidget *widget) {
  auto combobox = qobject_cast<QComboBox *>(widget);
  Q_ASSERT(combobox);

  connect(
    combobox, &QComboBox::currentIndexChanged, this,
    &ActionRecordComboBoxStrategy::onCurrentIndexChanged);
}

void ActionRecordComboBoxStrategy::onCurrentIndexChanged(int index) {
  recordAction<RecordedAction::ComboBoxCurrentChanged>(index);
}

/* ------------------------ ActionRecordSpinBoxStrategy --------------------- */

ActionRecordSpinBoxStrategy::ActionRecordSpinBoxStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(qMetaTypeId<QAbstractSpinBox>(), parent) {}

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
  recordAction<RecordedAction::DoubleSpinBoxValueChanged>(value);
}

void ActionRecordSpinBoxStrategy::onValueChanged(int value) {
  recordAction<RecordedAction::SpinBoxValueChanged>(value);
}

/* ------------------------- ActionRecordSliderStrategy --------------------- */

ActionRecordSliderStrategy::ActionRecordSliderStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(qMetaTypeId<QAbstractSlider>(), parent) {}

ActionRecordSliderStrategy::~ActionRecordSliderStrategy() = default;

void ActionRecordSliderStrategy::installConnections(QWidget *widget) {
  auto slider = qobject_cast<QAbstractSlider *>(widget);
  Q_ASSERT(slider);

  connect(
    slider, &QAbstractSlider::valueChanged, this,
    &ActionRecordSliderStrategy::onValueChanged);
}

void ActionRecordSliderStrategy::onValueChanged(int value) {
  recordAction<RecordedAction::SliderValueChanged>(value);
}

/* ------------------------- ActionRecordTabBarStrategy --------------------- */

ActionRecordTabBarStrategy::ActionRecordTabBarStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(qMetaTypeId<QTabBar>(), parent),
      m_closing(false) {}

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
  recordAction<RecordedAction::TabCurrentChanged>(index);
}

void ActionRecordTabBarStrategy::onTabClosed(int index) {
  recordAction<RecordedAction::TabClosed>(index);
}

void ActionRecordTabBarStrategy::onTabMoved(int from, int to) {
  recordAction<RecordedAction::TabMoved>(from, to);
}

/* ------------------------- ActionRecordToolBoxStrategy -------------------- */

ActionRecordToolBoxStrategy::ActionRecordToolBoxStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(qMetaTypeId<QToolBox>(), parent) {}

ActionRecordToolBoxStrategy::~ActionRecordToolBoxStrategy() = default;

void ActionRecordToolBoxStrategy::installConnections(QWidget *widget) {
  auto toolbox = qobject_cast<QToolBox *>(widget);
  Q_ASSERT(toolbox);

  connect(
    toolbox, &QToolBox::currentChanged, this,
    &ActionRecordToolBoxStrategy::onCurrentChanged);
}

void ActionRecordToolBoxStrategy::onCurrentChanged(int index) {
  recordAction<RecordedAction::ToolBoxCurrentChanged>(index);
}

/* ------------------------- ActionRecordMenuStrategy ---------------------- */

ActionRecordMenuStrategy::ActionRecordMenuStrategy(QObject *parent)
    : ActionRecordStrategy(qMetaTypeId<QMenu>(), parent) {}

ActionRecordMenuStrategy::~ActionRecordMenuStrategy() = default;

bool ActionRecordMenuStrategy::eventFilter(QObject *obj, QEvent *event) {
  const auto tryTrigger = [this](auto menu, auto action) {
    if (!action) return;

    const auto is_disabled = !action->isEnabled();
    const auto is_submenu = action->menu();
    const auto is_inactive = action != menu->activeAction();
    if (is_disabled || is_submenu || is_inactive) return;

    onTriggered(action);
  };

  if (auto menu = getWidgetAs<QMenu>(); menu == obj) {
    switch (event->type()) {
      case QEvent::KeyPress: {
        const auto key_event = static_cast<QKeyEvent *>(event);
        const auto key = key_event->key();

        if (key == Qt::Key_Enter || key == Qt::Key_Return) {
          const auto action = menu->activeAction();
          tryTrigger(menu, action);
        }

        break;
      }
      case QEvent::MouseButtonRelease: {
        const auto mouse_event = static_cast<QMouseEvent *>(event);
        const auto mouse_position = mouse_event->position().toPoint();
        const auto action = menu->actionAt(mouse_position);

        tryTrigger(menu, action);

        break;
      }
    }
  }

  return ActionRecordStrategy::eventFilter(obj, event);
}

void ActionRecordMenuStrategy::onTriggered(QAction *action) { /* TODO */ }

/* ------------------------ ActionRecordTextEditStrategy -------------------- */

ActionRecordTextEditStrategy::ActionRecordTextEditStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(qMetaTypeId<QTextEdit>(), parent) {}

ActionRecordTextEditStrategy::~ActionRecordTextEditStrategy() = default;

void ActionRecordTextEditStrategy::installConnections(QWidget *widget) {
  auto textedit = qobject_cast<QTextEdit *>(widget);
  Q_ASSERT(textedit);

  connect(textedit, &QTextEdit::textChanged, this, [this, textedit]() {
    onTextChanged(textedit->toPlainText());
  });
}

void ActionRecordTextEditStrategy::onTextChanged(const QString &text) {
  recordAction<RecordedAction::TextEditTextChanged>(text);
}

/* ------------------------ ActionRecordLineEditStrategy -------------------- */

ActionRecordLineEditStrategy::ActionRecordLineEditStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(qMetaTypeId<QLineEdit>(), parent) {}

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
  recordAction<RecordedAction::LineEditTextChanged>(text);
}

void ActionRecordLineEditStrategy::onReturnPressed() {
  recordAction<RecordedAction::LineEditReturnPressed>();
}

/* ------------------------ ActionRecordItemViewStrategy -------------------- */

ActionRecordItemViewStrategy::ActionRecordItemViewStrategy(QObject *parent)
    : ActionRecordWidgetStrategy(qMetaTypeId<QAbstractItemView>(), parent) {}

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

void ActionRecordItemViewStrategy::removeConnections(QWidget *widget) {
  auto itemview = qobject_cast<QAbstractItemView *>(widget);
  Q_ASSERT(itemview);

  auto model = itemview->model();
  if (model) { model->disconnect(this); }
}

void ActionRecordItemViewStrategy::onDataChanged(
  const QModelIndex &topLeft, const QModelIndex &bottomRight,
  const QList<int> &roles) {
  /* TODO */
}

}// namespace specter
