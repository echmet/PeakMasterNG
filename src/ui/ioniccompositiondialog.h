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
  ~IonicCompositionDialog() override;

  virtual QSize sizeHint() const override;

private:
  Ui::IonicCompositionDialog *ui;

  AnalytesDissociationModel *m_analytesModel;
  QStandardItemModel *m_analytesNamesModel;

  QString m_lastSelectedAnalyte;

  static const QString FILL_TO_STRING;

private slots:
  void onAnalyteSelectionChanged(int idx);
  void onAnalytesDissociationDataUpdated();
};

#endif // IONICCOMPOSITIONDIALOG_H
