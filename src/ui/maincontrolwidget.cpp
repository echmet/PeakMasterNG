#include "maincontrolwidget.h"
#include "ui_maincontrolwidget.h"

#include "eigenzonedetailsdialog.h"
#include "ioniccompositiondialog.h"
#include "nonidealitycorrectionsdialog.h"
#include "elementaries/uihelpers.h"
#include "adjustphdialog.h"

#include "../globals.h"

#include "../gearbox/results_models/eigenzonedetailsmodel.h"

#include <cassert>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QScreen>
#include <QTimer>
#include <QWindow>

MainControlWidget::MainControlWidget(GDMProxy &GDMProxy, ResultsModels &resultsModels, QWidget *parent) :
  QWidget{parent},
  ui{new Ui::MainControlWidget},
  m_runSetupMapperModel{this},
  m_analytesDissociationModel{resultsModels.analytesDissociationModel()},
  m_bgeIonicCompositionModel{resultsModels.bgeIonicCompositionModel()},
  m_eigenzoneDetailsModel{resultsModels.eigenzoneDetailsModel()},
  h_GDMProxy{GDMProxy}
{
  ui->setupUi(this);

  auto eofcbox = ui->qcbox_eof;

  m_mustBePositiveAV = std::make_shared<AdditionalFloatingValidator>([](const double d) { return d > 0.0; });

  ui->qle_eofValue->setProperty(AdditionalFloatingValidator::PROPERTY_NAME,
                                QVariant::fromValue<AdditionalFloatingValidatorVec>({
                                  std::make_shared<AdditionalFloatingValidator>([eofcbox](const double d) {
                                    const auto data = eofcbox->currentData();
                                    if (!data.canConvert<EOF_Type>())
                                      return true;
                                    const auto val = data.value<EOF_Type>();

                                    if (val == EOF_MARKER_TIME)
                                      return d > 0.0;
                                    return true;
                                  })}));

  ui->qle_totalLength->setProperty(AdditionalFloatingValidator::PROPERTY_NAME,
                                   QVariant::fromValue<AdditionalFloatingValidatorVec>({ m_mustBePositiveAV }));
  ui->qle_detectorPosition->setProperty(AdditionalFloatingValidator::PROPERTY_NAME,
                                        QVariant::fromValue<AdditionalFloatingValidatorVec>(
                                          { m_mustBePositiveAV,
                                            std::make_shared<AdditionalFloatingValidator>([this](const double d) -> bool {
                                                const double tl = this->m_runSetupMappedData.at(m_runSetupMapperModel.indexFromItem(RunSetupItems::TOTAL_LENGTH));
                                                return tl >= d;
                                              }
                                            )
                                          }));
  ui->qle_drivingVoltage->setProperty(AdditionalFloatingValidator::PROPERTY_NAME,
                                      QVariant::fromValue<AdditionalFloatingValidatorVec>({ m_mustBePositiveAV }));

  if (Globals::isZombieOS())
    ui->ql_resistivity->setText(tr("Resistivity (Ohm.m)"));
  else
    ui->ql_resistivity->setText(tr("Resistivity (Ohm\xE2\x8B\x85m)"));

  initBackgroundPropsModel(resultsModels.backgroundMapperModel());
  initRunSetupModel();
  ui->qtbv_systemEigenzones->setModel(resultsModels.systemEigenzonesModel());
  ui->qtbv_systemEigenzones->resizeColumnsToContents();

  m_bgeIonicCompDlg = new IonicCompositionDialog{m_bgeIonicCompositionModel, m_analytesDissociationModel, this};
  m_bgeIonicCompDlg->setWindowTitle(tr("BGE ionic composition"));
  m_nonidealityCorrectionsDlg = new NonidealityCorrectionsDialog{this};
  connect(ui->qpb_nonidealityCorrections, &QPushButton::clicked, this, &MainControlWidget::onNonidealityCorrectionsClicked);

  connect(&m_runSetupMapperModel, &FloatMapperModel<double>::dataChanged, this, &MainControlWidget::onRunSetupChanged);
  connect(ui->qcbox_polarity, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainControlWidget::onRunSetupChanged);
  connect(ui->qcbox_eof, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainControlWidget::onRunSetupChanged);
  connect(ui->qpb_details, &QPushButton::clicked, this, &MainControlWidget::onShowEigenzoneDetailsClicked);
  connect(ui->qpb_bgeIonicComposition, &QPushButton::clicked, this, &MainControlWidget::onBGEIonicCompositionClicked);
  connect(ui->qpb_adjustpH, &QPushButton::clicked,
          [this]() {
            try {
              const auto corrections = m_nonidealityCorrectionsDlg->state();

              AdjustpHDialog dlg{h_GDMProxy, corrections.debyeHuckel, corrections.onsagerFuoss, this};
              dlg.exec();

              emit pHAdjusted();
            } catch (const std::bad_cast &) {
              assert(false);
            }
         });

  EigenzoneDetailsModel *ezdModel = qobject_cast<EigenzoneDetailsModel *>(m_eigenzoneDetailsModel);
  if (ezdModel == nullptr)
    return;
  ezdModel->displayDeltas(false);
  m_ezDetailsDlg = new EigenzoneDetailsDialog{m_eigenzoneDetailsModel, false, this};
  connect(m_ezDetailsDlg, &EigenzoneDetailsDialog::displayDeltasChanged, ezdModel, &EigenzoneDetailsModel::displayDeltas);

  connect(ui->qle_totalLength, &FloatingValueLineEdit::valueChanged, ui->qle_detectorPosition, &FloatingValueLineEdit::revalidate);
}

