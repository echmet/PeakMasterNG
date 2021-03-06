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

  connect(ui->qpb_adjust, &QPushButton::clicked, this, &AdjustpHDialog::onAdjustClicked);
}

AdjustpHDialog::~AdjustpHDialog()
{
  delete ui;
}

void AdjustpHDialog::onAdjustClicked()
{
  const auto &v = ui->qle_targetpH->text();

  bool ok;
  const double pH = DoubleToStringConvertor::back(v, &ok);
  if (!ok) {
    QMessageBox mbox{QMessageBox::Warning, tr("Invalid input"), tr("Invalid pH value")};

    mbox.exec();
    return;
  }

  auto ctuent = selectedConstituent();
  if (ctuent.isEmpty()) {
    QMessageBox mbox{QMessageBox::Warning, tr("Invalid input"), tr("No constituent selected")};

    mbox.exec();
    return;
  }

  pHAdjusterInterface iface{ctuent.toStdString(), h_GDMProxy, m_debyeHuckel, m_onsagerFuoss};

  try {
    const double adjustedpH = iface.adjustpH(pH);
    m_model->updateConcentration(ctuent);

    ui->qle_currentpH->setText(DoubleToStringConvertor::convert(adjustedpH));
  } catch (const pHAdjusterInterface::Exception &ex) {
    const auto msg = ex.what() + QString{"\n\nNote that it may not be possible to achieve the target pH "
                                         "by adjusting the concentration of %1."}.arg(ctuent);
    QMessageBox mbox{QMessageBox::Warning, tr("Failed to adjust pH"), msg};

    mbox.exec();
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
