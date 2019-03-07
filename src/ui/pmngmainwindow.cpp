#include "pmngmainwindow.h"
#include "ui_pmngmainwindow.h"

#include "maincontrolwidget.h"
#include "signalplotwidget.h"
#include "../gearbox/results_models/resultsmodels.h"
#include "../gearbox/results_models/analytesextrainfomodel.h"
#include "../gearbox/results_models/eigenzonedetailsmodel.h"
#include "systemcompositionwidget.h"
#include "aboutdialog.h"
#include "../globals.h"
#include "../gearbox/calculatorworker.h"
#include "../persistence/persistence.h"
#include "operationinprogressdialog.h"
#include "../gearbox/efgdisplayer.h"
#include "../gearbox/efgcsvexporter.h"
#include "databaseeditordialog.h"
#include "../gearbox/databaseproxy.h"
#include "toggletracepointsdialog.h"
#include "hacks.h"
#include "checkforupdatedialog.h"
#include "../softwareupdater.h"
#include "elementaries/uihelpers.h"
#include "refocuser.h"

#include <cassert>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QShortcut>
#include <QScreen>
#include <QSplitter>
#include <QThread>
#include <QTimer>
#include <QWindow>

#define THEY_LIVE

class Inhibitor {
public:
  Inhibitor(bool &val) :
    h_val{val}
  {
    val = true;
  }
  ~Inhibitor()
  {
    h_val = false;
  }

private:
  bool &h_val;
};

class InvalidEOFMarkerTime : public std::runtime_error {
public:
  InvalidEOFMarkerTime() : std::runtime_error{"EOF marker time is invalid"}
  {}
};

static
void displayInvalidEOFMsgBox()
{
    QMessageBox mbox{QMessageBox::Critical, QObject::tr("Calculation failed"),
                     QObject::tr("Value of EOF marker must be positive.")};

    mbox.exec();
}

static
void inputToEOFValueType(double &EOFValue, CalculatorInterface::EOFValueType &EOFvt, const double inValue, const MainControlWidget::EOF_Type type)
{
  if (type == MainControlWidget::EOF_MOBILITY) {
    EOFValue = inValue;
    EOFvt = CalculatorInterface::EOFValueType::MOBILITY;
  } else if (type == MainControlWidget::EOF_MARKER_TIME) {
    if (inValue <= 0.0)
      throw InvalidEOFMarkerTime{};
    EOFValue = inValue * 60.0;
    EOFvt = CalculatorInterface::EOFValueType::MARKER_TIME;
  } else {
    EOFValue = 0.0;
    EOFvt = CalculatorInterface::EOFValueType::MOBILITY;
  }
}

static
QVector<AnalytesExtraInfoModel::ExtraInfo> makeAnalytesExtraInfo(const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi,
                                                                 const QAbstractTableModel * const ezDetailsModelBase)
{
  const EigenzoneDetailsModel *ezDetailsModel = static_cast<const EigenzoneDetailsModel *>(ezDetailsModelBase);
  QVector<AnalytesExtraInfoModel::ExtraInfo> data{};

  assert(tdzi.size() == static_cast<size_t>(ezDetailsModel->columnCount()));

  for (int idx = 0; idx < ezDetailsModel->columnCount(); idx++) {
    const auto &tdInfo = tdzi.at(idx);
    const auto &ezProps = ezDetailsModel->eigenzonePropsAt(idx);
    if (tdInfo.isSystemZone)
      continue;

    const double uEff = ezProps.mobility;
    const double uEMD = ezProps.uEMD;
    const bool detected = [&tdInfo]() {
      return tdInfo.timeMax > 0.0 && tdInfo.beginsAt > 0.0;
    }();

    data.append(AnalytesExtraInfoModel::ExtraInfo{tdInfo.name, uEff, tdInfo.timeMax, uEMD, tdInfo.concentrationMax, tdInfo.conductivityMax, detected});
  }

  return data;
}

