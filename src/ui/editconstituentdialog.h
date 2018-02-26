#ifndef EDITCONSTITUENTDIALOG_H
#define EDITCONSTITUENTDIALOG_H

#include <QDialog>
#include "internal_models/constituentchargesmodel.h"

namespace Ui {
  class EditConstituentDialog;
}

namespace gdm {
  class PhysicalProperties;
}

class DatabaseProxy;
class EditChargesWidget;
class FloatingValueDelegate;

class EditConstituentDialog : public QDialog
{
  Q_OBJECT

public:
  enum class ConstituentType {
    NUCLEUS,
    LIGAND
  };
  Q_ENUM(ConstituentType)

  explicit EditConstituentDialog(DatabaseProxy &dbProxy, const bool viscosityCorrectionEnabled, QWidget *parent = nullptr);
  explicit EditConstituentDialog(DatabaseProxy &dbProxy, const QString &name, const EditConstituentDialog::ConstituentType type,
                                 const gdm::PhysicalProperties &props, const bool allowTypeChange, const bool viscosityCorrectionEnabled,
                                 QWidget *parent = nullptr);
  ~EditConstituentDialog();
  int chargeHigh() const;
  int chargeLow() const;
  std::vector<double> mobilities() const;
  QString name() const;
  std::vector<double> pKas() const;
  ConstituentType type() const;
  double viscosityCoefficient() const;

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

Q_DECLARE_METATYPE(EditConstituentDialog::ConstituentType)

#endif // EDITCONSTITUENTDIALOG_H
