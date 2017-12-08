#include "nucleusionicform.h"
#include <limits>

NucleusIonicForm::NucleusIonicForm() :
  charge(std::numeric_limits<int>::min())
{
}

NucleusIonicForm::NucleusIonicForm(const int charge) :
  charge(charge)
{
}

bool NucleusIonicForm::addLigandIon(const LigandIonicForm &ligandIF)
{
  for (const LigandIonicForm &ligIF : _ligandIonicForms) {
    if (ligIF == ligandIF)
      return false;
  }

  _ligandIonicForms.append(ligandIF);

  return true;
}

bool NucleusIonicForm::removeLigandIon(const QString ligandName, const int charge)
{
  for (int idx = 0; idx < _ligandIonicForms.size(); idx++) {
    const LigandIonicForm &ligIF = _ligandIonicForms.at(idx);

    if (ligIF.matches(charge, ligandName)) {
      _ligandIonicForms.removeAt(idx);
      return true;
    }
  }

  return false;
}
