#ifndef GDM_PHYSICALPROPERTIES_H
#define GDM_PHYSICALPROPERTIES_H

#include "../common/chargeinterval.h"
#include "../common/mobility.h"

#include <vector>

namespace gdm {

class PhysicalProperties
{
public:
    /*implicit*/ PhysicalProperties() = default; //Constructs an object with just charge zero
    explicit PhysicalProperties(ChargeInterval charges, std::vector<double> pKas, std::vector<double> mobilities, double viscosityCoefficient); //mobilities must have same size as charges; pKas one less

    void assign(ChargeInterval charges, const std::vector<double>& pKas, const std::vector<double>& mobilities, double viscosityCoefficient); //mobilities must have same size as charges; pKas one less

    ChargeInterval charges() const noexcept;
    double pKa(ChargeNumber targetCharge) const noexcept;
    Mobility mobility(ChargeNumber charge) const noexcept;

    ChargeNumber pushLowCharge(double pKa = 0.0, Mobility mobility = 0.0); //mobility must be zero if charge added is zero, positive otherwise; returns the charge added
    ChargeNumber pushHighCharge(double pKa = 0.0, Mobility mobility = 0.0); //mobility must be zero if charge added is zero, positive otherwise; returns the charge added
    ChargeNumber popLowCharge(); //Object must have at least two charges; returns the charge removed
    ChargeNumber popHighCharge(); //Object must have at least two charges; returns the charge removed

    void setPKa(ChargeNumber targetCharge, double pKa);
    void setMobility(ChargeNumber charge, Mobility mobility);  //mobility must be zero if charge added is zero, positive otherwise

    const std::vector<Mobility>& mobilities() const noexcept;
    const std::vector<double>& pKas() const noexcept;
    double viscosityCoefficient() const noexcept;

private:
    std::vector<double>::size_type pKaIndex(ChargeNumber targetCharge) const noexcept; //Returns the index in the _pKas vector that corresponds to a given charge
    std::vector<Mobility>::size_type mobilityIndex(ChargeNumber charge) const noexcept; //Returns the index in the _mobilities vector that corresponds to a given charge

    ChargeInterval _charges = {0};
    std::vector<double> _pKas = {};
    std::vector<Mobility> _mobilities = {0.0};
    double _viscosityCoefficient = 0.0;
};

bool operator==(const PhysicalProperties& a, const PhysicalProperties& b) noexcept;
bool operator!=(const PhysicalProperties& a, const PhysicalProperties& b) noexcept;

} // namespace gdm

#endif // GDM_PHYSICALPROPERTIES_H