static
void makeYesNoMessagebox(QMessageBox &mbox, const QString &title, const QString &text)
{
  mbox.setIcon(QMessageBox::Question);
  mbox.setText(title);
  mbox.setInformativeText(text);
  mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  mbox.setDefaultButton(QMessageBox::No);
}

static
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

  throw std::runtime_error("Unhandled signal type value");
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
                               AnalytesExtraInfoModel * const analytesEXIModel, const QAbstractTableModel * const eigenzoneDetailsModel,
                               QWidget *parent) :
  QMainWindow{parent},
  m_calcIface{calcIface},
  m_signalPlotWidget{new SignalPlotWidget{this}},
  h_scompWidget{scompWidget},
  h_analytesEXIModel{analytesEXIModel},
  h_eigenzoneDetailsModel{eigenzoneDetailsModel},
  m_plotParamsModel{this},
  m_persistence{persistence},
  m_lastLoadPath{""},
  m_lastSavePath{""},
  h_dbProxy{dbProxy},
  m_fullCalcInProgress{false},
  ui{new Ui::PMNGMainWindow}
{
  ui->setupUi(this);

  auto splitter = new QSplitter{Qt::Vertical, this};
  hacks::makeSplitterAppear(splitter);
  ui->qvlay_compositionEFG->insertWidget(0, splitter);

  splitter->addWidget(h_scompWidget);
  splitter->addWidget(m_signalPlotWidget);

  m_mainCtrlWidget = new MainControlWidget{resultsModels, this};
  ui->qvlay_leftPane->addWidget(m_mainCtrlWidget);

  m_checkForUpdateDlg = new CheckForUpdateDialog{this};

  initPlotParams();

  m_qpb_new = new QPushButton{tr("New"), this};
  connect(m_qpb_new, &QPushButton::clicked, ui->actionNew, &QAction::trigger);
  m_qpb_load = new QPushButton{tr("Load"), this};
  connect(m_qpb_load, &QPushButton::clicked, ui->actionLoad, &QAction::trigger);
  m_qpb_save = new QPushButton{tr("Save"), this};
  connect(m_qpb_save, &QPushButton::clicked, ui->actionSave, &QAction::trigger);
  m_qpb_saveAs = new QPushButton{tr("Save as"), this};
  connect(m_qpb_saveAs, &QPushButton::clicked, ui->actionSave_as, &QAction::trigger);
  m_qpb_calculate = new QPushButton{tr("Calculate!"), this};
  connect(m_qpb_calculate, &QPushButton::clicked, this, &PMNGMainWindow::onCalculate);
  connect(m_signalPlotWidget, &SignalPlotWidget::replotElectrophoregram, this, &PMNGMainWindow::onPlotElectrophoregram);

  connect(h_scompWidget, &SystemCompositionWidget::compositionChanged, this, &PMNGMainWindow::onCompositionChanged);
  connect(m_mainCtrlWidget, &MainControlWidget::runSetupChanged, this, &PMNGMainWindow::onRunSetupChanged);

  connect(ui->actionExit, &QAction::triggered, this, &PMNGMainWindow::onExit);
  connect(ui->actionAbout, &QAction::triggered, this, &PMNGMainWindow::onAbout);
  connect(ui->actionNew, &QAction::triggered, this, &PMNGMainWindow::onNew);
  connect(ui->actionLoad, &QAction::triggered, this, &PMNGMainWindow::onLoad);
  connect(ui->actionSave, &QAction::triggered, this, &PMNGMainWindow::onSave);
  connect(ui->actionSave_as, &QAction::triggered, this, &PMNGMainWindow::onSaveAs);
  connect(ui->actionExportEFGAsCSV, &QAction::triggered, this, &PMNGMainWindow::onExportElectrophoregramAsCSV);
  connect(ui->actionDatabase_editor, &QAction::triggered, this, &PMNGMainWindow::onDatabaseEditor);
  connect(ui->actionOpen_database, &QAction::triggered, this, &PMNGMainWindow::onOpenDatabase);
  connect(ui->actionSet_debugging_output, &QAction::triggered, this, &PMNGMainWindow::onSetDebuggingOutput);
  connect(ui->actionCheck_for_update, &QAction::triggered, this, &PMNGMainWindow::onOpenUpdateDialog);

  ui->mainToolBar->addWidget(m_qpb_new);
  ui->mainToolBar->addWidget(m_qpb_load);
  ui->mainToolBar->addWidget(m_qpb_save);
  ui->mainToolBar->addWidget(m_qpb_saveAs);
  ui->mainToolBar->addWidget(m_qpb_calculate);

  m_calculateShortcut = new QShortcut(QKeySequence::Refresh, this);
  m_calculateShortcutTwo = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this);
  connect(m_calculateShortcut, &QShortcut::activated, this, &PMNGMainWindow::onCalculate);
  connect(m_calculateShortcutTwo, &QShortcut::activated, this, &PMNGMainWindow::onCalculate);

  setControlsIcons();

  h_scompWidget->setViscosityCorrectionEnabled(m_mainCtrlWidget->runSetup().correctForViscosity);

  setWindowIcon(Globals::icon());

