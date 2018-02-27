#include "pmngmainwindow.h"
#include "ui_pmngmainwindow.h"

#include "maincontrolwidget.h"
#include "signalplotwidget.h"
#include "../gearbox/results_models/resultsmodels.h"
#include "systemcompositionwidget.h"
#include "aboutdialog.h"
#include "../globals.h"
#include "../gearbox/calculatorworker.h"
#include "../persistence/persistence.h"
#include "operationinprogressdialog.h"
#include "../gearbox/efgdisplayer.h"
#include "../gearbox/efgcsvexporter.h"
#include "databaseeditordialog.h"

#include <QDialogButtonBox>
#include <QDataWidgetMapper>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QThread>

void inputToEOFValueType(double &EOFValue, CalculatorInterface::EOFValueType &EOFvt, const double inValue, const MainControlWidget::EOF_Type type)
{
  if (type == MainControlWidget::EOF_MOBILITY) {
    EOFValue = inValue;
    EOFvt = CalculatorInterface::EOFValueType::MOBILITY;
  } else if (type == MainControlWidget::EOF_MARKER_TIME) {
    EOFValue = inValue * 60.0;
    EOFvt = CalculatorInterface::EOFValueType::MARKER_TIME;
  } else {
    EOFValue = 0.0;
    EOFvt = CalculatorInterface::EOFValueType::MOBILITY;
  }
}

SignalPlotWidget::SignalStyle plotSignalStyle(const CalculatorInterface::SignalTypes type)
{
  switch (type) {
  case CalculatorInterface::SignalTypes::CONDUCTIVITY:
    return SignalPlotWidget::SignalStyle::CONDUCTIVITY;
  case CalculatorInterface::SignalTypes::PH_RESPONSE:
    return SignalPlotWidget::SignalStyle::PH_RESPONSE;
  case CalculatorInterface::SignalTypes::ALL_ANALYTES:
    return SignalPlotWidget::SignalStyle::ALL_ANALYTES;
  case CalculatorInterface::SignalTypes::CONCENTRATION:
    return SignalPlotWidget::SignalStyle::CONCENTRATION;
  }
}

QVector<PMNGMainWindow::SignalItem> PMNGMainWindow::s_defaultSignalItems{
                                                                         { "Conductivity", { CalculatorInterface::SignalTypes::CONDUCTIVITY, "" , "Conductivity (S/m)" }},
                                                                         { "pH response", { CalculatorInterface::SignalTypes::PH_RESPONSE, "" , QString::fromUtf8("\xCE\x94 pH") }},
                                                                         { "All analytes", { CalculatorInterface::SignalTypes::ALL_ANALYTES, "", "Concentration (mM)" }}
                                                                        };

