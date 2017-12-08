#include "modifyconstituentchargepushbutton.h"
#include <stdexcept>

ModifyConstituentChargePushButton::ModifyConstituentChargePushButton(const ChargeOperation op, QWidget *parent) :
  QPushButton{parent},
  m_chargeOp{op}
{
}

void ModifyConstituentChargePushButton::setText(const int charge)
{
  QString text = [](const ChargeOperation op) {
    switch (op) {
    case ChargeOperation::ADD:
      return tr("Add");
    case ChargeOperation::REMOVE:
      return tr("Remove");
    }
  }(m_chargeOp);

  text += QString{" (%1)"}.arg(charge);
  QPushButton::setText(text);
}