#ifdef THEY_LIVE
  QMainWindow::setWindowTitle(QString("%1 %2").arg(QString::fromUtf8("\x49\x20\x63\x61\x6d\x65\x20\x68\x65\x72\x65\x20\x74\x6f\x20\x64\x72\x61\x77"
                                                                     "\x20\x70\x65\x61\x6b\x73\x20\x61\x6e\x64\x20\x63\x68\x65\x77\x20\x62\x75\x62"
                                                                     "\x62\x6c\x65\x67\x75\x6d\x2e\x2e\x2e\x20\x61\x6e\x64\x20\x49\x27\x6d\x20\x61"
                                                                     "\x6c\x6c\x20\x6f\x75\x74\x74\x61\x20\x67\x75\x6d"),
                                                                      Globals::VERSION_STRING()));

  QTimer::singleShot(4466, this, [this]() { this->setWindowTitle(Globals::VERSION_STRING()); });
#else
  setWindowTitle(Globals::VERSION_STRING());
#endif // THEY_LIVE

  QTimer::singleShot(0, this, &PMNGMainWindow::connectOnScreenChanged); /* This must be done from the event queue after the window is created */
  QTimer::singleShot(0, this, [this]() { onScreenChanged(UIHelpers::findScreenForWidget(this)); });
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

void PMNGMainWindow::closeEvent(QCloseEvent *evt)
{
  QMessageBox mbox{QMessageBox::Question, tr("Confirm action"),
                   QString{"Do you really want to exit %1?\nAll unsaved data will be lost."}.arg(Globals::SOFTWARE_NAME),
                   QMessageBox::Yes | QMessageBox::No};

  if (mbox.exec() != QMessageBox::Yes)
    evt->ignore();
}

void PMNGMainWindow::connectOnScreenChanged()
{
  connect(this->window()->windowHandle(), &QWindow::screenChanged, this, &PMNGMainWindow::onScreenChanged);
}

void PMNGMainWindow::connectUpdater(SoftwareUpdater *const updater)
{
  connect(m_checkForUpdateDlg, &CheckForUpdateDialog::checkForUpdate, updater, &SoftwareUpdater::onCheckForUpdate);
  connect(updater, &SoftwareUpdater::checkComplete, m_checkForUpdateDlg, &CheckForUpdateDialog::onCheckComplete);
}

void PMNGMainWindow::initPlotParams()
{
  m_plotParamsData.resize(m_plotParamsModel.indexFromItem(SignalPlotWidget::PlotParamsItems::LAST_INDEX));

  m_plotParamsData[m_plotParamsModel.indexFromItem(SignalPlotWidget::PlotParamsItems::CUTOFF)] = 60.0;
  m_plotParamsData[m_plotParamsModel.indexFromItem(SignalPlotWidget::PlotParamsItems::INJ_ZONE_LENGTH)] = 1.0;

  m_plotParamsModel.setUnderlyingData(&m_plotParamsData);

  m_signalPlotWidget->setPlotParamsMapper(&m_plotParamsModel);

  initSignalItems();
}