MainControlWidget::~MainControlWidget()
{
  delete ui;
}

void MainControlWidget::connectOnScreenChanged()
{
  auto wnd = this->window();
  if (wnd == nullptr)
    return;

  auto wh = wnd->windowHandle();
  if (wh != nullptr)
    connect(wh, &QWindow::screenChanged, this, &MainControlWidget::onScreenChanged);
}

void MainControlWidget::initBackgroundPropsModel(BackgroundPropertiesMapping::MapperModel *model)
{
  m_backgroundPropsMapper = new QDataWidgetMapper{this};
  m_backgroundPropsMapper->setModel(model);

  m_backgroundPropsMapper->setItemDelegate(&m_fltDelegateBackgroundProps);

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
  m_runSetupMappedData[m_runSetupMapperModel.indexFromItem(RunSetupItems::DETECTOR_POSITION)] = 41.5;
  m_runSetupMappedData[m_runSetupMapperModel.indexFromItem(RunSetupItems::DRIVING_VOLTAGE)] = 20000.0;

  m_runSetupMapperModel.setUnderlyingData(&m_runSetupMappedData);

  m_runSetupMapper->setModel(&m_runSetupMapperModel);
  m_runSetupMapper->setItemDelegate(&m_fltDelegateRunSetup);
  m_runSetupMapper->addMapping(ui->qle_totalLength, m_runSetupMapperModel.indexFromItem(RunSetupItems::TOTAL_LENGTH));
  m_runSetupMapper->addMapping(ui->qle_detectorPosition, m_runSetupMapperModel.indexFromItem(RunSetupItems::DETECTOR_POSITION));
  m_runSetupMapper->addMapping(ui->qle_drivingVoltage, m_runSetupMapperModel.indexFromItem(RunSetupItems::DRIVING_VOLTAGE));
  m_runSetupMapper->addMapping(ui->qle_eofValue, m_runSetupMapperModel.indexFromItem(RunSetupItems::EOF_VALUE));
  m_runSetupMapper->toFirst();

  ui->qcbox_polarity->addItem(tr("Positive"), POLARITY_POSITIVE);
  ui->qcbox_polarity->addItem(tr("Negative"), POLARITY_NEGATIVE);
  ui->qcbox_polarity->setCurrentIndex(0);

  ui->qcbox_eof->addItem(tr("No EOF"), EOF_NONE);
  ui->qcbox_eof->addItem(tr("Marker time (min)"), EOF_MARKER_TIME);
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
  ui->qle_eofValue->clearFocus();

  return m_runSetupMappedData.at(m_runSetupMapperModel.indexFromItem(RunSetupItems::EOF_VALUE));
}

void MainControlWidget::onBGEIonicCompositionClicked()
{
  m_bgeIonicCompDlg->resize(m_bgeIonicCompDlg->sizeHint());
  m_bgeIonicCompDlg->show();
}

