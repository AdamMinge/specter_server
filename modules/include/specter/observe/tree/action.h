#ifndef SPECTER_OBSERVE_TREE_ACTION_H
#define SPECTER_OBSERVE_TREE_ACTION_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QQueue>
/* ---------------------------------- Standard ------------------------------ */
#include <variant>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ----------------------------- TreeObservedAction ------------------------- */

class LIB_SPECTER_API TreeObservedAction {
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
  TreeObservedAction(const ACTION_SUBTYPE &action);

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
TreeObservedAction::TreeObservedAction(const ACTION_SUBTYPE &action) {
  m_data = action;
}

template<typename ACTION_SUBTYPE>
bool TreeObservedAction::is() const {
  return std::holds_alternative<ACTION_SUBTYPE>(m_data);
}

template<typename ACTION_SUBTYPE>
const ACTION_SUBTYPE *TreeObservedAction::getIf() const {
  return std::get_if<ACTION_SUBTYPE>(&m_data);
}

template<typename TYPE>
decltype(auto) TreeObservedAction::visit(TYPE &&visitor) const {
  return std::visit(std::forward<TYPE>(visitor), m_data);
}

}// namespace specter

#endif// SPECTER_OBSERVE_TREE_ACTION_H