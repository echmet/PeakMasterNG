#include "eigenzonedetailsdialog.h"
#include "ui_eigenzonedetailsdialog.h"
#include "elementaries/uihelpers.h"

#include "../globals.h"

#include <cmath>
#include <QAbstractTableModel>
#include <QFontMetrics>
#include <QScreen>
#include <QWindow>

EigenzoneDetailsDialog::EigenzoneDetailsDialog(QAbstractTableModel *model, const bool displayDeltas, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EigenzoneDetailsDialog}
{
  ui->setupUi(this);

  ui->qtbv_ezProps->setModel(model);
  ui->qtbv_ezProps->resizeColumnsToContents();
  ui->qcbox_dispalyDeltas->setChecked(displayDeltas);

  connect(ui->qcbox_dispalyDeltas, &QCheckBox::stateChanged, this, &EigenzoneDetailsDialog::onDisplayDeltasChanged);
}

EigenzoneDetailsDialog::~EigenzoneDetailsDialog()
{
  delete ui;
}

void EigenzoneDetailsDialog::onDisplayDeltasChanged(const int state)
{
  emit displayDeltasChanged(state == Qt::Checked);
}

QSize EigenzoneDetailsDialog::sizeHint() const
{
  static const auto iround = [](const auto v) {
    return static_cast<int>(std::floor(v + 0.5));
  };

  const auto theme = UIHelpers::detectWindowsTheme();
  /* Arbitrarily chosen values */

  static const qreal COL_HEIGHT_AMPL = [theme](){
    switch (theme) {
    case UIHelpers::WindowsTheme::WINDOWS_XP:
      return 2.36;
    case UIHelpers::WindowsTheme::WINDOWS_CLASSIC:
      return 2.43;
    default:
      return 2.0;
    }
  }();
  const int WIDTH_SPACER = 125;
  const int HEIGHT_SPACER = 75;
  const QScreen *scr = UIHelpers::findScreenForWidget(this);

  auto fm = ui->qtbv_ezProps->fontMetrics();
  const auto model = ui->qtbv_ezProps->model();

  int width = 0;
  for (int idx = 0; idx < model->columnCount(); idx++) {
    const QString text = model->headerData(idx, Qt::Horizontal).toString();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    int advance = fm.width(text);
#else
    int advance = fm.horizontalAdvance(text);
#endif // QT_VERSION
    width += std::floor(advance * (theme != UIHelpers::WindowsTheme::NOT_WINDOWS ? 2.7 : 2.3) + 0.5);
  }

  QString longestRow;
  for (int idx = 0; idx < model->rowCount(); idx++) {
    const QString text = model->headerData(idx, Qt::Vertical).toString();
    if (text.length() > longestRow.length())
      longestRow = text;
  }
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  width += fm.width(longestRow) + WIDTH_SPACER;
#else
  width += fm.horizontalAdvance(longestRow) + WIDTH_SPACER;
#endif // QT_VERSION

  int height = iround(fm.height() * (model->rowCount() + 1) * COL_HEIGHT_AMPL + HEIGHT_SPACER);

  if (scr != nullptr) {
    if (width > scr->availableSize().width())
      width = scr->availableSize().width();
    if (height > scr->availableSize().height())
      height = scr->availableSize().height();
  }

  return QSize{width, height};
}