PMNGMainWindow::PMNGMainWindow(SystemCompositionWidget *scompWidget,
                               CalculatorInterface &&calcIface, ResultsModels resultsModels,
                               persistence::Persistence &persistence,
                               DatabaseProxy &dbProxy,
                               QWidget *parent) :
  QMainWindow{parent},
  m_calcIface{calcIface},
  m_signalPlotWidget{new SignalPlotWidget{this}},
  h_scompWidget{scompWidget},
  m_plotParamsModel{this},
  m_persistence{persistence},
  m_lastLoadPath{""},
  m_lastSavePath{""},
  h_dbProxy{dbProxy},
  ui{new Ui::PMNGMainWindow}
{
  ui->setupUi(this);

  setWindowTitle(Globals::VERSION_STRING());

  ui->qvlay_compositionEFG->insertWidget(0, h_scompWidget);
  ui->qvlay_compositionEFG->addWidget(m_signalPlotWidget);

  m_mainCtrlWidget = new MainControlWidget{resultsModels, this};
  ui->qvlay_leftPane->addWidget(m_mainCtrlWidget);

  initPlotParams();

  connect(ui->qcb_autoPlotCutoff, &QCheckBox::stateChanged, this, &PMNGMainWindow::onAutoPlotCutoffStateChanged);
  connect(ui->qcbox_signal, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &PMNGMainWindow::onPlotElectrophoregram);

  m_qpb_new = new QPushButton{tr("New"), this};
  connect(m_qpb_new, &QPushButton::clicked, ui->actionNew, &QAction::trigger);
  m_qpb_load = new QPushButton{tr("Load"), this};
  connect(m_qpb_load, &QPushButton::clicked, ui->actionLoad, &QAction::trigger);
  m_qpb_save = new QPushButton{tr("Save"), this};
  connect(m_qpb_save, &QPushButton::clicked, ui->actionSave, &QAction::trigger);
  m_qpb_calculate = new QPushButton{tr("Calculate!"), this};
  connect(m_qpb_calculate, &QPushButton::clicked, this, &PMNGMainWindow::onCalculate);

  connect(h_scompWidget, &SystemCompositionWidget::compositionChanged, this, &PMNGMainWindow::onCompositionChanged);
  connect(m_mainCtrlWidget, &MainControlWidget::runSetupChanged, this, &PMNGMainWindow::onRunSetupChanged);

  connect(ui->actionExit, &QAction::triggered, this, &PMNGMainWindow::onExit);
  connect(ui->actionAbout, &QAction::triggered, this, &PMNGMainWindow::onAbout);
  connect(ui->actionNew, &QAction::triggered, this, &PMNGMainWindow::onNew);
  connect(ui->actionLoad, &QAction::triggered, this, &PMNGMainWindow::onLoad);
  connect(ui->actionSave, &QAction::triggered, this, &PMNGMainWindow::onSave);
  connect(ui->actionExportEFGAsCSV, &QAction::triggered, this, &PMNGMainWindow::onExportElectrophoregramAsCSV);
  connect(ui->actionDatabase_editor, &QAction::triggered, this, &PMNGMainWindow::onDatabaseEditor);

  ui->mainToolBar->addWidget(m_qpb_new);
  ui->mainToolBar->addWidget(m_qpb_load);
  ui->mainToolBar->addWidget(m_qpb_save);
  ui->mainToolBar->addWidget(m_qpb_calculate);

  onAutoPlotCutoffStateChanged(Qt::Checked);

  setControlsIcons();

  h_scompWidget->setViscosityCorrectionEnabled(m_mainCtrlWidget->runSetup().correctForViscosity);

  setWindowIcon(Globals::icon());
}

PMNGMainWindow::~PMNGMainWindow()
{
  delete ui;
}

void PMNGMainWindow::addConstituentsSignals(const QVector<QString> &constituents)
{
  for (const auto &a : constituents) {
    QStandardItem *si = new QStandardItem{QString{tr("c (%1)")}.arg(a)};
    QString plotCaption = QString{"c %1 (mM)"}.arg(a);
    si->setData(QVariant::fromValue<CalculatorInterface::Signal>({ CalculatorInterface::SignalTypes::CONCENTRATION, a, plotCaption }));
    m_signalTypesModel->appendRow(si);
  }
}

void PMNGMainWindow::initPlotParams()
{
  m_plotParamsData.resize(m_plotParamsModel.indexFromItem(PlotParamsItems::LAST_INDEX));
  m_plotParamsModel.setUnderlyingData(&m_plotParamsData);

  m_plotParamsData[m_plotParamsModel.indexFromItem(PlotParamsItems::CUTOFF)] = 60.0;
  m_plotParamsData[m_plotParamsModel.indexFromItem(PlotParamsItems::INJ_ZONE_LENGTH)] = 1.0;

  m_plotParamsMapper = new QDataWidgetMapper{this};
  m_plotParamsMapper->setModel(&m_plotParamsModel);
  m_plotParamsMapper->addMapping(ui->qle_plotCutoff, m_plotParamsModel.indexFromItem(PlotParamsItems::CUTOFF));
  m_plotParamsMapper->addMapping(ui->qle_injZoneLength, m_plotParamsModel.indexFromItem(PlotParamsItems::INJ_ZONE_LENGTH));
  m_plotParamsMapper->toFirst();

  initSignalItems();
}

void PMNGMainWindow::initSignalItems()
{
  m_signalTypesModel = new QStandardItemModel{this};

  ui->qcbox_signal->setModel(m_signalTypesModel);
  resetSignalItems();
}

