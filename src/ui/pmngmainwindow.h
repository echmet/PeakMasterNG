#ifndef PMNGMAINWINDOW_H
#define PMNGMAINWINDOW_H

#include "../gearbox/calculatorinterface.h"
#include "../mappers/floatmappermodel.h"
#include "../gearbox/floatingvaluedelegate.h"
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
class DatabaseProxy;
class EFGDisplayer;
class MainControlWidget;
class ResultsModels;
class QDataWidgetMapper;
class QPushButton;
class QShortcut;
class QStandardItemModel;
class SignalPlotWidget;
class SystemCompositionWidget;

class PMNGMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  enum class PlotParamsItems {
    CUTOFF,
    INJ_ZONE_LENGTH,
    LAST_INDEX
  };

  explicit PMNGMainWindow(SystemCompositionWidget *scompWidget,
                          CalculatorInterface &&calcIface, ResultsModels resultsModels,
                          persistence::Persistence &persistence,
                          DatabaseProxy &dbProxy,
                          AnalytesExtraInfoModel * const analytesEXIModel, const QAbstractTableModel * const eigenzoneDetailsModel,
                          QWidget *parent = nullptr);
  ~PMNGMainWindow();

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

  AnalytesExtraInfoModel * const h_analytesEXIModel;
  const QAbstractTableModel * const h_eigenzoneDetailsModel;

  QDataWidgetMapper *m_plotParamsMapper;
  FloatMapperModel<PlotParamsItems> m_plotParamsModel;
  QVector<double> m_plotParamsData;

  FloatingValueDelegate m_fltDelegate;

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
  void onAutoPlotCutoffStateChanged(const int state);
  void onCalculate();
  void onCompositionChanged();
  void onDatabaseEditor();
  void onLoad();
  void onExit();
  void onExportElectrophoregramAsCSV();
  void onNew();
  void onOpenDatabase();
  void onPlotElectrophoregram();
  void onRunSetupChanged(const bool invalidate);
  void onSave();
  void onSetDebuggingOutput();

signals:
  void clearAll();
};

#endif // PMNGMAINWINDOW_H
