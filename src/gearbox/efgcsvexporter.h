#ifndef EFGCSVEXPORTER_H
#define EFGCSVEXPORTER_H

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

  static EFGCSVExporter *s_me;
};

#endif // EFGCSVEXPORTER_H
