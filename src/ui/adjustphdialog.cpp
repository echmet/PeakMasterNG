// vim: sw=2 ts=2 sts=2 expandtab

#include "adjustphdialog.h"
#include "ui_adjustphdialog.h"

#include "internal_models/adjustphtablemodel.h"
#include "../gearbox/backgroundgdmproxy.h"
#include "../gearbox/doubletostringconvertor.h"
#include "../gearbox/phadjusterinterface.h"

#include <QMessageBox>

AdjustpHDialog::AdjustpHDialog(GDMProxy &GDMProxy, const bool debyeHuckel, const bool onsagerFuoss,
                               const double currentpH, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::AdjustpHDialog},
  h_GDMProxy{dynamic_cast<BackgroundGDMProxy&>(GDMProxy)},
  m_debyeHuckel{debyeHuckel},
  m_onsagerFuoss{onsagerFuoss}
{
  ui->setupUi(this);

  m_model = new AdjustpHTableModel{h_GDMProxy.allBackgroundNames(), h_GDMProxy, this};
  ui->qtbv_bgeConstituents->setModel(m_model);

  auto hdr = ui->qtbv_bgeConstituents->horizontalHeader();
  for (int col = 0; col < m_model->columnCount(); col++) {
    hdr->setSectionResizeMode(col, QHeaderView::ResizeToContents);
    ui->qtbv_bgeConstituents->setColumnWidth(col, hdr->sizeHintForColumn(col));
  }

  ui->qtbv_bgeConstituents->setSelectionMode(QAbstractItemView::SingleSelection);

  ui->qle_currentpH->setText(DoubleToStringConvertor::convert(currentpH));

  connect(ui->qpb_adjust, &QPushButton::clicked, this, [this]() {
    bool ok;
    auto pH = getTargetpH(&ok);
    auto ctuent = selectedConstituent();
    if (ok)
      adjustConcentration(ctuent, pH);
  });
  connect(ui->qtbv_bgeConstituents, &QTableView::doubleClicked, this, [this](const QModelIndex &idx) {
    bool ok;
    auto pH = getTargetpH(&ok);
    auto ctuent = idx.model()->data(idx.model()->index(idx.row(), 0));
    if (ctuent.isValid() && ok)
      adjustConcentration(ctuent.toString(), pH);
  });
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
    pHAdjusterInterface iface{ctuent.toStdString(), h_GDMProxy, m_debyeHuckel, m_onsagerFuoss};
    const double adjustedpH = iface.adjustpH(targetpH);
    m_model->updateConcentration(ctuent);

    ui->qle_currentpH->setText(DoubleToStringConvertor::convert(adjustedpH));
  } catch (const pHAdjusterInterface::Exception &ex) {
    const auto msg = ex.what() + QString{"\n\nNote that it may not be possible to achieve the target pH "
                                         "by adjusting the concentration of %1."}.arg(ctuent);
    QMessageBox mbox{QMessageBox::Warning, tr("Failed to adjust pH"), msg};

    mbox.exec();
  }
}

double AdjustpHDialog::getTargetpH(bool *ok)
{
  auto v = ui->qle_targetpH->text();
  const double pH = DoubleToStringConvertor::back(v, ok);
  if (!ok) {
    QMessageBox mbox{QMessageBox::Warning, tr("Invalid input"), tr("Invalid pH value")};

    mbox.exec();
  }

  return pH;
}

QString AdjustpHDialog::selectedConstituent()
{
  const auto &indexes = ui->qtbv_bgeConstituents->selectionModel()->selectedIndexes();
  if (indexes.empty())
    return {};

  const auto &idx = indexes.constFirst();

  return m_model->data(m_model->index(idx.row(), 0)).toString();
}
