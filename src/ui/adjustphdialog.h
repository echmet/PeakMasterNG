#ifndef ADJUSTPHDIALOG_H
#define ADJUSTPHDIALOG_H

#include <QDialog>

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

private:
  void onAdjustClicked();
  QString selectedConstituent();

  Ui::AdjustpHDialog *ui;

  BackgroundGDMProxy &h_GDMProxy;
  AdjustpHTableModel *m_model;
  const bool m_debyeHuckel;
  const bool m_onsagerFuoss;
};

#endif // ADJUSTPHDIALOG_H
