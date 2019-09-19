#ifndef ADJUSTPHDIALOG_H
#define ADJUSTPHDIALOG_H

#include <QDialog>

namespace Ui {
class AdjustpHDialog;
}

class AdjustpHDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AdjustpHDialog(QWidget *parent = nullptr);
  ~AdjustpHDialog();

private:
  Ui::AdjustpHDialog *ui;
};

#endif // ADJUSTPHDIALOG_H
