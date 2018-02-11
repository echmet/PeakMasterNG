#ifndef EFGCSVEXPORTER_H
#define EFGCSVEXPORTER_H

class CSVExportOptionsDialog;
class EFGDisplayer;
class QFileDialog;

class EFGCSVExporter
{
public:
  static EFGDisplayer make();

private:
  explicit EFGCSVExporter();
  ~EFGCSVExporter();

  QFileDialog *m_fileDlg;
  CSVExportOptionsDialog *m_optionsDlg;

  static EFGCSVExporter *s_me;
};

#endif // EFGCSVEXPORTER_H
