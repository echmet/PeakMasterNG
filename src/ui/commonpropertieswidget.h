#ifndef COMMONPROPERTIESWIDGET_H
#define COMMONPROPERTIESWIDGET_H

#include "../gearbox/results_models/resultsmodels.h"
#include "../gearbox/floatingvaluedelegate.h"

#include <QWidget>

namespace Ui {
  class CommonPropertiesWidget;
}

class QDataWidgetMapper;

class CommonPropertiesWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CommonPropertiesWidget(ResultsModels &resultsModels, QWidget *parent = nullptr);
  ~CommonPropertiesWidget();

private:
  void initBackgroundPropsModel(BackgroundPropertiesMapping::MapperModel *model);

  Ui::CommonPropertiesWidget *ui;

  FloatingValueDelegate m_floatDelegate;
  QDataWidgetMapper *m_backgroundPropsMapper;
};

#endif // COMMONPROPERTIESWIDGET_H
