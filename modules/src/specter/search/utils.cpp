/* ----------------------------------- Local -------------------------------- */
#include "specter/search/utils.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QApplication>
#include <QWidget>
/* -------------------------------------------------------------------------- */

namespace specter {

QObjectList getTopLevelObjects() {
  auto topLevelObjects = QObjectList{};
  for (const auto object : qApp->topLevelWidgets()) {
    if (!object->parent()) { topLevelObjects.append(object); }
  }
  return topLevelObjects;
}

}// namespace specter
