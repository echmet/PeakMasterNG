#ifndef ADJUSTPHDIALOG_H
#define ADJUSTPHDIALOG_H

#include <QDialog>

#include <map>
#include <string>

namespace Ui {
  class AdjustpHDialog;
}

class AdjustpHTableModel;
class BackgroundGDMProxy;
class GDMProxy;

class AdjustpHDialog : public QDialog
{
  Q_OBJECT
public:
  explicit AdjustpHDialog(GDMProxy &GDMProxy,
                          const bool debyeHuckel, const bool onsagerFuoss,
                          const double currentpH,
                          QWidget *parent = nullptr);
  ~AdjustpHDialog();
  void reject() override;

private:
  void adjustConcentration(const QString &ctuent, double targetpH);
  void calculatepH();
  double getTargetpH(bool *ok);
  void restoreConcentrations();
  QString selectedConstituent();

  Ui::AdjustpHDialog *ui;

  BackgroundGDMProxy &h_GDMProxy;
  AdjustpHTableModel *m_model;
  std::map<std::string, double> m_originalConcentrations;
  const bool m_debyeHuckel;
  const bool m_onsagerFuoss;
};

#endif // ADJUSTPHDIALOG_H
