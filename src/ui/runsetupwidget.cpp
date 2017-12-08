#include "runsetupwidget.h"
#include "ui_runsetupwidget.h"

#include <QDataWidgetMapper>

RunSetupWidget::RunSetupWidget(QWidget *parent) :
  QWidget{parent},
  ui{new Ui::RunSetupWidget},
  m_mapperModel{this}
{
  ui->setupUi(this);

  m_mapper = new QDataWidgetMapper{this};
  m_mappedData.resize(m_mapperModel.indexFromItem(Items::LAST_INDEX));

  m_mappedData[m_mapperModel.indexFromItem(Items::TOTAL_LENGTH)] = 50.0;
  m_mappedData[m_mapperModel.indexFromItem(Items::DETECTOR_POSITION)] = 43.5;
  m_mappedData[m_mapperModel.indexFromItem(Items::DRIVING_VOLTAGE)] = 20000.0;

  m_mapperModel.setUnderlyingData(&m_mappedData);

  m_mapper->setModel(&m_mapperModel);
  m_mapper->setItemDelegate(&m_fltDelegate);
  m_mapper->addMapping(ui->qle_totalLength, m_mapperModel.indexFromItem(Items::TOTAL_LENGTH));
  m_mapper->addMapping(ui->qle_detectorPosition, m_mapperModel.indexFromItem(Items::DETECTOR_POSITION));
  m_mapper->addMapping(ui->qle_drivingVoltage, m_mapperModel.indexFromItem(Items::DRIVING_VOLTAGE));
  m_mapper->addMapping(ui->qle_eofValue, m_mapperModel.indexFromItem(Items::EOF_VALUE));
  m_mapper->toFirst();

  ui->qcbox_polarity->addItem(tr("Positive"), POLARITY_POSITIVE);
  ui->qcbox_polarity->addItem(tr("Negative"), POLARITY_NEGATIVE);
  ui->qcbox_polarity->setCurrentIndex(0);

  ui->qcbox_eof->addItem(tr("No EOF"), EOF_NONE);
  ui->qcbox_eof->addItem(tr("Marker time"), EOF_MARKER_TIME);
  ui->qcbox_eof->addItem(tr("Mobility"), EOF_MOBILITY);
  connect(ui->qcbox_eof, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RunSetupWidget::onEOFCurrentIndexChanged);
  ui->qcbox_eof->setCurrentIndex(0);
  onEOFCurrentIndexChanged(ui->qcbox_eof->currentIndex());
}

RunSetupWidget::~RunSetupWidget()
{
  delete ui;
}

RunSetupWidget::EOF_Type RunSetupWidget::EOFInputType() const
{
  const QVariant v = ui->qcbox_eof->currentData();
  if (!v.canConvert<EOF_Type>())
    return EOF_NONE;

  return v.value<EOF_Type>();
}

double RunSetupWidget::EOFValue() const
{
  return m_mappedData.at(m_mapperModel.indexFromItem(Items::EOF_VALUE));
}

void RunSetupWidget::onEOFCurrentIndexChanged(const int idx)
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
}

RunSetupWidget::RunSetup RunSetupWidget::runSetup() const
{
  bool positiveVoltage = (ui->qcbox_polarity->currentData().value<Polarity>() == POLARITY_POSITIVE);

  return RunSetup{
    m_mappedData.at(m_mapperModel.indexFromItem(Items::TOTAL_LENGTH)),
    m_mappedData.at(m_mapperModel.indexFromItem(Items::DETECTOR_POSITION)),
    m_mappedData.at(m_mapperModel.indexFromItem(Items::DRIVING_VOLTAGE)),
    positiveVoltage,
    ui->qcb_correctForIS->isChecked()
  };
}
