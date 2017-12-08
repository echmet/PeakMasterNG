#include "maincontrolwidget.h"
#include "ui_maincontrolwidget.h"

#include "eigenzonedetailsdialog.h"

#include <QDataWidgetMapper>

MainControlWidget::MainControlWidget(ResultsModels &resultsModels, QWidget *parent) :
  QWidget{parent},
  ui{new Ui::MainControlWidget},
  m_runSetupMapperModel{this},
  m_eigenzoneDetailsModel{resultsModels.eigenzoneDetailsModel()}
{
  ui->setupUi(this);

  initBackgroundPropsModel(resultsModels.backgroundMapperModel());
  initRunSetupModel();
  ui->qtbv_systemEigenzones->setModel(resultsModels.systemEigenzonesModel());

  connect(&m_runSetupMapperModel, &FloatMapperModel<double>::dataChanged, this, &MainControlWidget::onRunSetupChanged);
  connect(ui->qcbox_polarity, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainControlWidget::onRunSetupChanged);
  connect(ui->qcb_correctForIS, &QCheckBox::stateChanged, this, &MainControlWidget::onRunSetupChangedInvalidate);
  connect(ui->qpb_details, &QPushButton::clicked, this, &MainControlWidget::onShowEigenzoneDetailsClicked);
}

MainControlWidget::~MainControlWidget()
{
  delete ui;
}

void MainControlWidget::initBackgroundPropsModel(BackgroundPropertiesMapping::MapperModel *model)
{
  m_backgroundPropsMapper = new QDataWidgetMapper{this};
  m_backgroundPropsMapper->setModel(model);

  m_backgroundPropsMapper->setItemDelegate(&m_fltDelegate);

  m_backgroundPropsMapper->addMapping(ui->qle_bufferCapacity, model->indexFromItem(BackgroundPropertiesMapping::Items::BUFFER_CAPACITY));
  m_backgroundPropsMapper->addMapping(ui->qle_conductivity, model->indexFromItem(BackgroundPropertiesMapping::Items::CONDUCTIVITY));
  m_backgroundPropsMapper->addMapping(ui->qle_resistivity, model->indexFromItem(BackgroundPropertiesMapping::Items::RESISTIVITY));
  m_backgroundPropsMapper->addMapping(ui->qle_ionicStrength, model->indexFromItem(BackgroundPropertiesMapping::Items::IONIC_STRENGTH));
  m_backgroundPropsMapper->addMapping(ui->qle_pH, model->indexFromItem(BackgroundPropertiesMapping::Items::PH));
  m_backgroundPropsMapper->addMapping(ui->qle_eofMobility, model->indexFromItem(BackgroundPropertiesMapping::Items::EOF_MOBILITY));
  m_backgroundPropsMapper->addMapping(ui->qle_eofMarkerTime, model->indexFromItem(BackgroundPropertiesMapping::Items::EOF_MARKER_TIME));

  m_backgroundPropsMapper->toFirst();
}