void PMNGMainWindow::initSignalItems()
{
  m_signalTypesModel = new QStandardItemModel{this};

  m_signalPlotWidget->setSignalItemsModel(m_signalTypesModel);
  resetSignalItems();
}

EFGDisplayer PMNGMainWindow::makeMainWindowEFGDisplayer()
{
  auto dispExecutor = [this](const QVector<QPointF> &data, const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi, const CalculatorInterface::Signal &signal) {
    m_signalPlotWidget->setSignal(data, plotSignalStyle(signal.type), signal.signalName, tdzi);
  };

  return EFGDisplayer(dispExecutor);
}

PMNGMainWindow::PlottingInfo PMNGMainWindow::makePlottingInfo()
{
  double EOFValue;
  CalculatorInterface::EOFValueType EOFvt;
  inputToEOFValueType(EOFValue, EOFvt, m_mainCtrlWidget->EOFValue(), m_mainCtrlWidget->EOFInputType());

  const double izLen = m_plotParamsData.at(m_plotParamsModel.indexFromItem(SignalPlotWidget::PlotParamsItems::INJ_ZONE_LENGTH));
  const double plotCutoff = [&]() {
    if (m_signalPlotWidget->autoPlotCutoff())
      return -1.0;
    return m_plotParamsData.at(m_plotParamsModel.indexFromItem(SignalPlotWidget::PlotParamsItems::CUTOFF));
  }();

  return PlottingInfo{ EOFValue, EOFvt, izLen, plotCutoff };
}

void PMNGMainWindow::onAbout()
{
  AboutDialog dlg{};
  dlg.exec();
}

void PMNGMainWindow::onCalculate()
{
  Inhibitor ihb{m_fullCalcInProgress};
  Refocuser refoc{};

  EFGDisplayer displayer = makeMainWindowEFGDisplayer();

  const QVariant lastSelectedSignal = resetSignalItems();

  PlottingInfo plotInfo{};
  try {
    plotInfo = makePlottingInfo();
  } catch (const InvalidEOFMarkerTime &)  {
    displayInvalidEOFMsgBox();
    return;
  }

  const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();
  h_scompWidget->commit();

  if (m_tracingSetup.tracingEnabled)
    m_calcIface.setTracepoints(m_tracingSetup.tracepointStates);
  else
    m_calcIface.disableAllTracepoints();

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

  const auto calcResult = worker.calcResult();
  if (calcResult == CalculatorWorker::CalculationResult::INVALID) {
    QMessageBox errmbox{QMessageBox::Critical, tr("Calculation failed"), worker.errorMsg()};
    errmbox.exec();
    return;
  }

  try {
    m_calcIface.publishResults(rs.totalLength, rs.detectorPosition, rs.drivingVoltage,
                               plotInfo.EOFValue, plotInfo.EOFvt, rs.positiveVoltage);
    if (calcResult == CalculatorWorker::CalculationResult::OK) {
      const auto tdzi =  m_calcIface.timeDependentZoneInformation(rs.totalLength, rs.detectorPosition, rs.drivingVoltage,
                                                                  plotInfo.EOFValue, plotInfo.EOFvt, rs.positiveVoltage,
                                                                  plotInfo.injZoneLength, plotInfo.plotCutoff);
      const auto exInfo = makeAnalytesExtraInfo(tdzi, h_eigenzoneDetailsModel);

      addConstituentsSignals(m_calcIface.allConstituents());
      selectSignalIfAvailable(lastSelectedSignal);
      plotElectrophoregram(displayer, tdzi, plotInfo.EOFValue, plotInfo.EOFvt, plotInfo.injZoneLength, plotInfo.plotCutoff);
      h_analytesEXIModel->setData(exInfo);
    } else if (calcResult == CalculatorWorker::CalculationResult::PARTIAL_BGE) {
      QMessageBox errmbox{QMessageBox::Warning, tr("Calculation incomplete"),
                          QString{tr("Solver was unable to calculate electromigration properties of the system. "
                                     "Properties of background electrolyte, however, are available.\n\n"
                                     "Error reported by the solver:\n"
                                     "%1")}.arg(worker.errorMsg())};
      errmbox.exec();
    } else if (calcResult == CalculatorWorker::CalculationResult::PARTIAL_EIGENZONES) {
      QMessageBox errmbox{QMessageBox::Warning, tr("Calculation incomplete"),
                          QString{tr("Solver was unable to calculate properties of some of the eigenzones. "
                                     "You can still use the results but keep in mind that properties "
                                     "of eigenzones displayed in red might be incorrect.\n\n"
                                     "Error reported by the solver:\n"
                                     "%1")}.arg(worker.errorMsg())};
      errmbox.exec();

      const auto tdzi =  m_calcIface.timeDependentZoneInformation(rs.totalLength, rs.detectorPosition, rs.drivingVoltage,
                                                                  plotInfo.EOFValue, plotInfo.EOFvt, rs.positiveVoltage,
                                                                  plotInfo.injZoneLength, plotInfo.plotCutoff);
      const auto exInfo = makeAnalytesExtraInfo(tdzi, h_eigenzoneDetailsModel);

      addConstituentsSignals(m_calcIface.allConstituents());
      selectSignalIfAvailable(lastSelectedSignal);
      plotElectrophoregram(displayer, tdzi, plotInfo.EOFValue, plotInfo.EOFvt, plotInfo.injZoneLength, plotInfo.plotCutoff);
      h_analytesEXIModel->setData(exInfo);
    }
  } catch (const CalculatorInterfaceException &ex) {
    QMessageBox errmbox{QMessageBox::Critical, tr("Cannot display results"), ex.what()};
    errmbox.exec();
  }

  if (m_tracingSetup.tracingEnabled) {
    const bool ret = m_calcIface.writeTrace(m_tracingSetup.outputFilePath.toStdString());

    if (!ret) {
      QMessageBox errBox{QMessageBox::Warning, tr("Trace"), tr("Failed to write trace file")};
      errBox.exec();
    }
  }
}

