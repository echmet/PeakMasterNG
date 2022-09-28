#ifndef ADJUSTUEFFOVERKBGEDIALOG_H
#define ADJUSTUEFFOVERKBGEDIALOG_H

#include <QDialog>
#include <QPair>

class BackgroundGDMProxy;
class SampleGDMProxy;
class GDMProxy;
class AdjustuEffOverkBGETableModel;

namespace Ui {
class AdjustuEffOverkBGEDialog;
}

class AdjustuEffOverkBGEDialog : public QDialog {
  Q_OBJECT
public:
  explicit AdjustuEffOverkBGEDialog(GDMProxy &backgroundGDMProxy, GDMProxy &sampleGDMProxy,
                                    const bool debyeHuckel, const bool onsagerFuoss,
                                    QWidget *parent = nullptr);
  ~AdjustuEffOverkBGEDialog();

private:
  void fillModel(const std::map<std::string, double> &uEkBs = {});
  QPair<QString, bool> selectedConstituent();

  Ui::AdjustuEffOverkBGEDialog *ui;

  AdjustuEffOverkBGETableModel *m_model;
  std::map<std::string, double> m_originalConcentrations;

  BackgroundGDMProxy &h_backgroundGDMProxy;
  SampleGDMProxy &h_sampleGDMProxy;

  const bool m_debyeHuckel;
  const bool m_onsagerFuoss;

private slots:
  void adjust();
};

#endif // ADJUSTUEFFOVERKBGEDIALOG_H