void MainControlWidget::onEOFCurrentIndexChanged(const int idx)
{
  const EOF_Type t = ui->qcbox_eof->itemData(idx).value<EOF_Type>();

  const bool hide = (t == EOF_NONE);

  ui->ql_eofValue->setHidden(hide);
  ui->qle_eofValue->setHidden(hide);

  ui->qle_eofValue->revalidate();

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

void MainControlWidget::onNonidealityCorrectionsClicked()
{
  if (m_nonidealityCorrectionsDlg->exec() == QDialog::Accepted)
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

void MainControlWidget::onScreenChanged(QScreen *screen)
{
  if (screen == nullptr)
    return;
  /* Yes, this check seems extremely wierd but there is no telling
   * what the fontMetrics() function might do when there is no screen
   * associated with the widget.
   * This is an odd attempt to work around some mind-boggling
   * crashes that apparently occur only on multi-head Windows systems.
   */

  int h = fontMetrics().height() * 4;
  ui->qtbv_systemEigenzones->setMinimumHeight(h);
}

void MainControlWidget::onShowEigenzoneDetailsClicked()
{
  m_ezDetailsDlg->resize(m_ezDetailsDlg->sizeHint());
  m_ezDetailsDlg->show();
}

MainControlWidget::RunSetup MainControlWidget::runSetup() const
{
  /* Commit changes in any active edits */
  ui->qle_totalLength->clearFocus();
  ui->qle_detectorPosition->clearFocus();
  ui->qle_drivingVoltage->clearFocus();
  ui->qle_eofValue->clearFocus();

  bool positiveVoltage = (ui->qcbox_polarity->currentData().value<Polarity>() == POLARITY_POSITIVE);
  const auto corrections = m_nonidealityCorrectionsDlg->state();

  return RunSetup{
    m_runSetupMappedData.at(m_runSetupMapperModel.indexFromItem(RunSetupItems::TOTAL_LENGTH)),
    m_runSetupMappedData.at(m_runSetupMapperModel.indexFromItem(RunSetupItems::DETECTOR_POSITION)),
    m_runSetupMappedData.at(m_runSetupMapperModel.indexFromItem(RunSetupItems::DRIVING_VOLTAGE)),
    positiveVoltage,
    corrections.debyeHuckel,
    corrections.onsagerFuoss,
    corrections.viscosity
  };
}

void MainControlWidget::setRunSetup(const RunSetup &rs, const QVariant &eofType, const double EOFValue)
{
  static const auto getPolarity = [](const bool positive) {
    if (positive)
      return POLARITY_POSITIVE;
    return POLARITY_NEGATIVE;
  };

  static const auto getEOFTypeIndex = [this](const QVariant &v)
  {
    for (int idx = 0; idx < ui->qcbox_eof->count(); idx++) {
      if (ui->qcbox_eof->itemData(idx).value<EOF_Type>() == v)
        return idx;
    }

    return 0;
  };

  m_runSetupMappedData[m_runSetupMapperModel.indexFromItem(RunSetupItems::TOTAL_LENGTH)] = rs.totalLength;
  m_runSetupMappedData[m_runSetupMapperModel.indexFromItem(RunSetupItems::DETECTOR_POSITION)] = rs.detectorPosition;
  m_runSetupMappedData[m_runSetupMapperModel.indexFromItem(RunSetupItems::DRIVING_VOLTAGE)] = rs.drivingVoltage;

  for (int idx = 0; idx < ui->qcbox_polarity->count(); idx++) {
    if (ui->qcbox_polarity->itemData(idx).value<Polarity>() == getPolarity(rs.positiveVoltage)) {
      ui->qcbox_polarity->setCurrentIndex(idx);
      break;
    }
  }

  m_runSetupMappedData[m_runSetupMapperModel.indexFromItem(RunSetupItems::EOF_VALUE)] = EOFValue;
  int eofIdx = getEOFTypeIndex(eofType);
  ui->qcbox_eof->setCurrentIndex(eofIdx);

  m_nonidealityCorrectionsDlg->setState(rs.correctForDebyeHuckel, rs.correctForOnsagerFuoss, rs.correctForViscosity);

  m_runSetupMapperModel.notifyAllDataChanged();
}