EFGDisplayer PMNGMainWindow::makeMainWindowEFGDisplayer()
{
  auto dispExecutor = [this](const QVector<QPointF> &data, std::vector<CalculatorInterface::SpatialZoneInformation> &&szi, const CalculatorInterface::Signal &signal) {
    m_signalPlotWidget->setSignal(data, plotSignalStyle(signal.type), signal.plotCaption, std::move(szi));
  };

  return EFGDisplayer(dispExecutor);
}

void PMNGMainWindow::onAbout()
{
  AboutDialog dlg{};
  dlg.exec();
}

void PMNGMainWindow::onAutoPlotCutoffStateChanged(const int state)
{
  ui->qle_plotCutoff->setDisabled(state == Qt::Checked);
}

void PMNGMainWindow::onCalculate()
{
  EFGDisplayer displayer = makeMainWindowEFGDisplayer();

  const QVariant lastSelectedSignal = resetSignalItems();

  double EOFValue;
  CalculatorInterface::EOFValueType EOFvt;
  inputToEOFValueType(EOFValue, EOFvt, m_mainCtrlWidget->EOFValue(), m_mainCtrlWidget->EOFInputType());
  const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();

  OperationInProgressDialog inProgDlg{"Calculating..."};

  CalculatorWorker worker{m_calcIface, rs.correctForDebyeHuckel, rs.correctForOnsagerFuoss, rs.correctForViscosity};
  QThread thread{};
  worker.moveToThread(&thread);

  connect(&thread, &QThread::started, &worker, &CalculatorWorker::process);
  connect(&worker, &CalculatorWorker::finished, &thread, &QThread::quit);
  connect(&worker, &CalculatorWorker::finished, &inProgDlg, &OperationInProgressDialog::onOperationCompleted);

  thread.start();
  inProgDlg.exec();
  thread.wait();

  const auto calcStatus = worker.calcStatus();
  if (calcStatus == CalculatorWorker::CalculationResult::INVALID) {
    QMessageBox errmbox{QMessageBox::Critical, tr("Calculation failed"), worker.errorMsg()};
    errmbox.exec();
    return;
  }

  try {
    m_calcIface.publishResults(rs.totalLength, rs.detectorPosition, rs.drivingVoltage, EOFValue, EOFvt, rs.positiveVoltage);
    if (calcStatus == CalculatorWorker::CalculationResult::OK) {
      addConstituentsSignals(m_calcIface.allConstituents());
      selectSignalIfAvailable(lastSelectedSignal);
      plotElectrophoregram(displayer);
    } else {
      QMessageBox errmbox{QMessageBox::Warning, tr("Calculation incomplete"),
                          QString{tr("Solver was unable to calculate electromigration properties of the system. "
                                     "Properties of background electrolyte, however, are available.\n\n"
                                     "Error reported by the sovler:\n"
                                     "%1")}.arg(worker.errorMsg())};
      errmbox.exec();
    }
  } catch (const CalculatorInterfaceException &ex) {
    QMessageBox errmbox{QMessageBox::Critical, tr("Cannot display results"), ex.what()};
    errmbox.exec();
  }
}

void PMNGMainWindow::onExportElectrophoregramAsCSV()
{
  try {
    plotElectrophoregram(EFGCSVExporter::make());
  } catch (CalculatorInterfaceException &ex) {
    QMessageBox mbox{QMessageBox::Critical, tr("Failed to export electrophoregram"), ex.what()};
    mbox.exec();
  }
}

void PMNGMainWindow::onCompositionChanged()
{
  m_calcIface.onInvalidate();
  m_signalPlotWidget->clear();
}

void PMNGMainWindow::onDatabaseEditor()
{
  DatabaseEditorDialog dlg{h_dbProxy, this};
  dlg.exec();
}

void PMNGMainWindow::onExit()
{
  close();
}

