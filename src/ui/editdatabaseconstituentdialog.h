#ifndef EDITDATABASECONSTITUENTDIALOG_H
#define EDITDATABASECONSTITUENTDIALOG_H

#include "../gearbox/iconstituenteditor.h"

#include <map>
#include <QDialog>

namespace Ui {
  class EditDatabaseConstituentDialog;
}

class EditChargesWidget;

class EditDatabaseConstituentDialog : public QDialog, public IConstituentEditor
{
  Q_OBJECT

public:
  explicit EditDatabaseConstituentDialog(const QString &name, const std::map<int, double> &pKas, const std::map<int, double> &mobilities,
                                         const int chargeLow, const int chargeHigh, QWidget *parent = nullptr);
  ~EditDatabaseConstituentDialog();

  virtual QString name() const override;
  virtual std::vector<double> pKas() const override;
  virtual std::vector<double> mobilities() const override;
  virtual int chargeLow() const override;
  virtual int chargeHigh() const override;
  virtual ConstituentType type() const override;
  virtual double viscosityCoefficient() const override;

private:
  Ui::EditDatabaseConstituentDialog *ui;

  EditChargesWidget *m_editChargesWidget;

private slots:
  void onAccepted();

signals:
  void validateInput(const EditDatabaseConstituentDialog *me, bool *ok);
};

#endif // EDITDATABASECONSTITUENTDIALOG_H