void PMNGMainWindow::onExportElectrophoregramAsCSV()
{
  try {
    const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();
    const auto plotInfo = makePlottingInfo();

    const auto tdzi =  m_calcIface.timeDependentZoneInformation(rs.totalLength, rs.detectorPosition, rs.drivingVoltage,
                                                                plotInfo.EOFValue, plotInfo.EOFvt, rs.positiveVoltage,
                                                                plotInfo.injZoneLength, plotInfo.plotCutoff);

    plotElectrophoregram(EFGCSVExporter::make(), tdzi,
                         plotInfo.EOFValue, plotInfo.EOFvt, plotInfo.injZoneLength, plotInfo.plotCutoff);
  } catch (CalculatorInterfaceException &ex) {
    QMessageBox mbox{QMessageBox::Critical, tr("Failed to export electrophoregram"), ex.what()};
    mbox.exec();
  } catch (const InvalidEOFMarkerTime &) {
    displayInvalidEOFMsgBox();
  }
}

void PMNGMainWindow::onCompositionChanged()
{
  if (m_fullCalcInProgress)
    return;

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

  if (!m_lastLoadPath.isEmpty())
    dlg.setDirectory(QFileInfo{m_lastLoadPath}.absolutePath());

  if (dlg.exec() != QDialog::Accepted)
    return;

  const auto &files = dlg.selectedFiles();
  if (files.empty())
    return;

  persistence::System system{};
  try {
    const auto &path = QFileInfo{files.at(0)}.absoluteFilePath();
    m_persistence.deserialize(path, system);

    QVariant eofType{};
    if (system.eofType == persistence::Persistence::SYS_EOF_TYPE_NONE)
      eofType = QVariant::fromValue<MainControlWidget::EOF_Type>(MainControlWidget::EOF_NONE);
    else if (system.eofType == persistence::Persistence::SYS_EOF_TYPE_MOBILITY)
      eofType = QVariant::fromValue<MainControlWidget::EOF_Type>(MainControlWidget::EOF_MOBILITY);
    else if (system.eofType == persistence::Persistence::SYS_EOF_TYPE_TIME)
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

    m_plotParamsData[m_plotParamsModel.indexFromItem(SignalPlotWidget::PlotParamsItems::INJ_ZONE_LENGTH)] = system.injectionZoneLength;
    m_plotParamsModel.notifyDataChanged(SignalPlotWidget::PlotParamsItems::INJ_ZONE_LENGTH, SignalPlotWidget::PlotParamsItems::INJ_ZONE_LENGTH);

    onCompositionChanged();
    m_mainCtrlWidget->setRunSetup(rs, eofType, system.eofValue);

    m_lastLoadPath = path;
    m_activeFile = ActiveFile{path};

    setWindowTitle(QFileInfo{path}.fileName());
  } catch (persistence::DeserializationException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Unable to load system"), ex.what()};
    mbox.exec();
  }
}

