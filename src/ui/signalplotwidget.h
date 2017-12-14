#ifndef SIGNALPLOTWIDGET_H
#define SIGNALPLOTWIDGET_H

class QwtPlot;
class QwtPlotCurve;
class QwtPlotPicker;
class QwtPlotZoomer;

#include "../gearbox/calculatorinterface.h"


#include <QPointF>
#include <QVector>
#include <QWidget>

namespace Ui {
  class SignalPlotWidget;
}

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

  explicit SignalPlotWidget(QWidget *parent = nullptr);
  ~SignalPlotWidget();
  void clear();
  void setSignal(const QVector<QPointF> &signal, const SignalStyle style, const QString &yAxisText,
                 std::vector<CalculatorInterface::SpatialZoneInformation> &&zinfo);

private:
  void setBrush(const SignalStyle style);

  Ui::SignalPlotWidget *ui;

  QwtPlot *m_plot;
  QwtPlotCurve *m_plotCurve;
  QwtPlotPicker *m_plotPicker;
  QwtPlotZoomer *m_plotZoomer;
  std::vector<QPointF> m_signal;

  std::vector<CalculatorInterface::SpatialZoneInformation> m_zinfo;

private slots:
  void onPointHovered(const QPoint &pos);

};

#endif // SIGNALPLOTWIDGET_H
