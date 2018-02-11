#include "csvexportoptionsdialog.h"
#include "ui_csvexportoptionsdialog.h"

CSVExportOptionsDialog::CSVExportOptionsDialog(QWidget *parent) :
  QDialog{parent},
  ui{new Ui::CSVExportOptionsDialog}
{
  ui->setupUi(this);

  ui->qcbox_decimalSeparator->addItem("Period (.)", QVariant::fromValue<DecimalSeparator>(DecimalSeparator::PERIOD));
  ui->qcbox_decimalSeparator->addItem("Comma (,)", QVariant::fromValue<DecimalSeparator>(DecimalSeparator::COMMA));

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CSVExportOptionsDialog::accept);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &CSVExportOptionsDialog::reject);
}

CSVExportOptionsDialog::~CSVExportOptionsDialog()
{
  delete ui;
}

CSVExportOptionsDialog::Options CSVExportOptionsDialog::options() const
{
  QVariant decSep = ui->qcbox_decimalSeparator->currentData();
  QString text = ui->qle_fieldDelimiter->text();

  if (text == "\\t")
    text = "\t";

  return {decSep.value<DecimalSeparator>(), text};
}