void PMNGMainWindow::onNew()
{
  QMessageBox mbox{};
  makeYesNoMessagebox(mbox,
                      tr("Create new system"),
                      tr("Are you sure you want to discard the current system and create a new one?"));

  if (mbox.exec() == QMessageBox::Yes) {
    emit clearAll();
    m_activeFile = ActiveFile{};
    setWindowTitle();
  }
}

void PMNGMainWindow::onOpenDatabase()
{
  QFileDialog dlg{this, tr("Load database file")};

  dlg.setAcceptMode(QFileDialog::AcceptOpen);
  dlg.setNameFilter("SQLite3 database (*.sql)");

  if (dlg.exec() == QDialog::Accepted) {
    if (!h_dbProxy.openDatabase(dlg.selectedFiles().at(0))) {
      QMessageBox errBox{QMessageBox::Warning, tr("Database error"), tr("Cannot open selected database file")};
      errBox.exec();
    }
  }
}

void PMNGMainWindow::onOpenUpdateDialog()
{
  m_checkForUpdateDlg->exec();
}

void PMNGMainWindow::onPlotElectrophoregram()
{
  try {
    EFGDisplayer displayer = makeMainWindowEFGDisplayer();

    const auto plotInfo = makePlottingInfo();
    const auto rs = m_mainCtrlWidget->runSetup();
    const auto tdzi = m_calcIface.timeDependentZoneInformation(rs.totalLength, rs.detectorPosition, rs.drivingVoltage,
                                                               plotInfo.EOFValue, plotInfo.EOFvt,
                                                               rs.positiveVoltage,
                                                               plotInfo.injZoneLength, plotInfo.plotCutoff);

    plotElectrophoregram(displayer, tdzi, plotInfo.EOFValue, plotInfo.EOFvt, plotInfo.injZoneLength, plotInfo.plotCutoff);
  } catch (CalculatorInterfaceException &ex) {
    QMessageBox mbox{QMessageBox::Critical, tr("Failed to plot electrophoregram"), ex.what()};
    mbox.exec();
  } catch (const InvalidEOFMarkerTime &) {
    /* Do not yell at the user unnecessarily */
  }
}

