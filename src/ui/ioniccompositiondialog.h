#ifndef IONICCOMPOSITIONDIALOG_H
#define IONICCOMPOSITIONDIALOG_H

#include <QDialog>

namespace Ui {
  class IonicCompositionDialog;
}

class AnalytesDissociationModel;
class QAbstractTableModel;
class QStandardItemModel;

class IonicCompositionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit IonicCompositionDialog(QAbstractTableModel *model, AnalytesDissociationModel *analytesModel, QWidget *parent = nullptr);
  ~IonicCompositionDialog();

private:
  Ui::IonicCompositionDialog *ui;

  AnalytesDissociationModel *m_analytesModel;
  QStandardItemModel *m_analytesNamesModel;

  QString m_lastSelectedAnalyte;

private slots:
  void onAnalyteSelectionChanged(int idx);
  void onAnalytesDissociationDataUpdated();
};

#endif // IONICCOMPOSITIONDIALOG_H
