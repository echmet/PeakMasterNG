#ifndef PMNGMAINWINDOW_H
#define PMNGMAINWINDOW_H

#include "../gearbox/calculatorinterface.h"
#include "../gearbox/results_models/resultsmodels.h"
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

class CalculatorInterface;
class CheckForUpdateDialog;
class DatabaseProxy;
class EFGDisplayer;
class GDMProxy;
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
                          GDMProxy &GDMProxy,
                          QWidget *parent = nullptr);
  ~PMNGMainWindow() override;
  void connectUpdater(SoftwareUpdater *updater);

protected:
  void closeEvent(QCloseEvent *evt) override;

private:
  class ActiveFile {
  public:
    explicit ActiveFile() :
      valid{false},
      m_path{""}
    {}

    ActiveFile(QString path) :
      valid{true},
      m_path(std::move(path))
    {}

    ActiveFile & operator=(const ActiveFile &other)
    {
      const_cast<bool&>(valid) = other.valid;
      const_cast<QString&>(m_path) = other.m_path;

      return  *this;
    }

    ActiveFile & operator=(ActiveFile &&other) noexcept
    {
      const_cast<bool&>(valid) = other.valid;
      const_cast<QString&>(m_path) = std::move(other.m_path);

      return  *this;
    }

    const QString & path() const noexcept
    {
      return m_path;
    }

    const bool valid;

  private:
    QString m_path;
  };

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

  void addConstituentsSignals(const CalculatorInterface::ConstituentPackVec &constituents);
  EFGDisplayer makeMainWindowEFGDisplayer();
  void initPlotParams();
  void initSignalItems();
  void loadSystem(const QString &path, const bool fromClipboard);
  PlottingInfo makePlottingInfo();
  void plotElectrophoregram(const EFGDisplayer &displayer, const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi,
                            const double EOFValue, CalculatorInterface::EOFValueType EOFvt, const double izLen, const double plotCutoff);
  QVariant resetSignalItems();
  void saveSystem(const QString &m_path, const bool toClipboard);
  void selectSignalIfAvailable(const QVariant &sig);
  void setControlsIcons();
  void setWindowTitle(const QString &file = {});

  QPushButton *m_qpb_new;
  QPushButton *m_qpb_load;
  QPushButton *m_qpb_save;
  QPushButton *m_qpb_saveAs;
  QPushButton *m_qpb_calculate;

  QShortcut *m_calculateShortcut;
  QShortcut *m_calculateShortcutTwo;

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
  const ResultsModels m_resultsModels;

  ToggleTracepointsDialog::TracingSetup m_tracingSetup;

  bool m_fullCalcInProgress;

  ActiveFile m_activeFile;
  bool m_unsaved;

  Ui::PMNGMainWindow *ui;

  static QVector<SignalItem> s_defaultSignalItems;

private slots:
  void connectOnScreenChanged();
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
  void onpHAdjusted();
  void onReplotElectrophoregram();
  void onRunSetupChanged(const bool invalidate);
  void onSave();
  void onSaveAs();
  void onScreenChanged(QScreen *screen);
  void onSetDebuggingOutput();

signals:
  void clearAll();
};

#endif // PMNGMAINWINDOW_H
