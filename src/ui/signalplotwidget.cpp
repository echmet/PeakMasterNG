#include "signalplotwidget.h"
#include "ui_signalplotwidget.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "doubleclickableqwtplotzoomer.h"
#include "ploteventfilter.h"
#include "../gearbox/doubletostringconvertor.h"

#include <QDataWidgetMapper>
#include <QStandardItemModel>
#include <QToolTip>
#include <QVBoxLayout>

static
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

  m_plotParamsMapper = new QDataWidgetMapper{this};

  auto filter = new PlotEventFilter{this};
  m_plot->canvas()->installEventFilter(filter);
  connect(filter, &PlotEventFilter::mouseMoved, this, &SignalPlotWidget::onPointHovered);

  {
    QVBoxLayout *lay = qobject_cast<QVBoxLayout *>(this->layout());
    if (lay != nullptr)
      lay->insertWidget(1, m_plot);
  }
  m_plot->setAxisTitle(QwtPlot::Axis::xBottom, "time (min)");

  m_plot->setMinimumHeight(50);
  this->setMinimumHeight(50);

  connect(ui->qpb_replot, &QPushButton::clicked, this, &SignalPlotWidget::onTriggerReplotElectrophoregram);
  connect(ui->qcbox_signal, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &SignalPlotWidget::onTriggerReplotElectrophoregram);
  connect(ui->qcb_autoPlotCutoff, &QCheckBox::stateChanged, this, &SignalPlotWidget::onAutoPlotCutoffStateChanged);

  onAutoPlotCutoffStateChanged(Qt::Checked);

  connect(ui->qle_plotCutoff, &FloatingValueLineEdit::valueChanged, this, &SignalPlotWidget::onPlottingParametersChanged);
  connect(ui->qle_injZoneLength, &FloatingValueLineEdit::valueChanged, this, &SignalPlotWidget::onPlottingParametersChanged);
  connect(ui->qcb_autoPlotCutoff, &QCheckBox::stateChanged, this, &SignalPlotWidget::onPlottingParametersChanged);
}

SignalPlotWidget::~SignalPlotWidget()
{
  delete ui;
}

bool SignalPlotWidget::autoPlotCutoff() const noexcept
{
  return ui->qcb_autoPlotCutoff->isChecked();
}

void SignalPlotWidget::clear()
{
  m_plotCurve->setSamples(QVector<QPointF>{});
  m_plot->replot();
}

void SignalPlotWidget::onAutoPlotCutoffStateChanged(const int state)
{
  ui->qle_plotCutoff->setDisabled(state == Qt::Checked);
}


void SignalPlotWidget::onPlottingParametersChanged()
{
  m_plotParamsMapper->submit();
  emit replotElectrophoregram();
}

void SignalPlotWidget::onPointHovered(const QPoint &pos)
{
  if (m_plotCurve->dataSize() < 1)
    return;

  const double tx = m_plot->canvasMap(QwtPlot::Axis::xBottom).invTransform(pos.x());

  const size_t cpIdx = findClosestIdx(m_signal, tx);
  const QPointF &cp = m_signal.at(cpIdx);

  const double x = cp.x();
  QStringList zoneNames{};
  for (const auto &z : m_tdzi) {
    if (x >= z.beginsAt && x <= z.endsAt)
      zoneNames << z.name;
  }

  if (zoneNames.isEmpty()) {
    QToolTip::hideText();
  } else {
    const auto zoneText = [&zoneNames]() -> std::pair<QString, QString> {
      QString resToolTip;
      QString resTray;

      for (int idx = 0; idx < zoneNames.size() - 1; idx++) {
        resToolTip += zoneNames.at(idx) + QLatin1String("\n");
        resTray += zoneNames.at(idx) + QLatin1String(" ");
      }
      resToolTip += zoneNames.last();
      resTray += zoneNames.last();

      return { resToolTip, resTray };
    }();

    QToolTip::showText(m_plot->mapToGlobal(pos), std::get<0>(zoneText), m_plot);
  }
}

void SignalPlotWidget::onTriggerReplotElectrophoregram()
{
  emit replotElectrophoregram();
}

int SignalPlotWidget::selectedSignalIndex() const noexcept
{
  return ui->qcbox_signal->currentIndex();
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

void SignalPlotWidget::setPlotParamsMapper(FloatMapperModel<PlotParamsItems> *model)
{
  m_plotParamsMapper->setModel(model);
  m_plotParamsMapper->setItemDelegate(&m_fltDelegate);
  m_plotParamsMapper->addMapping(ui->qle_plotCutoff, model->indexFromItem(PlotParamsItems::CUTOFF));
  m_plotParamsMapper->addMapping(ui->qle_injZoneLength, model->indexFromItem(PlotParamsItems::INJ_ZONE_LENGTH));
  m_plotParamsMapper->toFirst();
}

void SignalPlotWidget::setSignal(const QVector<QPointF> &signal, const SignalStyle style, const QString &yAxisText,
                                 const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi)
{
  m_plotCurve->setSamples(signal);
  setBrush(style);
  m_signal = signal.toStdVector();

  m_tdzi = tdzi;

  m_plot->setAxisTitle(QwtPlot::Axis::yLeft, yAxisText);

  const QRectF brect = m_plotCurve->boundingRect();
  m_plot->replot();
  m_plotZoomer->zoom(brect);
  m_plotZoomer->setZoomBase(brect);
}

void SignalPlotWidget::setSignalIndex(const int idx)
{
  ui->qcbox_signal->setCurrentIndex(idx);
}

void SignalPlotWidget::setSignalItemsModel(QStandardItemModel *model)
{
  ui->qcbox_signal->setModel(model);
}
