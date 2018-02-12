#include "csvexportoptionsdialog.h"
#include "ui_csvexportoptionsdialog.h"

CSVExportOptionsDialog::CSVExportOptionsDialog(QWidget *parent) :
  QDialog{parent},
  ui{new Ui::CSVExportOptionsDialog}
{
  ui->setupUi(this);

  ui->qcbox_decimalSeparator->addItem("Period (.)", QVariant::fromValue<DecimalSeparator>(DecimalSeparator::PERIOD));
  ui->qcbox_decimalSeparator->addItem("Comma (,)", QVariant::fromValue<DecimalSeparator>(DecimalSeparator::COMMA));

  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &CSVExportOptionsDialog::reject);
  connect(ui->qpb_toClipboard, &QPushButton::clicked, this, &CSVExportOptionsDialog::onToClipboardClicked);
  connect(ui->qpb_toFile, &QPushButton::clicked, this, &CSVExportOptionsDialog::onToFileClicked);
}

CSVExportOptionsDialog::~CSVExportOptionsDialog()
{
  delete ui;
}

CSVExportOptionsDialog::Options CSVExportOptionsDialog::makeOptions(const CSVExportOptionsDialog::ExportTarget target) const
{
  QVariant decSep = ui->qcbox_decimalSeparator->currentData();
  QString text = ui->qle_fieldDelimiter->text();

  if (text == "\\t")
    text = "\t";

  return {decSep.value<DecimalSeparator>(), text, target};
}

void CSVExportOptionsDialog::onToClipboardClicked()
{
  m_options = makeOptions(ExportTarget::CLIPBOARD);
  accept();
}

void CSVExportOptionsDialog::onToFileClicked()
{
  m_options = makeOptions(ExportTarget::FILE);
  accept();
}

CSVExportOptionsDialog::Options CSVExportOptionsDialog::options() const
{
  return m_options;
}
