#include "ioniccompositiondialog.h"
#include "ui_ioniccompositiondialog.h"
#include "../gearbox/results_models/analytesdissociationmodel.h"
#include "../gearbox/doubletostringconvertor.h"
#include "elementaries/uihelpers.h"

#include <QAbstractTableModel>
#include <QScreen>
#include <QStandardItemModel>

const QString IonicCompositionDialog::FILL_TO_STRING{"9.999999999_"};

static
int isContained(const QStandardItemModel *model, const QString &s)
{
  for (int idx = 0; idx < model->rowCount(); idx++) {
    const auto &item = model->data(model->index(idx, 0), Qt::UserRole + 1);
    if (item == s)
      return idx;
  }

  return -1;
}

IonicCompositionDialog::IonicCompositionDialog(QAbstractTableModel *bgeModel, AnalytesDissociationModel *analytesModel, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::IonicCompositionDialog},
  m_analytesModel{analytesModel}
{
  ui->setupUi(this);
  m_analytesNamesModel = new QStandardItemModel{};

  ui->qtbv_bgeIonicComposition->setModel(bgeModel);
  ui->qtbv_bgeIonicComposition->resizeColumnsToContents();

  ui->qtbv_analyteDissociation->setModel(m_analytesModel);
  ui->qtbv_analyteDissociation->resizeColumnsToContents();

  ui->qcbox_analyte->setModel(m_analytesNamesModel);

  ui->ql_miscalculation->setVisible(false);

  connect(m_analytesModel, &AnalytesDissociationModel::analytesChanged, this, &IonicCompositionDialog::onAnalytesDissociationDataUpdated);
  connect(ui->qcbox_analyte, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &IonicCompositionDialog::onAnalyteSelectionChanged);
}

IonicCompositionDialog::~IonicCompositionDialog()
{
  delete ui;
  delete m_analytesNamesModel;
}

void IonicCompositionDialog::onAnalyteSelectionChanged(int idx)
{
  if (idx < 0)
    return;

  const QString s = m_analytesNamesModel->data(m_analytesNamesModel->index(idx, 0), Qt::UserRole + 1).toString();
  m_analytesModel->selectAnalyte(s.toStdString());

  ui->ql_miscalculation->setVisible(m_analytesModel->isMiscalculated()); /* This must be called after selectAnalyte() */
  ui->qtbv_analyteDissociation->resizeColumnsToContents();
}

void IonicCompositionDialog::onAnalytesDissociationDataUpdated()
{
  ui->ql_miscalculation->setVisible(false);

  const auto &analytes = m_analytesModel->analytes();
  if (m_analytesNamesModel->rowCount() > 0)
    m_lastSelectedAnalyte = ui->qcbox_analyte->currentData(Qt::UserRole + 1).toString();
  m_analytesNamesModel->clear();

  for (const auto &a : analytes) {
    QStandardItem *item = new QStandardItem{QString::fromStdString(a)};
    item->setData(QString::fromStdString(a));

    m_analytesNamesModel->appendRow(item);
  }

  if (!analytes.empty()) {
    const int idx = isContained(m_analytesNamesModel, m_lastSelectedAnalyte);
    if (idx >= 0) {
      ui->qcbox_analyte->setCurrentIndex(idx);
      m_analytesModel->selectAnalyte(m_lastSelectedAnalyte.toStdString());
    } else {
      ui->qcbox_analyte->setCurrentIndex(0);
      m_analytesModel->selectAnalyte(analytes.front());
    }
    ui->ql_miscalculation->setVisible(m_analytesModel->isMiscalculated()); /* This must be called after selectAnalyte() */
    ui->qtbv_analyteDissociation->resizeColumnsToContents();
  }
}

QSize IonicCompositionDialog::sizeHint() const
{
   /* Arbitrarily chosen values */
  const int WIDTH_SPACER = 125;
  const int HEIGHT_SPACER = 75;
  const QScreen *scr = UIHelpers::findScreenForWidget(this);

  auto fm = ui->qtbv_bgeIonicComposition->fontMetrics();
  const auto model = ui->qtbv_bgeIonicComposition->model();

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  int width = qRound(fm.width(FILL_TO_STRING) * model->columnCount() * 1.2);
#else
  int width = qRound(fm.horizontalAdvance(FILL_TO_STRING) * model->columnCount() * 1.2);
#endif // QT_VERSION

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

  int height = fm.height() * (model->rowCount() + 1) * 2 + HEIGHT_SPACER;

  if (scr != nullptr) {
    if (width > scr->availableSize().width())
      width = scr->availableSize().width();
    if (height > scr->availableSize().height())
      height = scr->availableSize().height();
  }

  if (ui->ql_miscalculation->isVisibleTo(this))
    height += 2 * ui->ql_miscalculation->sizeHint().height();

  return QSize{width, height};
}
