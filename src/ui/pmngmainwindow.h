#ifndef PMNGMAINWINDOW_H
#define PMNGMAINWINDOW_H

#include "../gearbox/calculatorinterface.h"
#include "../mappers/floatmappermodel.h"
#include "signalplotwidget.h"
#include "toggletracepointsdialog.h"

#include <QMainWindow>
#include <vector>

namespace Ui {
  class PMNGMainWindow;
}

namespace persistence {
  class Persistence;
}

class AnalytesExtraInfoModel;
class CalculatorInterface;
class CheckForUpdateDialog;
class DatabaseProxy;
class EFGDisplayer;
class MainControlWidget;
class ResultsModels;
class QPushButton;
class QShortcut;
class QStandardItemModel;
class SignalPlotWidget;
class SoftwareUpdater;
class SoftwareUpdateResult;
class SystemCompositionWidget;

class PMNGMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit PMNGMainWindow(SystemCompositionWidget *scompWidget,
                          CalculatorInterface &&calcIface, ResultsModels resultsModels,
                          persistence::Persistence &persistence,
                          DatabaseProxy &dbProxy,
                          AnalytesExtraInfoModel * const analytesEXIModel, const QAbstractTableModel * const eigenzoneDetailsModel,
                          QWidget *parent = nullptr);
  ~PMNGMainWindow();
  void connectUpdater(SoftwareUpdater *updater);

private:
  class PlottingInfo {
  public:
    double EOFValue;
    CalculatorInterface::EOFValueType EOFvt;
    double injZoneLength;
    double plotCutoff;
  };

  class SignalItem {
  public:
    QString name;
    CalculatorInterface::Signal signal;
  };

  void addConstituentsSignals(const QVector<QString> &analytes);
  EFGDisplayer makeMainWindowEFGDisplayer();
  void initPlotParams();
  void initSignalItems();
  PlottingInfo makePlottingInfo();
  void plotElectrophoregram(const EFGDisplayer &displayer, const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi,
                            const double EOFValue, CalculatorInterface::EOFValueType EOFvt, const double izLen, const double plotCutoff);
  QVariant resetSignalItems();
  void selectSignalIfAvailable(const QVariant &sig);
  void setControlsIcons();

  QPushButton *m_qpb_new;
  QPushButton *m_qpb_load;
  QPushButton *m_qpb_save;
  QPushButton *m_qpb_calculate;

  QShortcut *m_calculateShortcut;

  CalculatorInterface m_calcIface;
  MainControlWidget *m_mainCtrlWidget;
  SignalPlotWidget *m_signalPlotWidget;
  SystemCompositionWidget *h_scompWidget;
  CheckForUpdateDialog *m_checkForUpdateDlg;

  AnalytesExtraInfoModel * const h_analytesEXIModel;
  const QAbstractTableModel * const h_eigenzoneDetailsModel;

  QVector<double> m_plotParamsData;
  FloatMapperModel<SignalPlotWidget::PlotParamsItems> m_plotParamsModel;

  persistence::Persistence &m_persistence;

  QStandardItemModel *m_signalTypesModel;

  QString m_lastLoadPath;
  QString m_lastSavePath;

  DatabaseProxy &h_dbProxy;

  ToggleTracepointsDialog::TracingSetup m_tracingSetup;

  Ui::PMNGMainWindow *ui;

  static QVector<SignalItem> s_defaultSignalItems;

private slots:
  void onAbout();
  void onCalculate();
  void onCompositionChanged();
  void onDatabaseEditor();
  void onLoad();
  void onExit();
  void onExportElectrophoregramAsCSV();
  void onNew();
  void onOpenDatabase();
  void onOpenUpdateDialog();
  void onPlotElectrophoregram();
  void onRunSetupChanged(const bool invalidate);
  void onSave();
  void onSetDebuggingOutput();

signals:
  void clearAll();
};

#endif // PMNGMAINWINDOW_H
