#include "signalplotwidget.h"
#include "ui_signalplotwidget.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "doubleclickableqwtplotzoomer.h"

#include <QVBoxLayout>

SignalPlotWidget::SignalPlotWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SignalPlotWidget)
{
  ui->setupUi(this);
  this->setLayout(new QVBoxLayout{});

  m_plot = new QwtPlot{this};
  m_plot->setCanvasBackground(QBrush(Qt::white));
  m_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  m_plotCurve = new QwtPlotCurve{};
  m_plotCurve->attach(m_plot);

  m_plotZoomer = new DoubleClickableQwtPlotZoomer(m_plot->canvas());
  m_plotZoomer->setTrackerMode(QwtPicker::AlwaysOn);
  m_plotZoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::LeftButton, Qt::ShiftModifier);

  m_plotPicker = new QwtPlotPicker(QwtPlot::Axis::xBottom, QwtPlot::Axis::yLeft,
                                   QwtPicker::NoRubberBand, QwtPicker::AlwaysOff,
                                   m_plot->canvas());
  m_plotPicker->setStateMachine(new QwtPickerClickPointMachine{}); /* Come blow my ClickPoint machine! */
  m_plotPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);

  this->layout()->addWidget(m_plot);
  m_plot->setAxisTitle(QwtPlot::Axis::xBottom, "time (min)");

  m_plot->setMinimumHeight(50);
  this->setMinimumHeight(50);
}

SignalPlotWidget::~SignalPlotWidget()
{
  delete ui;
}

void SignalPlotWidget::clear()
{
  m_plotCurve->setSamples(QVector<QPointF>{});
  m_plot->replot();
}

void SignalPlotWidget::setBrush(const SignalStyle style)
{
  const qreal pw = m_plotCurve->pen().widthF();

  switch (style) {
  case SignalStyle::CONDUCTIVITY:
    m_plotCurve->setPen(QPen{QBrush{Qt::blue, Qt::SolidPattern}, pw});
    break;
  case SignalStyle::PH_RESPONSE:
    m_plotCurve->setPen(QPen{QBrush(QColor{171, 109, 21}, Qt::SolidPattern), pw});
    break;
  case SignalStyle::ALL_ANALYTES:
    m_plotCurve->setPen(QPen{QBrush(QColor{0, 135, 38}, Qt::SolidPattern), pw});
    break;
  case SignalStyle::CONCENTRATION:
    m_plotCurve->setPen(QPen{QBrush(Qt::red, Qt::SolidPattern), pw});
    break;
  }
}
void SignalPlotWidget::setSignal(const QVector<QPointF> &signal, const SignalStyle style, const QString &yAxisText)
{
  m_plotCurve->setSamples(signal);
  setBrush(style);

  m_plot->setAxisTitle(QwtPlot::Axis::yLeft, yAxisText);

  const QRectF brect = m_plotCurve->boundingRect();
  m_plot->replot();
  m_plotZoomer->zoom(brect);
  m_plotZoomer->setZoomBase(brect);
}
