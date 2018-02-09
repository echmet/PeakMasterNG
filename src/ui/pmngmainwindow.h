#ifndef PMNGMAINWINDOW_H
#define PMNGMAINWINDOW_H

#include "../gearbox/calculatorinterface.h"
#include "../mappers/floatmappermodel.h"

#include <QMainWindow>

namespace Ui {
  class PMNGMainWindow;
}

namespace persistence {
  class Persistence;
}

class CalculatorInterface;
class EFGDisplayer;
class MainControlWidget;
class ResultsModels;
class QDataWidgetMapper;
class QPushButton;
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
                          QWidget *parent = nullptr);
  ~PMNGMainWindow();

private:
  class SignalItem {
  public:
    QString name;
    CalculatorInterface::Signal signal;
  };

  void addConstituentsSignals(const QVector<QString> &analytes);
  EFGDisplayer makeMainWindowEFGDisplayer();
  void initPlotParams();
  void initSignalItems();
  void plotElectrophoregram(const EFGDisplayer &displayer);
  void resetSignalItems();
  void setControlsIcons();

  QPushButton *m_qpb_new;
  QPushButton *m_qpb_load;
  QPushButton *m_qpb_save;
  QPushButton *m_qpb_calculate;

  CalculatorInterface m_calcIface;
  MainControlWidget *m_mainCtrlWidget;
  SignalPlotWidget *m_signalPlotWidget;
  SystemCompositionWidget *h_scompWidget;

  QDataWidgetMapper *m_plotParamsMapper;
  FloatMapperModel<PlotParamsItems> m_plotParamsModel;
  QVector<double> m_plotParamsData;

  persistence::Persistence &m_persistence;

  QStandardItemModel *m_signalTypesModel;

  QString m_lastLoadPath;
  QString m_lastSavePath;

  Ui::PMNGMainWindow *ui;

  static QVector<SignalItem> s_defaultSignalItems;

private slots:
  void onAbout();
  void onAutoPlotCutoffStateChanged(const int state);
  void onCalculate();
  void onCompositionChanged();
  void onLoad();
  void onNew();
  void onExit();
  void onExportElectrophoregramAsCSV();
  void onPlotElectrophoregram();
  void onRunSetupChanged(const bool invalidate);
  void onSave();

signals:
  void clearAll();
};

#endif // PMNGMAINWINDOW_H
