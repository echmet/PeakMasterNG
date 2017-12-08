#include "ligandionicform.h"
#include "constituent_ui.h"
#include <limits>

LigandIonicForm::LigandIonicForm() :
  charge(std::numeric_limits<int>::min()),
  _ligand(nullptr)
{
}

LigandIonicForm::LigandIonicForm(std::shared_ptr<Constituent> &ligand, const int charge) :
  charge(charge),
  _ligand(ligand)
{
}

QString LigandIonicForm::ligandName() const
{
  if (_ligand == nullptr)
    return QString("");

  return _ligand->name;
}

bool LigandIonicForm::matches(const int charge, const QString &ligandName) const
{
  if (_ligand == nullptr)
    return false;

  if (this->charge == charge &&
      _ligand->name == ligandName)
    return true;

  return false;
}

LigandIonicForm & LigandIonicForm::operator=(const LigandIonicForm &other)
{
  const_cast<int&>(charge) = other.charge;
  _ligand = other._ligand;

  return *this;
}

bool LigandIonicForm::operator==(const LigandIonicForm &other) const
{
  if (_ligand == nullptr)
    return false;

  if (charge == other.charge &&
      ligandName() == other.ligandName())
    return true;

  return false;
}

bool LigandIonicForm::operator!=(const LigandIonicForm &other) const
{
  return !(*this == other);
}
