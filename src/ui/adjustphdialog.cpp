#include "adjustphdialog.h"
#include "ui_adjustphdialog.h"

#include "internal_models/adjustphtablemodel.h"
#include "../gearbox/backgroundgdmproxy.h"

AdjustpHDialog::AdjustpHDialog(const GDMProxy &GDMProxy, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::AdjustpHDialog},
  h_GDMProxy{dynamic_cast<const BackgroundGDMProxy&>(GDMProxy)}
{
  ui->setupUi(this);

  m_model = new AdjustpHTableModel{h_GDMProxy.allBackgroundNames(), h_GDMProxy, this};
  ui->qtbv_bgeConstituents->setModel(m_model);
}

AdjustpHDialog::~AdjustpHDialog()
{
  delete ui;
}
