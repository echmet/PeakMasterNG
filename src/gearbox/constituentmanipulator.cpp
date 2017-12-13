#include "constituentmanipulator.h"

#include "../ui/editconstituentdialog.h"
#include "../gdm/core/common/gdmexcept.h"
#include "../gdm/core/constituent/physicalproperties.h"
#include "gdmproxy.h"

#include <cassert>
#include <QMessageBox>

bool validateConstituentProperties(const EditConstituentDialog *dlg)
{
  const QString name = dlg->name();
  const auto pKas = dlg->pKas();
  const auto mobilities = dlg->mobilities();
  const int chargeLow = dlg->chargeLow();
  const int chargeHigh = dlg->chargeHigh();

  if (name.length() < 1) {
    QMessageBox mbox{QMessageBox::Warning, QObject::tr("Invalid constituent properties"), QObject::tr("Constituent must have a name")};
    mbox.exec();
    return false;
  }

  /* Try to create a physical properties of constituent */
  try {
    gdm::PhysicalProperties props{gdm::ChargeInterval{chargeLow, chargeHigh}, pKas, mobilities};
  } catch (gdm::InvalidArgument &ex) {
    QMessageBox mbox{QMessageBox::Warning, QObject::tr("Invalid constituent properties"), ex.what()};
    mbox.exec();
    return false;
  }

  return true;
}

ConstituentManipulator::ConstituentManipulator(const GDMProxy &proxy) :
  h_proxy{proxy}
{
}

gdm::Constituent ConstituentManipulator::makeConstituent(const EditConstituentDialog *dlg)
{
  const gdm::ConstituentType type = [](EditConstituentDialog::ConstituentType _type) {
    switch (_type) {
      case EditConstituentDialog::ConstituentType::NUCLEUS:
        return gdm::ConstituentType::Nucleus;
      case EditConstituentDialog::ConstituentType::LIGAND:
        return gdm::ConstituentType::Ligand;
    }
  }(dlg->type());
  const std::string name = dlg->name().toStdString();
  auto mobilities = dlg->mobilities();
  auto pKas = dlg->pKas();
  const int chargeLow = dlg->chargeLow();
  const int chargeHigh = dlg->chargeHigh();
  gdm::PhysicalProperties physProps{gdm::ChargeInterval{chargeLow, chargeHigh}, pKas, mobilities};

  return gdm::Constituent{type, name, std::move(physProps)};
}

EditConstituentDialog * ConstituentManipulator::makeEditDialog(const std::string &name, GDMProxy &proxy, DatabaseProxy &dbProxy)
{
  assert(proxy.contains(name));

  gdm::Constituent ctuent = proxy.get(name);
  EditConstituentDialog::ConstituentType type = [](gdm::ConstituentType _type) {
    switch (_type) {
    case gdm::ConstituentType::Nucleus:
      return EditConstituentDialog::ConstituentType::NUCLEUS;
    case gdm::ConstituentType::Ligand:
      return EditConstituentDialog::ConstituentType::LIGAND;
    }
  }(ctuent.type());

  const bool allowTypeChange = !proxy.complexes(name);

  return new EditConstituentDialog{dbProxy, QString::fromStdString(name), type, ctuent.physicalProperties(), allowTypeChange};
}

void ConstituentManipulator::onValidateConstituentInput(const EditConstituentDialog *dlg, bool *ok)
{
  *ok = validateConstituentProperties(dlg);

  if (h_proxy.contains(dlg->name().toStdString())) {
    QMessageBox mbox{QMessageBox::Warning, tr("Invalid constituent"), QString(tr("Constituent with name \"%1\" is already present")).arg(dlg->name())};
    mbox.exec();
    *ok = false;
  }
}

void ConstituentManipulator::onValidateConstituentInputUpdate(const EditConstituentDialog *dlg, bool *ok)
{
  *ok = validateConstituentProperties(dlg);
}
