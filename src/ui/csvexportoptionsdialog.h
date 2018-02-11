#ifndef CSVEXPORTOPTIONSDIALOG_H
#define CSVEXPORTOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class CSVExportOptionsDialog;
}

class CSVExportOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  enum class DecimalSeparator {
    PERIOD,
    COMMA
  };

  class Options {
  public:
    const DecimalSeparator decimalSeparator;
    const QString fieldDelimiter;
  };

  explicit CSVExportOptionsDialog(QWidget *parent = nullptr);
  ~CSVExportOptionsDialog();
  Options options() const;

private:
  Ui::CSVExportOptionsDialog *ui;
};
Q_DECLARE_METATYPE(CSVExportOptionsDialog::DecimalSeparator)

#endif // CSVEXPORTOPTIONSDIALOG_H
