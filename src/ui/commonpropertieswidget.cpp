#include "commonpropertieswidget.h"
#include "ui_commonpropertieswidget.h"

#include <QDataWidgetMapper>

CommonPropertiesWidget::CommonPropertiesWidget(ResultsModels &resultsModels, QWidget *parent) :
  QWidget{parent},
  ui{new Ui::CommonPropertiesWidget}
{
  ui->setupUi(this);

  initBackgroundPropsModel(resultsModels.backgroundMapperModel());
  ui->qtbw_systemEigenzones->setModel(resultsModels.systemEigenzonesModel());
}

CommonPropertiesWidget::~CommonPropertiesWidget()
{
  delete ui;
}

void CommonPropertiesWidget::initBackgroundPropsModel(BackgroundPropertiesMapping::MapperModel *model)
{
  m_backgroundPropsMapper = new QDataWidgetMapper{this};
  m_backgroundPropsMapper->setModel(model);

  m_backgroundPropsMapper->setItemDelegate(&m_floatDelegate);

  m_backgroundPropsMapper->addMapping(ui->qle_bufferCapacity, model->indexFromItem(BackgroundPropertiesMapping::Items::BUFFER_CAPACITY));
  m_backgroundPropsMapper->addMapping(ui->qle_conductivity, model->indexFromItem(BackgroundPropertiesMapping::Items::CONDUCTIVITY));
  m_backgroundPropsMapper->addMapping(ui->qle_ionicStrength, model->indexFromItem(BackgroundPropertiesMapping::Items::IONIC_STRENGTH));
  m_backgroundPropsMapper->addMapping(ui->qle_pH, model->indexFromItem(BackgroundPropertiesMapping::Items::PH));
  m_backgroundPropsMapper->toFirst();
}
