#ifndef MODIFYCONSTITUENTCHARGEPUSHBUTTON_H
#define MODIFYCONSTITUENTCHARGEPUSHBUTTON_H

#include <QPushButton>

class ModifyConstituentChargePushButton : public QPushButton
{
  Q_OBJECT

public:
  enum class ChargeOperation {
    ADD,
    REMOVE
  };

  ModifyConstituentChargePushButton(const ChargeOperation op, QWidget *parent = nullptr);
  void setText(const int charge);

private:
  const ChargeOperation m_chargeOp;
};

#endif // MODIFYCONSTITUENTCHARGEPUSHBUTTON_H
