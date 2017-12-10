#include "pmngmainwindow.h"
#include "ui_pmngmainwindow.h"

#include "maincontrolwidget.h"
#include "signalplotwidget.h"
#include "../gearbox/results_models/resultsmodels.h"
#include "systemcompositionwidget.h"
#include "aboutdialog.h"
#include "../globals.h"
#include "../gearbox/calculatorworker.h"

#include <QDataWidgetMapper>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QThread>

#include <signal.h>

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
                               QWidget *parent) :
  QMainWindow{parent},
  m_calcIface{calcIface},
  m_signalPlotWidget{new SignalPlotWidget{this}},
  h_scompWidget{scompWidget},
  m_plotParamsModel{this},
  ui{new Ui::PMNGMainWindow}
{
  ui->setupUi(this);

  setWindowTitle(QString{"%1 %2.%3%4"}.arg(Globals::SOFTWARE_NAME).arg(Globals::VERSION_MAJ).arg(Globals::VERSION_MIN).arg(Globals::VERSION_REV));

  ui->qvlay_compositionEFG->insertWidget(0, h_scompWidget);
  ui->qvlay_compositionEFG->addWidget(m_signalPlotWidget);

  m_mainCtrlWidget = new MainControlWidget{resultsModels, this};
  ui->qvlay_leftPane->addWidget(m_mainCtrlWidget);

  initPlotParams();

  connect(ui->qcb_autoPlotCutoff, &QCheckBox::stateChanged, this, &PMNGMainWindow::onAutoPlotCutoffStateChanged);
  connect(ui->qcbox_signal, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PMNGMainWindow::onPlotElectrophoregram);

  m_qpb_calculate = new QPushButton{tr("Calculate!"), this};
  connect(m_qpb_calculate, &QPushButton::clicked, this, &PMNGMainWindow::onCalculate);

  connect(h_scompWidget, &SystemCompositionWidget::compositionChanged, this, &PMNGMainWindow::onCompositionChanged);
  connect(m_mainCtrlWidget, &MainControlWidget::runSetupChanged, this, &PMNGMainWindow::onRunSetupChanged);
  connect(ui->qpb_replotEFG, &QPushButton::clicked, this, &PMNGMainWindow::onPlotElectrophoregram);

  connect(ui->actionExit, &QAction::triggered, this, &PMNGMainWindow::onExit);
  connect(ui->actionCRASH, &QAction::triggered, this, &PMNGMainWindow::__onCrash);
  connect(ui->actionAbout, &QAction::triggered, this, &PMNGMainWindow::onAbout);

  ui->mainToolBar->addWidget(m_qpb_calculate);

  onAutoPlotCutoffStateChanged(Qt::Checked);

  setMenuIcons();
}

PMNGMainWindow::~PMNGMainWindow()
{
  delete ui;
}

void PMNGMainWindow::addConstituentsSignals(const QVector<QString> &constituents)
{
  for (const auto &a : constituents) {
    QStandardItem *si = new QStandardItem{QString{tr("Concentration (%1)")}.arg(a)};
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

void PMNGMainWindow::onAbout()
{
  AboutDialog dlg{};
  dlg.exec();
}

void PMNGMainWindow::onAutoPlotCutoffStateChanged(const int state)
{
  ui->qle_plotCutoff->setDisabled(state == Qt::Checked);
}

void PMNGMainWindow::__onCrash()
{
  ::raise(SIGSEGV);
}

void PMNGMainWindow::onCalculate()
{
  resetSignalItems();

  double EOFValue;
  CalculatorInterface::EOFValueType EOFvt;
  inputToEOFValueType(EOFValue, EOFvt, m_mainCtrlWidget->EOFValue(), m_mainCtrlWidget->EOFInputType());

  const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();

  bool calcOk;
  QString errorMsg;
  CalculatorWorker *worker = new CalculatorWorker{m_calcIface, rs, EOFValue, EOFvt, calcOk, errorMsg};
  QThread *thread = new QThread{};
  worker->moveToThread(thread);

  QMessageBox mbox(QMessageBox::Information, tr("Processing.."), tr("Hang in there, this can take a little while..."));
  mbox.setStandardButtons(0);
  connect(thread, &QThread::started, worker, &CalculatorWorker::process);
  connect(worker, &CalculatorWorker::finished, thread, &QThread::quit);
  connect(thread, &QThread::finished, &mbox, &QMessageBox::accept);
  thread->start();

  mbox.exec();

  thread->wait();

  if (calcOk) {
    m_calcIface.publishResults(rs.totalLength, rs.detectorPosition, rs.drivingVoltage, EOFValue, EOFvt, rs.positiveVoltage);
    addConstituentsSignals(m_calcIface.allConstituents());
    plotElectrophoregram();
  } else {
    QMessageBox mbox{QMessageBox::Critical, tr("Calculation failed"), errorMsg};
    mbox.exec();
  }

  worker->deleteLater();
  thread->deleteLater();
}

void PMNGMainWindow::onCompositionChanged()
{
  m_calcIface.onInvalidate();
  m_signalPlotWidget->clear();
}

void PMNGMainWindow::onExit()
{
  close();
}

void PMNGMainWindow::onPlotElectrophoregram()
{
  try {
    plotElectrophoregram();
  } catch (CalculatorInterfaceException &ex) {
    QMessageBox mbox{QMessageBox::Critical, tr("Failed to plot electrophoregram"), ex.what()};
    mbox.exec();
  }
}

void PMNGMainWindow::onRunSetupChanged(const bool invalidate)
{
  if (invalidate)
    onCompositionChanged();
  else {
    double EOFValue;
    CalculatorInterface::EOFValueType EOFvt;
    inputToEOFValueType(EOFValue, EOFvt, m_mainCtrlWidget->EOFValue(), m_mainCtrlWidget->EOFInputType());

    const MainControlWidget::RunSetup rs = m_mainCtrlWidget->runSetup();
    m_calcIface.recalculateTimes(rs.totalLength, rs.detectorPosition,
                                 rs.drivingVoltage,
                                 EOFValue, EOFvt,
                                 rs.positiveVoltage);
    plotElectrophoregram();
  }
}

void PMNGMainWindow::plotElectrophoregram()
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
  m_signalPlotWidget->setSignal(signalTrace, plotSignalStyle(signal.type), signal.plotCaption);
}

void PMNGMainWindow::resetSignalItems()
{
  m_signalTypesModel->clear();

  for (const auto &item : s_defaultSignalItems) {
    QStandardItem *si = new QStandardItem{item.name};
    si->setData(QVariant::fromValue<CalculatorInterface::Signal>(item.signal));
    m_signalTypesModel->appendRow(si);
  }
}

void PMNGMainWindow::setMenuIcons()
{
#ifdef Q_OS_LINUX
  ui->actionExit->setIcon(QIcon::fromTheme("application-exit"));
  ui->actionAbout->setIcon(QIcon::fromTheme("help-about"));
#else
  ui->actionExit->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
  ui->actionAbout->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
#endif // Q_OS
}
