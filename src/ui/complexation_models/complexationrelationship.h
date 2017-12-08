#ifndef UI_COMPLEXATIONRELATIONSHIP_H
#define UI_COMPLEXATIONRELATIONSHIP_H

#include <memory>
#include <tuple>
#include <QMap>
#include <QString>
#include <QVector>

class Constituent;

class ComplexationRelationship
{
public:
  typedef std::tuple<QVector<double>, QVector<double>> ComplexFormProperties;
  typedef QMap<int, ComplexFormProperties> LigandComplexationMap;
  typedef QMap<int, LigandComplexationMap> RelationshipsMap;

  explicit ComplexationRelationship();
  ComplexationRelationship(const std::shared_ptr<Constituent> &nucleus, const std::shared_ptr<Constituent> &ligand);
  void addComplexForm(int nucleusCharge, int ligandCharge, const QVector<double> &mobilities, const QVector<double> &pBs);
  const ComplexFormProperties & properties(const int nucleusCharge, const int ligandCharge) const;

  const std::shared_ptr<const Constituent> ligand() const;
  const std::shared_ptr<const Constituent> nucleus() const;

private:
  std::shared_ptr<Constituent> _nucleus;
  std::shared_ptr<Constituent> _ligand;

  RelationshipsMap _knownComplexForms;

};

#endif // UI_COMPLEXATIONRELATIONSHIP_H
