#ifndef SIGNALPLOTWIDGET_H
#define SIGNALPLOTWIDGET_H

class QwtPlot;
class QwtPlotCurve;
class QwtPlotPicker;
class QwtPlotZoomer;

#include "../gearbox/calculatorinterface.h"
#include "../gearbox/floatingvaluedelegate.h"

#include <QPointF>
#include <QVector>
#include <QWidget>

namespace Ui {
  class SignalPlotWidget;
}

class QDataWidgetMapper;
class QStandardItemModel;

class SignalPlotWidget : public QWidget
{
  Q_OBJECT

public:
  enum class SignalStyle {
    CONDUCTIVITY,
    PH_RESPONSE,
    ALL_ANALYTES,
    CONCENTRATION
  };
  enum class PlotParamsItems {
    CUTOFF,
    INJ_ZONE_LENGTH,
    LAST_INDEX
  };

  explicit SignalPlotWidget(QWidget *parent = nullptr);
  ~SignalPlotWidget();
  bool autoPlotCutoff() const noexcept;
  void clear();
  int selectedSignalIndex() const noexcept;
  void setPlotParamsMapper(FloatMapperModel<PlotParamsItems> *model);
  void setSignal(const QVector<QPointF> &signal, const SignalStyle style, const QString &yAxisText,
                 const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi);
  void setSignalIndex(const int idx);
  void setSignalItemsModel(QStandardItemModel *model);

private:
  void setBrush(const SignalStyle style);

  Ui::SignalPlotWidget *ui;

  QwtPlot *m_plot;
  QwtPlotCurve *m_plotCurve;
  QwtPlotPicker *m_plotPicker;
  QwtPlotZoomer *m_plotZoomer;
  std::vector<QPointF> m_signal;

  std::vector<CalculatorInterface::TimeDependentZoneInformation> m_tdzi;

  FloatingValueDelegate m_fltDelegate;
  QDataWidgetMapper *m_plotParamsMapper;

private slots:
  void onAutoPlotCutoffStateChanged(const int state);
  void onPlottingParametersChanged();
  void onPointHovered(const QPoint &pos);
  void onTriggerReplotElectrophoregram();

signals:
  void replotElectrophoregram();
};

#endif // SIGNALPLOTWIDGET_H
