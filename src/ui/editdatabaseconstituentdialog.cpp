#include "editdatabaseconstituentdialog.h"
#include "ui_editdatabaseconstituentdialog.h"

#include "editchargeswidget.h"

#include <QVBoxLayout>

EditDatabaseConstituentDialog::EditDatabaseConstituentDialog(const QString &name, const std::map<int, double> &pKas, const std::map<int, double> &mobilities,
                                                             const int chargeLow, const int chargeHigh,
                                                             QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EditDatabaseConstituentDialog}
{
  ui->setupUi(this);

  m_editChargesWidget = new EditChargesWidget{pKas, mobilities, chargeLow, chargeHigh, this};
  static_cast<QVBoxLayout *>(layout())->insertWidget(1, m_editChargesWidget);
  ui->qle_name->setText(name);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &EditDatabaseConstituentDialog::onAccepted);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &EditDatabaseConstituentDialog::reject);
}

EditDatabaseConstituentDialog::~EditDatabaseConstituentDialog()
{
  delete ui;
}

int EditDatabaseConstituentDialog::chargeLow() const
{
  return m_editChargesWidget->chargeLow();
}

int EditDatabaseConstituentDialog::chargeHigh() const
{
  return m_editChargesWidget->chargeHigh();
}

std::vector<double> EditDatabaseConstituentDialog::mobilities() const
{
  return m_editChargesWidget->mobilities();
}

QString EditDatabaseConstituentDialog::name() const
{
  return ui->qle_name->text();
}

void EditDatabaseConstituentDialog::onAccepted()
{
  bool ok;

  emit validateInput(this, &ok);

  if (ok)
    accept();
}

std::vector<double> EditDatabaseConstituentDialog::pKas() const
{
  return m_editChargesWidget->pKas();
}

EditDatabaseConstituentDialog::ConstituentType EditDatabaseConstituentDialog::type() const
{
  return ConstituentType::NUCLEUS;
}

double EditDatabaseConstituentDialog::viscosityCoefficient() const
{
  return 0;
}
