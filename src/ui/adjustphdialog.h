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
  explicit AdjustpHDialog(const GDMProxy &GDMProxy, QWidget *parent = nullptr);
  ~AdjustpHDialog();

private:
  Ui::AdjustpHDialog *ui;

  const BackgroundGDMProxy &h_GDMProxy;
  AdjustpHTableModel *m_model;
};

#endif // ADJUSTPHDIALOG_H
