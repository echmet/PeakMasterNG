// vim: sw=2 ts=2 sts=2 expandtab

#include "adjustphdialog.h"
#include "ui_adjustphdialog.h"

#include "internal_models/adjustphtablemodel.h"
#include "../gearbox/backgroundgdmproxy.h"
#include "../gearbox/doubletostringconvertor.h"
#include "../gearbox/floatingvaluedelegate.h"
#include "../gearbox/phadjusterinterface.h"

#include <QAbstractItemModel>
#include <QMessageBox>
#include <QPushButton>

AdjustpHDialog::AdjustpHDialog(GDMProxy &GDMProxy, const bool debyeHuckel, const bool onsagerFuoss,
                               const double currentpH, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::AdjustpHDialog},
  h_GDMProxy{dynamic_cast<BackgroundGDMProxy&>(GDMProxy)},
  m_debyeHuckel{debyeHuckel},
  m_onsagerFuoss{onsagerFuoss}
{
  ui->setupUi(this);

  for (const auto &name : h_GDMProxy.allBackgroundNames())
    m_originalConcentrations[name] = h_GDMProxy.concentrations(name).at(0);

  m_model = new AdjustpHTableModel{h_GDMProxy.allBackgroundNames(), h_GDMProxy, this};
  ui->qtbv_bgeConstituents->setModel(m_model);

  auto fltDelegate = new FloatingValueDelegate{false, this};
  ui->qtbv_bgeConstituents->setItemDelegateForColumn(1, fltDelegate);

  auto hdr = ui->qtbv_bgeConstituents->horizontalHeader();
  for (int col = 0; col < m_model->columnCount(); col++) {
    hdr->setSectionResizeMode(col, QHeaderView::ResizeToContents);
    ui->qtbv_bgeConstituents->setColumnWidth(col, hdr->sizeHintForColumn(col));
  }

  ui->qtbv_bgeConstituents->setSelectionMode(QAbstractItemView::SingleSelection);

  ui->qle_currentpH->setText(DoubleToStringConvertor::convert(currentpH));

  auto resetButton = ui->buttonBox->button(QDialogButtonBox::Reset);
  connect(resetButton, &QPushButton::clicked, this, [this]() {
    restoreConcentrations();
    for (const auto &it : m_originalConcentrations) {
      m_model->updateConcentration(QString::fromStdString(it.first));
    }
  });
  connect(ui->qtbv_bgeConstituents, &QTableView::doubleClicked, this, [this](const QModelIndex &idx) {
    bool ok;
    auto pH = getTargetpH(&ok);
    auto ctuent = idx.model()->data(idx.model()->index(idx.row(), 0));
    if (ctuent.isValid() && ok)
      adjustConcentration(ctuent.toString(), pH);
  });
  connect(m_model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
    if (!roles.contains(Qt::EditRole))
      return;

    if (topLeft.column() <= 1 && bottomRight.column() >= 1)
      calculatepH();
  });
  connect(ui->qpb_calcpH, &QPushButton::clicked, this, [this]() {
    calculatepH();
  });

  calculatepH();
}

AdjustpHDialog::~AdjustpHDialog()
{
  delete ui;
}

void AdjustpHDialog::adjustConcentration(const QString &ctuent, const double targetpH)
{
  if (ctuent.isEmpty()) {
    QMessageBox::warning(this, tr("Invalid input"), tr("No constituent selected"));
    return;
  }

  try {
    pHAdjusterInterface iface{h_GDMProxy, m_debyeHuckel, m_onsagerFuoss};
    const double adjustedpH = iface.adjustpH(ctuent.toStdString(), targetpH);
    m_model->updateConcentration(ctuent);

    ui->qle_currentpH->setText(DoubleToStringConvertor::convert(adjustedpH));
  } catch (const pHAdjusterInterface::Exception &ex) {
    const auto msg = ex.what() + QString{"\n\nNote that it may not be possible to achieve the target pH "
                                         "by adjusting the concentration of %1."}.arg(ctuent);
    QMessageBox mbox{QMessageBox::Warning, tr("Failed to adjust pH"), msg};

    mbox.exec();
  }
}

void AdjustpHDialog::calculatepH()
{
  try {
    pHAdjusterInterface iface{h_GDMProxy, m_debyeHuckel, m_onsagerFuoss};
    double pH = iface.calculatepH();

    ui->qle_currentpH->setText(DoubleToStringConvertor::convert(pH));
  } catch (const pHAdjusterInterface::Exception &ex) {
    QMessageBox::warning(
      this,
      tr("Failed to calculate pH"),
      QString{tr("Error returned: %1")}.arg(ex.what())
    );
  }
}

double AdjustpHDialog::getTargetpH(bool *ok)
{
  auto v = ui->qle_targetpH->text();
  const double pH = DoubleToStringConvertor::back(v, ok);
  if (!ok)
    QMessageBox::warning(this, tr("Invalid input"), tr("Invalid pH value"));

  return pH;
}

void AdjustpHDialog::reject()
{
  restoreConcentrations();
  QDialog::reject();
}

void AdjustpHDialog::restoreConcentrations()
{
  for (const auto &it : m_originalConcentrations) {
    const double cSample = h_GDMProxy.concentrations(it.first).at(1);
    h_GDMProxy.setConcentrations(it.first, { it.second, cSample });
  }
}

QString AdjustpHDialog::selectedConstituent()
{
  const auto &indexes = ui->qtbv_bgeConstituents->selectionModel()->selectedIndexes();
  if (indexes.empty())
    return {};

  const auto &idx = indexes.constFirst();

  return m_model->data(m_model->index(idx.row(), 0)).toString();
}
