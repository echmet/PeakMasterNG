#include "signalplotwidget.h"
#include "ui_signalplotwidget.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "doubleclickableqwtplotzoomer.h"
#include "ploteventfilter.h"
#include "../gearbox/doubletostringconvertor.h"

#include <QVBoxLayout>

size_t findClosestIdx(const std::vector<QPointF> &data, const double currentX)
{
  const double firstX = data.front().x();
  const double lastX = data.back().x();

  if (currentX < firstX)
    return 0;
  if (currentX > lastX)
    return data.size() - 1;

  const double span = lastX - firstX;
  const double region = (currentX - firstX) / span;
  const size_t regionIdx = static_cast<size_t>(std::floor(region * data.size()));
  if (regionIdx >= data.size()) /* Safety net */
    return static_cast<size_t>(data.size()) - 1;

  for (size_t idx = regionIdx; idx < data.size(); idx++) {
    if (data.at(idx).x() <= currentX)
      return idx;
  }

  return regionIdx;
}

SignalPlotWidget::SignalPlotWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SignalPlotWidget)
{
  ui->setupUi(this);

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

  auto filter = new PlotEventFilter{this};
  m_plot->canvas()->installEventFilter(filter);
  connect(filter, &PlotEventFilter::mouseMoved, this, &SignalPlotWidget::onPointHovered);

  {
    QVBoxLayout *lay = qobject_cast<QVBoxLayout *>(this->layout());
    if (lay != nullptr)
      lay->insertWidget(0, m_plot);
  }
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

void SignalPlotWidget::onPointHovered(const QPoint &pos)
{
  if (m_plotCurve->dataSize() < 1)
    return;

  const double tx = m_plot->canvasMap(QwtPlot::Axis::xBottom).invTransform(pos.x());

  const size_t cpIdx = findClosestIdx(m_signal, tx);
  const QPointF &cp = m_signal.at(cpIdx);

  ui->ql_xVal->setText(DoubleToStringConvertor::convert(cp.x()));
  ui->ql_yVal->setText(DoubleToStringConvertor::convert(cp.y()));

  const double x = cp.x();
  QString zoneName{};
  for (const auto &z : m_zinfo) {
    if (x >= z.beginsAt && x <= z.endsAt)
      zoneName += z.name + " ";
  }
  ui->ql_zoneVal->setText(zoneName);
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

void SignalPlotWidget::setSignal(const QVector<QPointF> &signal, const SignalStyle style, const QString &yAxisText,
                                 std::vector<CalculatorInterface::SpatialZoneInformation> &&zinfo)
{
  m_plotCurve->setSamples(signal);
  setBrush(style);
  m_signal = signal.toStdVector();

  m_zinfo = std::move(zinfo);

  m_plot->setAxisTitle(QwtPlot::Axis::yLeft, yAxisText);

  const QRectF brect = m_plotCurve->boundingRect();
  m_plot->replot();
  m_plotZoomer->zoom(brect);
  m_plotZoomer->setZoomBase(brect);
}
