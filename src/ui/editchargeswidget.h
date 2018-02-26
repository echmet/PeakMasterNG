#ifndef EDITCHARGESWIDGET_H
#define EDITCHARGESWIDGET_H

#include <QWidget>
#include "internal_models/constituentchargesmodel.h"

namespace Ui {
  class EditChargesWidget;
}

namespace gdm {
  class PhysicalProperties;
}

class FloatingValueDelegate;
class ModifyConstituentChargePushButton;

class EditChargesWidget : public QWidget
{
  Q_OBJECT

public:
  explicit EditChargesWidget(QWidget *parent = nullptr);
  explicit EditChargesWidget(const  gdm::PhysicalProperties &props, QWidget *parent = nullptr);
  ~EditChargesWidget();
  int chargeHigh() const;
  int chargeLow() const;
  std::vector<double> mobilities() const;
  std::vector<double> pKas() const;
  void setCharges(const std::map<int, double> &pKas, const std::map<int, double> &mobilities, const int chargeLow, const int chargeHigh);

private:
  void setupWidget();
  void updateChargeHigh();
  void updateChargeLow();
  void updateChargeModifiers();

  Ui::EditChargesWidget *ui;

  ModifyConstituentChargePushButton *m_qpb_addLow;
  ModifyConstituentChargePushButton *m_qpb_removeLow;
  ModifyConstituentChargePushButton *m_qpb_addHigh;
  ModifyConstituentChargePushButton *m_qpb_removeHigh;

  FloatingValueDelegate *m_fltDelegate;
  ConstituentChargesModel m_chargesModel;

private slots:
  void onAddChargeLow();
  void onAddChargeHigh();
  void onRemoveChargeLow();
  void onRemoveChargeHigh();
};

#endif // EDITCHARGESWIDGET_H
