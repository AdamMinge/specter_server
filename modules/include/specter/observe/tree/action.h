#ifndef SPECTER_OBSERVE_ACTION_H
#define SPECTER_OBSERVE_ACTION_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QQueue>
/* ---------------------------------- Standard ------------------------------ */
#include <variant>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/id.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ----------------------------- TreeObservedAction ------------------------- */

class LIB_SPECTER_API TreeObservedAction {
public:
  struct ObjectAdded {
    ObjectId object_id;
    ObjectId parent_id;
  };

  struct ObjectRemoved {
    ObjectId object_id;
  };

  struct ObjectReparented {
    ObjectId object_id;
    ObjectId parent_id;
  };

  struct ObjectRenamed {
    ObjectId object_id;
    ObjectQuery object_query;
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

#endif// SPECTER_OBSERVE_ACTION_H