void PMNGMainWindow::onLoad()
{
  QFileDialog dlg{};
  dlg.setAcceptMode(QFileDialog::AcceptOpen);
  dlg.setNameFilter("JSON file (*.json)");

  if (m_lastLoadPath.length() > 0)
    dlg.setDirectory(m_lastLoadPath);

  if (dlg.exec() != QDialog::Accepted)
    return;

  const auto &files = dlg.selectedFiles();
  if (files.size() < 1)
    return;

  persistence::System system{};
  try {
    m_persistence.deserialize(files.at(0), system);

    QVariant eofType{};
    if (system.eofType == "N")
      eofType = QVariant::fromValue<MainControlWidget::EOF_Type>(MainControlWidget::EOF_NONE);
    else if (system.eofType == "U")
      eofType = QVariant::fromValue<MainControlWidget::EOF_Type>(MainControlWidget::EOF_MOBILITY);
    else if (system.eofType == "T")
      eofType = QVariant::fromValue<MainControlWidget::EOF_Type>(MainControlWidget::EOF_MARKER_TIME);
    else {
      QMessageBox mbox{QMessageBox::Warning, tr("Loading error"), tr("Invalid value of EOF type")};
      mbox.exec();
      return;
    }

    MainControlWidget::RunSetup rs{
      system.totalLength,
      system.detectorPosition,
      system.drivingVoltage,
      system.positiveVoltage,
      system.correctForDebyeHuckel,
      system.correctForOnsagerFuoss,
      system.correctForViscosity
    };

    onCompositionChanged();
    m_mainCtrlWidget->setRunSetup(rs, eofType, system.eofValue);

    m_lastLoadPath = QFileInfo(files.at(0)).absolutePath();
  } catch (persistence::DeserializationException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Unable to load system"), ex.what()};
    mbox.exec();
  }
}

void PMNGMainWindow::onNew()
{
  QMessageBox mbox{};
  mbox.setIcon(QMessageBox::Question);
  mbox.setText(tr("Create new system"));
  mbox.setInformativeText(tr("Are you sure you want to discard the current system and create a new one?"));
  mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  mbox.setDefaultButton(QMessageBox::No);
  if (mbox.exec() == QMessageBox::Yes)
    emit clearAll();
}

void PMNGMainWindow::onPlotElectrophoregram()
{
  try {
    EFGDisplayer displayer = makeMainWindowEFGDisplayer();
    plotElectrophoregram(displayer);
  } catch (CalculatorInterfaceException &ex) {
    QMessageBox mbox{QMessageBox::Critical, tr("Failed to plot electrophoregram"), ex.what()};
    mbox.exec();
  }
}

void PMNGMainWindow::onRunSetupChanged(const bool invalidate)
{
  h_scompWidget->setViscosityCorrectionEnabled(m_mainCtrlWidget->runSetup().correctForViscosity);

  if (invalidate)
    onCompositionChanged();
  else {
    double EOFValue;
    CalculatorInterface::EOFValueType EOFvt;
    inputToEOFValueType(EOFValue, EOFvt, m_mainCtrlWidget->EOFValue(), m_mainCtrlWidget->EOFInputType());

    const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();
    try {
      EFGDisplayer displayer = makeMainWindowEFGDisplayer();
      m_calcIface.recalculateTimes(rs.totalLength, rs.detectorPosition,
                                   rs.drivingVoltage,
                                   EOFValue, EOFvt,
                                   rs.positiveVoltage);
      plotElectrophoregram(displayer);
    } catch (const CalculatorInterfaceException &ex) {
      QMessageBox mbox{QMessageBox::Critical, tr("Failed to plot electrophoregram"), ex.what()};
      mbox.exec();
    }
  }
}

void PMNGMainWindow::onSave()
{
  QFileDialog dlg{};
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setNameFilter("JSON file (*.json)");
  dlg.setWindowTitle(tr("Save composition"));

  if (m_lastSavePath.length() > 0)
    dlg.setDirectory(m_lastSavePath);

  if (dlg.exec() != QDialog::Accepted)
    return;

  const auto &files = dlg.selectedFiles();
  if (files.size() < 1)
    return;

  const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();
  const QString et = [](MainControlWidget::EOF_Type t) {
      switch (t) {
      case MainControlWidget::EOF_NONE:
        return "N";
      case MainControlWidget::EOF_MOBILITY:
        return "U";
      case MainControlWidget::EOF_MARKER_TIME:
        return "T";
      return "";
      }
    }(m_mainCtrlWidget->EOFInputType());

  persistence::System sys{
    rs.totalLength,
    rs.detectorPosition,
    rs.drivingVoltage,
    rs.positiveVoltage,
    et,
    m_mainCtrlWidget->EOFValue(),
    rs.correctForDebyeHuckel,
    rs.correctForOnsagerFuoss,
    rs.correctForViscosity
  };

  try {
    m_persistence.serialize(files.at(0), sys);
    m_lastSavePath = QFileInfo(files.at(0)).absolutePath();
  } catch (persistence::SerializationException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Unable to save system"), ex.what()};
    mbox.exec();
  }
}

