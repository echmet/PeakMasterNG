// vim: sw=2 ts=2 sts=2 expandtab

#include "adjustueffoverkbgedialog.h"
#include "ui_adjustueffoverkbgedialog.h"

#include "internal_models/adjustueffoverkbgetablemodel.h"
#include "../gearbox/backgroundgdmproxy.h"
#include "../gearbox/doubletostringconvertor.h"
#include "../gearbox/ueffoverkbgecalculatorinterface.h"
#include "../gearbox/samplegdmproxy.h"

#include <QMessageBox>

#include <string>
#include <vector>

AdjustuEffOverkBGEDialog::AdjustuEffOverkBGEDialog(GDMProxy &backgroundGDMProxy, GDMProxy &sampleGDMProxy,
                                                   const bool debyeHuckel, const bool onsagerFuoss,
                                                   QWidget *parent) :
  QDialog{parent},
  ui{new Ui::AdjustuEffOverkBGEDialog},
  h_backgroundGDMProxy{static_cast<BackgroundGDMProxy&>(backgroundGDMProxy)},
  h_sampleGDMProxy{static_cast<SampleGDMProxy&>(sampleGDMProxy)},
  m_debyeHuckel{debyeHuckel},
  m_onsagerFuoss{onsagerFuoss}
{
  ui->setupUi(this);

  for (const auto &name : h_backgroundGDMProxy.allBackgroundNames())
    m_originalConcentrations[name] = h_backgroundGDMProxy.concentrations(name).at(0);

  m_model = new AdjustuEffOverkBGETableModel{this};

  uEffOverkBGECalculatorInterface iface{h_backgroundGDMProxy, m_debyeHuckel, m_onsagerFuoss};
  fillModel(iface.currentuEkBs());

  ui->qtbv_allConstituents->setModel(m_model);
  ui->qtbv_allConstituents->setSelectionMode(QAbstractItemView::SingleSelection);
  auto hdr = ui->qtbv_allConstituents->horizontalHeader();
  for (int col = 0; col < m_model->columnCount(); col++) {
    hdr->setSectionResizeMode(col, QHeaderView::ResizeToContents);
    ui->qtbv_allConstituents->setColumnWidth(col, hdr->sizeHintForColumn(col));
  }

  connect(ui->qpb_adjust, &QPushButton::clicked, this, [this]() {
    const auto &v = ui->qle_targetuEkB->text();

    bool ok;
    const double uEbK = DoubleToStringConvertor::back(v, &ok);
    if (!ok) {
      QMessageBox::warning(this, tr("Invalid input"), QString{tr("Invalid target %1 value")}.arg(uEffOverkBGEText()));
      return;
    }

    const auto selection = selectedConstituent();
    const auto &constutuentName = selection.first.toStdString();
    if (constutuentName.empty()) {
      QMessageBox::warning(this, tr("Invalid input"), tr("No background component was selected"));
      return;
    }
    if (selection.second) {
      QMessageBox::warning(this, tr("Invalid input"), tr("Selected constituent must be a background component"));
      return;
    }

    const double cOriginal = h_backgroundGDMProxy.concentrations(constutuentName).at(0);
    const double cSample = h_backgroundGDMProxy.concentrations(constutuentName).at(1);
    try {
      uEffOverkBGECalculatorInterface iface{h_backgroundGDMProxy, m_debyeHuckel, m_onsagerFuoss};
      double cAdjusted = iface.findConcentration(constutuentName, uEbK);

      h_backgroundGDMProxy.setConcentrations(constutuentName, { cAdjusted, cSample });
      fillModel(iface.currentuEkBs());

    } catch (const uEffOverkBGECalculatorInterface::Exception &ex) {
      // Make sure we reset concentrations to their original values
      h_backgroundGDMProxy.setConcentrations(constutuentName, { cOriginal, cSample });

      QMessageBox::warning(this, tr("Calculation failed"), ex.what());
    }
  });

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
    this->accept();
  });
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [this]() {
    // Restore original concentrations
    for (const auto &it : m_originalConcentrations) {
      const double cSample = h_backgroundGDMProxy.concentrations(it.first).at(1);
      h_backgroundGDMProxy.setConcentrations(it.first, { it.second, cSample });
    }

    this->reject();
  });
}

AdjustuEffOverkBGEDialog::~AdjustuEffOverkBGEDialog()
{
  delete ui;
}

void AdjustuEffOverkBGEDialog::fillModel(const std::map<std::string, double> &uEkBs)
{
  std::vector<AdjustuEffOverkBGETableModel::Item> data{};

  const auto getuEkB = [&uEkBs](const std::string &name) {
    auto it = uEkBs.find(name);
    return it == uEkBs.cend() ? 0.0 : it->second;
  };

  for (const auto &name : h_backgroundGDMProxy.allBackgroundNames()) {
    auto c = h_backgroundGDMProxy.concentrations(name)[0];
    data.emplace_back(QString::fromStdString(name), false, c, getuEkB(name));
  }
  for (const auto &name : h_backgroundGDMProxy.allAnalyteNames()) {
    auto c = h_sampleGDMProxy.concentrations(name)[0];
    data.emplace_back(QString::fromStdString(name), true, c, getuEkB(name));
  }

  m_model->setUnderlyingData(std::move(data));
}

QPair<QString, bool> AdjustuEffOverkBGEDialog::selectedConstituent()
{
  const auto &indexes = ui->qtbv_allConstituents->selectionModel()->selectedIndexes();
  if (indexes.empty())
    return {"", false};

  const auto &idx = indexes.constFirst();

  auto isAnalyte = m_model->data(m_model->index(idx.row(), 0), Qt::UserRole + 1).toBool();
  auto name = m_model->data(m_model->index(idx.row(), 1)).toString();

  return QPair<QString, bool>{std::move(name), isAnalyte};
}
