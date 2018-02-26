#ifndef EDITCONSTITUENTDIALOG_H
#define EDITCONSTITUENTDIALOG_H

#include <QDialog>
#include "internal_models/constituentchargesmodel.h"
#include "../gearbox/iconstituenteditor.h"

namespace Ui {
  class EditConstituentDialog;
}

namespace gdm {
  class PhysicalProperties;
}

class DatabaseProxy;
class EditChargesWidget;
class FloatingValueDelegate;

class EditConstituentDialog : public QDialog, public IConstituentEditor
{
  Q_OBJECT

public:
  explicit EditConstituentDialog(DatabaseProxy &dbProxy, const bool viscosityCorrectionEnabled, QWidget *parent = nullptr);
  explicit EditConstituentDialog(DatabaseProxy &dbProxy, const QString &name, const EditConstituentDialog::ConstituentType type,
                                 const gdm::PhysicalProperties &props, const bool allowTypeChange, const bool viscosityCorrectionEnabled,
                                 QWidget *parent = nullptr);
  ~EditConstituentDialog();
  virtual int chargeHigh() const override;
  virtual int chargeLow() const override;
  virtual std::vector<double> mobilities() const override;
  virtual QString name() const override;
  virtual std::vector<double> pKas() const override;
  virtual ConstituentType type() const override;
  virtual double viscosityCoefficient() const override;

private:
  void setConstituentType(const EditConstituentDialog::ConstituentType type);
  void setViscosityElements(const double viscosityCoefficient);
  void setupWidget();

  Ui::EditConstituentDialog *ui;

  EditChargesWidget *m_editChargesWidget;

  DatabaseProxy &h_dbProxy;

  static const double VISCOSITY_COEFF_VERY_SMALL;
  static const double VISCOSITY_COEFF_SMALL;
  static const double VISCOSITY_COEFF_LARGE;

private slots:
  void onAccepted();
  void onAddToDatabase();
  void onPickFromDatabase();
  void onRejected();
  void onViscosityCoefficientIndexChanged(const int idx);

signals:
  void addToDatabase(const EditConstituentDialog *me);
  void validateInput(const EditConstituentDialog *me, bool *ok);
};


#endif // EDITCONSTITUENTDIALOG_H
