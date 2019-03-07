#include "efgcsvexporter.h"
#include "calculatorinterface.h"
#include "efgdisplayer.h"

#include <QPointF>
#include <QVector>
#include <QFileDialog>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QLocale>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>

#include "../ui/csvexportoptionsdialog.h"

EFGCSVExporter *EFGCSVExporter::s_me{nullptr};

EFGDisplayer EFGCSVExporter::make()
{
  if (s_me == nullptr)
    s_me = new EFGCSVExporter{};

  auto me = EFGCSVExporter::s_me;
  auto executor = [me](const QVector<QPointF> &data, const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi, const CalculatorInterface::Signal &sig) {
    Q_UNUSED(tdzi);
    Q_UNUSED(sig);

    for (;;) {
      if (me->m_optionsDlg->exec() != QDialog::Accepted)
        return;

      const auto opts = me->m_optionsDlg->options();

      if (opts.fieldDelimiter.length() != 1) {
        QMessageBox mbox{QMessageBox::Warning, QObject::tr("Invalid export options"), QObject::tr("Invalid field delimiter")};
        mbox.exec();
        continue;
      }

      if ((opts.fieldDelimiter == "." && opts.decimalSeparator == CSVExportOptionsDialog::DecimalSeparator::PERIOD) ||
          (opts.fieldDelimiter == "," && opts.decimalSeparator == CSVExportOptionsDialog::DecimalSeparator::COMMA)) {
        QMessageBox mbox{QMessageBox::Warning, QObject::tr("Invalid export options"), QObject::tr("Decimal and field separators cannot be the same")};
        mbox.exec();
        continue;
      }

      break;
    }
    const auto opts = me->m_optionsDlg->options();

    const QLocale loc = [](const CSVExportOptionsDialog::DecimalSeparator sep) {
      switch (sep) {
        case CSVExportOptionsDialog::DecimalSeparator::COMMA:
          return QLocale(QLocale::Czech);
        case CSVExportOptionsDialog::DecimalSeparator::PERIOD:
          return QLocale(QLocale::C);
      }

      throw std::runtime_error("Unhandled decimal separator value");
    }(opts.decimalSeparator);

    QString outStr;
    QTextStream ostr;
    ostr.setLocale(loc);
    ostr.setCodec(QTextCodec::codecForName("UTF-8"));
    ostr.setString(&outStr);
    ostr.setRealNumberNotation(QTextStream::ScientificNotation);
    ostr.setRealNumberPrecision(17);

    ostr << "time (min)" << opts.fieldDelimiter << sig.signalName << "\n";
    for (const auto &pt : data)
      ostr << pt.x() << opts.fieldDelimiter << pt.y() << "\n";

    if (opts.exportTarget == CSVExportOptionsDialog::ExportTarget::FILE) {
      if (me->m_fileDlg->exec() != QDialog::Accepted)
        return;

      const auto &files = me->m_fileDlg->selectedFiles();
      if (files.empty())
        return;

       QFile output{files.first()};
       if (!output.open(QIODevice::WriteOnly | QIODevice::Text))
         throw CalculatorInterfaceException{"Cannot open output file"};

       output.write(outStr.toUtf8());
    } else {
      QApplication::clipboard()->setText(outStr);
    }
  };

  return EFGDisplayer(executor);
}

EFGCSVExporter::EFGCSVExporter()
{
  m_fileDlg = new QFileDialog{};
  m_optionsDlg = new CSVExportOptionsDialog{};

  m_fileDlg->setAcceptMode(QFileDialog::AcceptSave);
  m_fileDlg->setNameFilter("CSV file (*.csv)");
}

EFGCSVExporter::~EFGCSVExporter()
{
  delete m_fileDlg;
  delete m_optionsDlg;
}
