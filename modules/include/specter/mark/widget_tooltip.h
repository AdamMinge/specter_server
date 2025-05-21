#ifndef SPECTER_MARK_WIDGET_TOOLTIP_H
#define SPECTER_MARK_WIDGET_TOOLTIP_H

/* ------------------------------------ Qt ---------------------------------- */
#include <QDialog>
#include <QLabel>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
/* -------------------------------------------------------------------------- */

namespace specter {

/* ---------------------------- MarkerWidgetTooltip ------------------------ */

class LIB_SPECTER_API MarkerWidgetTooltip : public QDialog {
  Q_OBJECT

 public:
  explicit MarkerWidgetTooltip(QWidget *parent = nullptr);
  ~MarkerWidgetTooltip() override;

  [[nodiscard]] QWidget *getWidget() const;
  void setWidget(QWidget *object);

 private:
  void initUi();

 private:
  QLabel *m_label;
  QWidget *m_widget;
};

}  // namespace specter

#endif  // SPECTER_MARK_WIDGET_TOOLTIP_H