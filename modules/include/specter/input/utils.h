#ifndef SPECTER_INPUT_UTILS_H
#define SPECTER_INPUT_UTILS_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QWidget>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace specter {

[[nodiscard]] QWidget *getTargetWidget();
[[nodiscard]] QWidget *getTargetWidget(const QPoint &pos);

}// namespace specter

#endif// SPECTER_INPUT_UTILS_H