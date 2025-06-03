#ifndef SPECTER_OBSERVE_PROPERTY_ACTION_H
#define SPECTER_OBSERVE_PROPERTY_ACTION_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QQueue>
/* ---------------------------------- Standard ------------------------------ */
#include <variant>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* --------------------------- PropertyObservedAction ----------------------- */

class LIB_SPECTER_API PropertyObservedAction {
public:
  struct PropertyAdded {
    QString property;
    QVariant value;
    bool read_only;
  };

  struct PropertyRemoved {
    QString property;
  };

  struct PropertyUpdated {
    QString property;
    QVariant old_value;
    QVariant new_value;
  };

public:
  template<typename ACTION_SUBTYPE>
  PropertyObservedAction(const ACTION_SUBTYPE &action);

  template<typename ACTION_SUBTYPE>
  [[nodiscard]] bool is() const;

  template<typename ACTION_SUBTYPE>
  [[nodiscard]] const ACTION_SUBTYPE *getIf() const;

  [[nodiscard]] std::size_t index() const;

  template<typename TYPE>
  decltype(auto) visit(TYPE &&visitor) const;

private:
  std::variant<PropertyAdded, PropertyRemoved, PropertyUpdated> m_data;
};

template<typename ACTION_SUBTYPE>
PropertyObservedAction::PropertyObservedAction(const ACTION_SUBTYPE &action) {
  m_data = action;
}

template<typename ACTION_SUBTYPE>
bool PropertyObservedAction::is() const {
  return std::holds_alternative<ACTION_SUBTYPE>(m_data);
}

template<typename ACTION_SUBTYPE>
const ACTION_SUBTYPE *PropertyObservedAction::getIf() const {
  return std::get_if<ACTION_SUBTYPE>(&m_data);
}

template<typename TYPE>
decltype(auto) PropertyObservedAction::visit(TYPE &&visitor) const {
  return std::visit(std::forward<TYPE>(visitor), m_data);
}

}// namespace specter

#endif// SPECTER_OBSERVE_PROPERTY_ACTION_H