void PMNGMainWindow::plotElectrophoregram(const EFGDisplayer &displayer)
{
  if (!m_calcIface.resultsAvailable())
    return;

  const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();

  const double izLen = m_plotParamsData.at(m_plotParamsModel.indexFromItem(PlotParamsItems::INJ_ZONE_LENGTH));
  const double plotCutoff = [&]() {
    if (ui->qcb_autoPlotCutoff->isChecked())
      return -1.0;
    return m_plotParamsData.at(m_plotParamsModel.indexFromItem(PlotParamsItems::CUTOFF));
  }();

  /* Signal type to plot */
  const QStandardItem *sigItem = m_signalTypesModel->item(ui->qcbox_signal->currentIndex(), 0);
  if (sigItem == nullptr)
    return;
  if (!sigItem->data().canConvert<CalculatorInterface::Signal>())
    return;
  const auto &signal = sigItem->data().value<CalculatorInterface::Signal>();

  /* EOF mobility value */
  double EOFValue;
  CalculatorInterface::EOFValueType EOFvt;
  inputToEOFValueType(EOFValue, EOFvt, m_mainCtrlWidget->EOFValue(), m_mainCtrlWidget->EOFInputType());

  const auto signalTrace = m_calcIface.plotElectrophoregram(rs.totalLength, rs.detectorPosition,
                                                            rs.drivingVoltage, rs.positiveVoltage,
                                                            EOFValue, EOFvt, izLen, plotCutoff,
                                                            signal);
  auto zoneInfo = m_calcIface.spatialZoneInformation(rs.totalLength, rs.detectorPosition, rs.drivingVoltage,
                                                     EOFValue, EOFvt, rs.positiveVoltage);

  displayer(signalTrace, std::move(zoneInfo), signal);
}

QVariant PMNGMainWindow::resetSignalItems()
{
  const QVariant current = ui->qcbox_signal->currentData(Qt::UserRole + 1);
  m_signalTypesModel->clear();

  for (const auto &item : s_defaultSignalItems) {
    QStandardItem *si = new QStandardItem{item.name};
    si->setData(QVariant::fromValue<CalculatorInterface::Signal>(item.signal));
    m_signalTypesModel->appendRow(si);
  }

  return current;
}

void PMNGMainWindow::selectSignalIfAvailable(const QVariant &sig)
{
  const auto _sig = sig.value<CalculatorInterface::Signal>();

  for (int idx = 0; idx < m_signalTypesModel->rowCount(); idx++) {
    const auto _csig = m_signalTypesModel->data(m_signalTypesModel->index(idx, 0), Qt::UserRole + 1).value<CalculatorInterface::Signal>();
    if (_sig == _csig) {
      ui->qcbox_signal->setCurrentIndex(idx);
      return;
    }
  }
}

void PMNGMainWindow::setControlsIcons()
{
#ifdef Q_OS_LINUX
  /* Menu bar */
  ui->actionNew->setIcon(QIcon::fromTheme("document-new"));
  ui->actionLoad->setIcon(QIcon::fromTheme("document-open"));
  ui->actionSave->setIcon(QIcon::fromTheme("document-save"));
  ui->actionExit->setIcon(QIcon::fromTheme("application-exit"));
  ui->actionAbout->setIcon(QIcon::fromTheme("help-about"));

  /* Button bar */
  m_qpb_new->setIcon(QIcon::fromTheme("document-new"));
  m_qpb_load->setIcon(QIcon::fromTheme("document-open"));
  m_qpb_save->setIcon(QIcon::fromTheme("document-save"));
  m_qpb_calculate->setIcon(QIcon::fromTheme("media-playback-start"));
#else
  /* Menu bar */
  ui->actionNew->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
  ui->actionLoad->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  ui->actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  ui->actionExit->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
  ui->actionAbout->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));

  /* Button bar */
  m_qpb_new->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
  m_qpb_load->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  m_qpb_save->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  m_qpb_calculate->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
#endif // Q_OS_
}