void PMNGMainWindow::onRunSetupChanged(const bool invalidate)
{
  if (m_fullCalcInProgress)
    return;

  h_scompWidget->setViscosityCorrectionEnabled(m_mainCtrlWidget->runSetup().correctForViscosity);

  if (invalidate)
    onCompositionChanged();
  else {
    try {
      const auto plotInfo = makePlottingInfo();
      const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();

      EFGDisplayer displayer = makeMainWindowEFGDisplayer();
      m_calcIface.recalculateTimes(rs.totalLength, rs.detectorPosition,
                                   rs.drivingVoltage,
                                   plotInfo.EOFValue, plotInfo.EOFvt,
                                   rs.positiveVoltage);

      const auto tdzi = m_calcIface.timeDependentZoneInformation(rs.totalLength, rs.detectorPosition, rs.drivingVoltage,
                                                                 plotInfo.EOFValue, plotInfo.EOFvt, rs.positiveVoltage,
                                                                 plotInfo.injZoneLength, plotInfo.plotCutoff);
      const auto exInfo = makeAnalytesExtraInfo(tdzi, h_eigenzoneDetailsModel);

      plotElectrophoregram(displayer, tdzi, plotInfo.EOFValue, plotInfo.EOFvt, plotInfo.injZoneLength, plotInfo.plotCutoff);
      h_analytesEXIModel->setData(exInfo);
    } catch (const CalculatorInterfaceException &ex) {
      QMessageBox mbox{QMessageBox::Critical, tr("Failed to plot electrophoregram"), ex.what()};
      mbox.exec();
    } catch (const InvalidEOFMarkerTime &) {
      return; /* Do not yell at the user unnecessarily */
    }
  }
}

void PMNGMainWindow::onSave()
{
  if (!m_activeFile.valid)
    onSaveAs();
  else {
    QMessageBox mbox{};
    makeYesNoMessagebox(mbox,
                        tr("Confim action"),
                        QString{tr("Overwrite existing file %1?")}.arg(QFileInfo{m_activeFile.path()}.fileName()));

    if (mbox.exec() == QMessageBox::Yes)
      saveSystem(m_activeFile.path());
  }
}

void PMNGMainWindow::onSaveAs()
{
  QFileDialog dlg{};
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setNameFilter("JSON file (*.json)");
  dlg.setWindowTitle(tr("Save composition"));

  if (!m_lastSavePath.isEmpty())
    dlg.setDirectory(QFileInfo{m_lastSavePath}.absolutePath());

  if (dlg.exec() != QDialog::Accepted)
    return;

  const auto &files = dlg.selectedFiles();
  if (files.empty())
    return;

  saveSystem(files.at(0));
}

void PMNGMainWindow::onScreenChanged(QScreen *screen)
{
  static const int MIN_SIZE{800};

  if (screen == nullptr)
    return;

  const qreal dpi = screen->logicalDotsPerInch();
  int screenHeight = screen->geometry().height();

  screenHeight = qRound(screenHeight * dpi / 96.0);
  if (screenHeight <= MIN_SIZE) {
    const int adjSize = qRound(0.7 * screenHeight);
    setMinimumHeight(adjSize);
  } else {
    setMinimumHeight(MIN_SIZE);
  }
}

void PMNGMainWindow::onSetDebuggingOutput()
{
  static QSize dlgSize;
  const auto tracepointInformation = m_calcIface.tracepointInformation();

  ToggleTracepointsDialog dlg{tracepointInformation, m_tracingSetup, this};
  if (!dlgSize.isEmpty())
    dlg.resize(dlgSize);

  const int ret = dlg.exec();
  dlgSize = dlg.size();

  if (ret != QDialog::Accepted)
    return;

  m_tracingSetup = dlg.result();
}

void PMNGMainWindow::plotElectrophoregram(const EFGDisplayer &displayer, const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi,
                                          const double EOFValue, const CalculatorInterface::EOFValueType EOFvt,
                                          const double izLen, const double plotCutoff)
{
  if (!m_calcIface.resultsAvailable())
    return;

  const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();

  /* Signal type to plot */
  const QStandardItem *sigItem = m_signalTypesModel->item(m_signalPlotWidget->selectedSignalIndex(), 0);
  if (sigItem == nullptr)
    return;
  if (!sigItem->data().canConvert<CalculatorInterface::Signal>())
    return;
  const auto &signal = sigItem->data().value<CalculatorInterface::Signal>();

  const auto signalTrace = m_calcIface.plotElectrophoregram(rs.totalLength, rs.detectorPosition,
                                                            rs.drivingVoltage, rs.positiveVoltage,
                                                            EOFValue, EOFvt, izLen, plotCutoff,
                                                            signal);

  const auto exInfo = makeAnalytesExtraInfo(tdzi, h_eigenzoneDetailsModel);
  h_analytesEXIModel->setData(exInfo);
  displayer(signalTrace, tdzi, signal);
}

