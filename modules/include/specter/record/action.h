#ifndef SPECTER_RECORD_ACTION_H
#define SPECTER_RECORD_ACTION_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QQueue>
/* ---------------------------------- Standard ------------------------------ */
#include <variant>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------- RecordedAction --------------------------- */

class LIB_SPECTER_API RecordedAction {
public:
  struct ContextMenuOpened {
    ObjectQuery object;
  };

  struct ButtonClicked {
    ObjectQuery object;
  };

  struct ButtonToggled {
    ObjectQuery object;
    bool checked;
  };

  struct ComboBoxCurrentChanged {
    ObjectQuery object;
    int index;
  };

  struct SpinBoxValueChanged {
    ObjectQuery object;
    int value;
  };

  struct DoubleSpinBoxValueChanged {
    ObjectQuery object;
    double value;
  };

  struct SliderValueChanged {
    ObjectQuery object;
    int value;
  };

  struct TabCurrentChanged {
    ObjectQuery object;
    int index;
  };

  struct TabClosed {
    ObjectQuery object;
    int index;
  };

  struct TabMoved {
    ObjectQuery object;
    int from;
    int to;
  };

  struct ToolBoxCurrentChanged {
    ObjectQuery object;
    int index;
  };

  struct ActionTriggered {
    ObjectQuery object;
  };

  struct TextEditTextChanged {
    ObjectQuery object;
    QString value;
  };

  struct LineEditTextChanged {
    ObjectQuery object;
    QString value;
  };

  struct LineEditReturnPressed {
    ObjectQuery object;
  };

public:
  template<typename ACTION_SUBTYPE>
  RecordedAction(const ACTION_SUBTYPE &action);

  template<typename ACTION_SUBTYPE>
  [[nodiscard]] bool is() const;

  template<typename ACTION_SUBTYPE>
  [[nodiscard]] const ACTION_SUBTYPE *getIf() const;

  [[nodiscard]] std::size_t index() const;

  template<typename TYPE>
  decltype(auto) visit(TYPE &&visitor) const;

private:
  std::variant<
    ContextMenuOpened, ButtonClicked, ButtonToggled, ComboBoxCurrentChanged,
    SpinBoxValueChanged, DoubleSpinBoxValueChanged, SliderValueChanged,
    TabCurrentChanged, TabClosed, TabMoved, ToolBoxCurrentChanged,
    ActionTriggered, TextEditTextChanged, LineEditTextChanged,
    LineEditReturnPressed>
    m_data;
};

template<typename ACTION_SUBTYPE>
RecordedAction::RecordedAction(const ACTION_SUBTYPE &action) {
  m_data = action;
}

template<typename ACTION_SUBTYPE>
bool RecordedAction::is() const {
  return std::holds_alternative<ACTION_SUBTYPE>(m_data);
}

template<typename ACTION_SUBTYPE>
const ACTION_SUBTYPE *RecordedAction::getIf() const {
  return std::get_if<ACTION_SUBTYPE>(&m_data);
}

template<typename TYPE>
decltype(auto) RecordedAction::visit(TYPE &&visitor) const {
  return std::visit(std::forward<TYPE>(visitor), m_data);
}

}// namespace specter

#endif// SPECTER_RECORD_ACTION_H