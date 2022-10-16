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

  enum class ExportTarget {
    FILE,
    CLIPBOARD
  };

  class Options {
  public:
    Options() :
      decimalSeparator{DecimalSeparator::PERIOD},
      fieldDelimiter{"\t"},
      exportTarget{ExportTarget::FILE}
    {}
    Options(const DecimalSeparator decimalSeparator, const QString &fieldDelimiter, const ExportTarget exportTarget) :
      decimalSeparator{decimalSeparator},
      fieldDelimiter{fieldDelimiter},
      exportTarget{exportTarget}
    {}
    Options(const Options &other) = default;
    Options(Options &&other) noexcept = default;

    Options & operator=(const Options &other)
    {
      const_cast<DecimalSeparator&>(decimalSeparator) = other.decimalSeparator;
      const_cast<QString&>(fieldDelimiter) = other.fieldDelimiter;
      const_cast<ExportTarget&>(exportTarget) = other.exportTarget;

      return *this;
    }

    const DecimalSeparator decimalSeparator;
    const QString fieldDelimiter;
    const ExportTarget exportTarget;
  };

  explicit CSVExportOptionsDialog(QWidget *parent = nullptr);
  ~CSVExportOptionsDialog();
  Options options() const;

private:
  Options makeOptions(const ExportTarget target) const;

  Ui::CSVExportOptionsDialog *ui;

  Options m_options;

private slots:
  void onToClipboardClicked();
  void onToFileClicked();
};
Q_DECLARE_METATYPE(CSVExportOptionsDialog::DecimalSeparator)

#endif // CSVEXPORTOPTIONSDIALOG_H
