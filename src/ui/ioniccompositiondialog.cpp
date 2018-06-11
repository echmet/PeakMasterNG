#include "ioniccompositiondialog.h"
#include "ui_ioniccompositiondialog.h"
#include "../gearbox/results_models/analytesdissociationmodel.h"
#include "../gearbox/doubletostringconvertor.h"

#include <QAbstractTableModel>
#include <QStandardItemModel>

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
  ui->qle_effectiveMobility->setText(DoubleToStringConvertor::convert(m_analytesModel->effectiveMobility()));
}

void IonicCompositionDialog::onAnalytesDissociationDataUpdated()
{
  const auto &analytes = m_analytesModel->analytes();
  if (m_analytesNamesModel->rowCount() > 0)
    m_lastSelectedAnalyte = ui->qcbox_analyte->currentData(Qt::UserRole + 1).toString();
  m_analytesNamesModel->clear();

  for (const auto &a : analytes) {
    QStandardItem *item = new QStandardItem{QString::fromStdString(a)};
    item->setData(QString::fromStdString(a));

    m_analytesNamesModel->appendRow(item);
  }

  if (analytes.size() > 0) {
    const int idx = isContained(m_analytesNamesModel, m_lastSelectedAnalyte);
    if (idx >= 0) {
      ui->qcbox_analyte->setCurrentIndex(idx);
      m_analytesModel->selectAnalyte(m_lastSelectedAnalyte.toStdString());
    } else {
      ui->qcbox_analyte->setCurrentIndex(0);
      m_analytesModel->selectAnalyte(analytes.front());
    }
    ui->qle_effectiveMobility->setText(DoubleToStringConvertor::convert(m_analytesModel->effectiveMobility()));
  } else
    ui->qle_effectiveMobility->setText("");
}
