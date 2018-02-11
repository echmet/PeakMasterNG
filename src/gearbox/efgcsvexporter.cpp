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

#include "../ui/csvexportoptionsdialog.h"

EFGCSVExporter *EFGCSVExporter::s_me{nullptr};

EFGDisplayer EFGCSVExporter::make()
{
  if (s_me == nullptr)
    s_me = new EFGCSVExporter{};

  auto me = EFGCSVExporter::s_me;
  auto executor = [me](const QVector<QPointF> &data, std::vector<CalculatorInterface::SpatialZoneInformation> &&szi, const CalculatorInterface::Signal &sig) {
    Q_UNUSED(szi);
    Q_UNUSED(sig);

    if (me->m_fileDlg->exec() != QDialog::Accepted)
      return;

    const auto &files = me->m_fileDlg->selectedFiles();
    if (files.empty())
      return;

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

    QFile output{files.first()};
    if (!output.open(QIODevice::WriteOnly | QIODevice::Text))
      throw CalculatorInterfaceException{"Cannot open output file"};

    const QLocale loc = [](const CSVExportOptionsDialog::DecimalSeparator sep) {
      switch (sep) {
        case CSVExportOptionsDialog::DecimalSeparator::COMMA:
          return QLocale(QLocale::Czech);
        case CSVExportOptionsDialog::DecimalSeparator::PERIOD:
          return QLocale(QLocale::C);
      }
    }(opts.decimalSeparator);

    QTextStream ostr;
    ostr.setLocale(loc);
    ostr.setCodec(QTextCodec::codecForName("UTF-8"));
    ostr.setDevice(&output);
    ostr.setRealNumberNotation(QTextStream::ScientificNotation);
    ostr.setRealNumberPrecision(17);

    for (const auto &pt : data)
      ostr << pt.x() << opts.fieldDelimiter << pt.y() << "\n";

    output.flush();
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
