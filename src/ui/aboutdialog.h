#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AboutDialog(QWidget *parent = nullptr);
  ~AboutDialog();

private:
  Ui::AboutDialog *ui;

  QPixmap m_cimecPix;
  QPixmap m_echmetPix;

private slots:
  void onCloseClicked();

  qreal scaleFactor();

};

#endif // ABOUTDIALOG_H