QVariant PMNGMainWindow::resetSignalItems()
{
  //const QVariant current = ui->qcbox_signal->currentData(Qt::UserRole + 1);
  const QVariant current = m_signalTypesModel->data(m_signalTypesModel->index(m_signalPlotWidget->selectedSignalIndex(), 0), Qt::UserRole + 1);
  m_signalTypesModel->clear();

  for (const auto &item : s_defaultSignalItems) {
    auto si = new QStandardItem{item.name};
    si->setData(QVariant::fromValue<CalculatorInterface::Signal>(item.signal));
    m_signalTypesModel->appendRow(si);
  }

  return current;
}

void PMNGMainWindow::saveSystem(const QString &m_path)
{
  const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();
  const QString et = [](MainControlWidget::EOF_Type t) {
      switch (t) {
      case MainControlWidget::EOF_NONE:
        return persistence::Persistence::SYS_EOF_TYPE_NONE;
      case MainControlWidget::EOF_MOBILITY:
        return persistence::Persistence::SYS_EOF_TYPE_MOBILITY;
      case MainControlWidget::EOF_MARKER_TIME:
        return persistence::Persistence::SYS_EOF_TYPE_TIME;
      }
      return QString{};
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
    rs.correctForViscosity,
    m_plotParamsData.at(m_plotParamsModel.indexFromItem(SignalPlotWidget::PlotParamsItems::INJ_ZONE_LENGTH))
  };

  try {
    m_persistence.serialize(m_path, sys);
    m_lastSavePath = m_path;

    m_activeFile = ActiveFile{m_path};
    setWindowTitle(QFileInfo{m_path}.fileName());
  } catch (persistence::SerializationException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Unable to save system"), ex.what()};
    mbox.exec();
  }
}

void PMNGMainWindow::selectSignalIfAvailable(const QVariant &sig)
{
  const auto _sig = sig.value<CalculatorInterface::Signal>();

  for (int idx = 0; idx < m_signalTypesModel->rowCount(); idx++) {
    const auto _csig = m_signalTypesModel->data(m_signalTypesModel->index(idx, 0), Qt::UserRole + 1).value<CalculatorInterface::Signal>();
    if (_sig == _csig) {
      m_signalPlotWidget->setSignalIndex(idx);
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
  ui->actionSave_as->setIcon(QIcon::fromTheme("document-save-as"));
  ui->actionExit->setIcon(QIcon::fromTheme("application-exit"));
  ui->actionAbout->setIcon(QIcon::fromTheme("help-about"));
  ui->actionCheck_for_update->setIcon(QIcon::fromTheme("system-software-update"));

  /* Button bar */
  m_qpb_new->setIcon(QIcon::fromTheme("document-new"));
  m_qpb_load->setIcon(QIcon::fromTheme("document-open"));
  m_qpb_save->setIcon(QIcon::fromTheme("document-save"));
  m_qpb_saveAs->setIcon(QIcon::fromTheme("document-save-as"));
  m_qpb_calculate->setIcon(QIcon::fromTheme("media-playback-start"));
#else
  /* Menu bar */
  ui->actionNew->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
  ui->actionLoad->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  ui->actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  ui->actionSave_as->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  ui->actionExit->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
  ui->actionAbout->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
  ui->actionCheck_for_update->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

  /* Button bar */
  m_qpb_new->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
  m_qpb_load->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  m_qpb_save->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  m_qpb_saveAs->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  m_qpb_calculate->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
#endif // Q_OS_
}

void PMNGMainWindow::setWindowTitle(const QString &file)
{
  if (file.isEmpty())
    QMainWindow::setWindowTitle(Globals::VERSION_STRING());
  else
    QMainWindow::setWindowTitle(QString{"%1 - %2"}.arg(Globals::VERSION_STRING(), file));
}
