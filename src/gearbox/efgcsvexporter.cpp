#include "efgcsvexporter.h"
#include "calculatorinterface.h"
#include "efgdisplayer.h"

#include <QPointF>
#include <QVector>
#include <QFileDialog>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>

EFGCSVExporter *EFGCSVExporter::s_me{nullptr};

EFGDisplayer EFGCSVExporter::make()
{
  if (s_me == nullptr)
    s_me = new EFGCSVExporter{};

  auto me = EFGCSVExporter::s_me;
  auto executor= [me](const QVector<QPointF> &data, std::vector<CalculatorInterface::SpatialZoneInformation> &&szi, const CalculatorInterface::Signal &sig) {
    Q_UNUSED(szi);
    Q_UNUSED(sig);

    if (me->m_fileDlg->exec() != QDialog::Accepted)
      return;

    const auto &files = me->m_fileDlg->selectedFiles();
    if (files.empty())
      return;

    QFile output{files.first()};
    if (!output.open(QIODevice::WriteOnly | QIODevice::Text))
      throw CalculatorInterfaceException{"Cannot open output file"};

    QTextStream ostr;
    ostr.setCodec(QTextCodec::codecForName("UTF-8"));
    ostr.setDevice(&output);
    ostr.setRealNumberNotation(QTextStream::ScientificNotation);
    ostr.setRealNumberPrecision(17);

    for (const auto &pt : data)
      ostr << pt.x() << "\t" << pt.y() << "\n";

    output.flush();
  };

  return EFGDisplayer(executor);
}

EFGCSVExporter::EFGCSVExporter()
{
  m_fileDlg = new QFileDialog{};

  m_fileDlg->setAcceptMode(QFileDialog::AcceptSave);
  m_fileDlg->setNameFilter("CSV file (*.csv)");
}

EFGCSVExporter::~EFGCSVExporter()
{
  delete m_fileDlg;
}
