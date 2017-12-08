#include "complexationrelationship.h"

#include "constituent_ui.h"

#include <cassert>

ComplexationRelationship::ComplexationRelationship() :
  _nucleus(nullptr),
  _ligand(nullptr)
{
}

ComplexationRelationship::ComplexationRelationship(const std::shared_ptr<Constituent> &nucleus, const std::shared_ptr<Constituent> &ligand) :
  _nucleus(nucleus),
  _ligand(ligand)
{
  for (int nucleusCharge = nucleus->lowCharge; nucleusCharge <= nucleus->highCharge; nucleusCharge++) {
    for (int ligandCharge = ligand->lowCharge; ligandCharge <= ligand->highCharge; ligandCharge++) {
      _knownComplexForms[nucleusCharge][ligandCharge] = {{}, {}};
    }
  }
}

void ComplexationRelationship::addComplexForm(const int nucleusCharge, const int ligandCharge, const QVector<double> &mobilities, const QVector<double> &pBs)
{
  assert(nucleusCharge >= _nucleus->lowCharge && nucleusCharge <= _nucleus->highCharge);
  assert(ligandCharge >= _ligand->lowCharge && ligandCharge <= _ligand->highCharge);

  _knownComplexForms[nucleusCharge][ligandCharge] = {mobilities, pBs};
}

const std::shared_ptr<const Constituent> ComplexationRelationship::ligand() const
{
  return _ligand;
}

const std::shared_ptr<const Constituent> ComplexationRelationship::nucleus() const
{
  return _nucleus;
}

const ComplexationRelationship::ComplexFormProperties & ComplexationRelationship::properties(const int nucleusCharge, const int ligandCharge) const
{
  assert(nucleusCharge >= _nucleus->lowCharge && nucleusCharge <= _nucleus->highCharge);
  assert(ligandCharge >= _ligand->lowCharge && ligandCharge <= _ligand->highCharge);

  return *_knownComplexForms.find(nucleusCharge)->find(ligandCharge);
}