void MainControlWidget::initRunSetupModel()
{
  m_runSetupMapper = new QDataWidgetMapper{this};
  m_runSetupMappedData.resize(m_runSetupMapperModel.indexFromItem(RunSetupItems::LAST_INDEX));

  m_runSetupMappedData[m_runSetupMapperModel.indexFromItem(RunSetupItems::TOTAL_LENGTH)] = 50.0;
  m_runSetupMappedData[m_runSetupMapperModel.indexFromItem(RunSetupItems::DETECTOR_POSITION)] = 43.5;
  m_runSetupMappedData[m_runSetupMapperModel.indexFromItem(RunSetupItems::DRIVING_VOLTAGE)] = 20000.0;

  m_runSetupMapperModel.setUnderlyingData(&m_runSetupMappedData);

  m_runSetupMapper->setModel(&m_runSetupMapperModel);
  m_runSetupMapper->setItemDelegate(&m_fltDelegate);
  m_runSetupMapper->addMapping(ui->qle_totalLength, m_runSetupMapperModel.indexFromItem(RunSetupItems::TOTAL_LENGTH));
  m_runSetupMapper->addMapping(ui->qle_detectorPosition, m_runSetupMapperModel.indexFromItem(RunSetupItems::DETECTOR_POSITION));
  m_runSetupMapper->addMapping(ui->qle_drivingVoltage, m_runSetupMapperModel.indexFromItem(RunSetupItems::DRIVING_VOLTAGE));
  m_runSetupMapper->addMapping(ui->qle_eofValue, m_runSetupMapperModel.indexFromItem(RunSetupItems::EOF_VALUE));
  m_runSetupMapper->toFirst();

  ui->qcbox_polarity->addItem(tr("Positive"), POLARITY_POSITIVE);
  ui->qcbox_polarity->addItem(tr("Negative"), POLARITY_NEGATIVE);
  ui->qcbox_polarity->setCurrentIndex(0);

  ui->qcbox_eof->addItem(tr("No EOF"), EOF_NONE);
  ui->qcbox_eof->addItem(tr("Marker time"), EOF_MARKER_TIME);
  ui->qcbox_eof->addItem(tr("Mobility"), EOF_MOBILITY);
  connect(ui->qcbox_eof, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainControlWidget::onEOFCurrentIndexChanged);
  ui->qcbox_eof->setCurrentIndex(0);
  onEOFCurrentIndexChanged(ui->qcbox_eof->currentIndex());
}

MainControlWidget::EOF_Type MainControlWidget::EOFInputType() const
{
  const QVariant v = ui->qcbox_eof->currentData();
  if (!v.canConvert<EOF_Type>())
    return EOF_NONE;

  return v.value<EOF_Type>();
}

double MainControlWidget::EOFValue() const
{
  return m_runSetupMappedData.at(m_runSetupMapperModel.indexFromItem(RunSetupItems::EOF_VALUE));
}

void MainControlWidget::onEOFCurrentIndexChanged(const int idx)
{
  const EOF_Type t = ui->qcbox_eof->itemData(idx).value<EOF_Type>();

  const bool hide = (t == EOF_NONE);

  ui->ql_eofValue->setHidden(hide);
  ui->qle_eofValue->setHidden(hide);

  switch (t) {
  case EOF_NONE:
    return;
  case EOF_MARKER_TIME:
    ui->ql_eofValue->setText(tr("EOF Marker time (min)"));
    return;
  case EOF_MOBILITY:
    ui->ql_eofValue->setText(tr("EOF Mobility"));
    return;
  }

  emit runSetupChanged(false);
}

void MainControlWidget::onIonicStrengthToggled()
{
  emit runSetupChanged(true);
}

void MainControlWidget::onRunSetupChanged()
{
  emit runSetupChanged(false);
}

void MainControlWidget::onRunSetupChangedInvalidate()
{
  emit runSetupChanged(true);
}

void MainControlWidget::onShowEigenzoneDetailsClicked()
{
  EigenzoneDetailsDialog dlg{m_eigenzoneDetailsModel, this};

  dlg.exec();
}

MainControlWidget::RunSetup MainControlWidget::runSetup() const
{
  bool positiveVoltage = (ui->qcbox_polarity->currentData().value<Polarity>() == POLARITY_POSITIVE);

  return RunSetup{
    m_runSetupMappedData.at(m_runSetupMapperModel.indexFromItem(RunSetupItems::TOTAL_LENGTH)),
    m_runSetupMappedData.at(m_runSetupMapperModel.indexFromItem(RunSetupItems::DETECTOR_POSITION)),
    m_runSetupMappedData.at(m_runSetupMapperModel.indexFromItem(RunSetupItems::DRIVING_VOLTAGE)),
    positiveVoltage,
    ui->qcb_correctForIS->isChecked()
  };
}
