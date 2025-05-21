#ifndef SPECTER_OBSERVE_ACTION_H
#define SPECTER_OBSERVE_ACTION_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QQueue>
/* ---------------------------------- Standard ------------------------------ */
#include <variant>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ------------------------------- ObservedAction --------------------------- */

class LIB_SPECTER_API ObservedAction {
public:
  struct ObjectAdded {
    ObjectQuery object;
    ObjectQuery parent;
  };

  struct ObjectRemoved {
    ObjectQuery object;
  };

  struct ObjectReparented {
    ObjectQuery object;
    ObjectQuery parent;
  };

  struct ObjectRenamed {
    ObjectQuery old_object;
    ObjectQuery new_object;
  };

public:
  template<typename ACTION_SUBTYPE>
  ObservedAction(const ACTION_SUBTYPE &action);

  template<typename ACTION_SUBTYPE>
  [[nodiscard]] bool is() const;

  template<typename ACTION_SUBTYPE>
  [[nodiscard]] const ACTION_SUBTYPE *getIf() const;

  [[nodiscard]] std::size_t index() const;

  template<typename TYPE>
  decltype(auto) visit(TYPE &&visitor) const;

private:
  std::variant<ObjectAdded, ObjectRemoved, ObjectReparented, ObjectRenamed>
    m_data;
};

template<typename ACTION_SUBTYPE>
ObservedAction::ObservedAction(const ACTION_SUBTYPE &action) {
  m_data = action;
}

template<typename ACTION_SUBTYPE>
bool ObservedAction::is() const {
  return std::holds_alternative<ACTION_SUBTYPE>(m_data);
}

template<typename ACTION_SUBTYPE>
const ACTION_SUBTYPE *ObservedAction::getIf() const {
  return std::get_if<ACTION_SUBTYPE>(&m_data);
}

template<typename TYPE>
decltype(auto) ObservedAction::visit(TYPE &&visitor) const {
  return std::visit(std::forward<TYPE>(visitor), m_data);
}

}// namespace specter

#endif// SPECTER_OBSERVE_ACTION_H