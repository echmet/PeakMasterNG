#ifndef GDM_COMPLEXFORM_H
#define GDM_COMPLEXFORM_H

#include <vector>
#include <cstddef>

#include "../common/mobility.h"
#include "chargecombination.h"

namespace gdm {

class ComplexForm
{
public:
    /*implicit*/ ComplexForm(ChargeCombination charges, std::size_t maxCount, std::vector<double> pBs, std::vector<Mobility> mobilities);

    ChargeCombination charges() const noexcept;

    std::size_t maxCount() const noexcept;
    const std::vector<double>& pBs() const noexcept;
    const std::vector<Mobility>& mobilities() const noexcept;


    void assign(ChargeCombination charges, std::size_t maxCount, const std::vector<double>& pBs, const std::vector<Mobility>& mobilities);
    void assign(std::size_t maxCount, const std::vector<double>& pBs, const std::vector<Mobility>& mobilities);

private:
    ChargeCombination _charges;
    std::vector<double> _pBs;
    std::vector<Mobility> _mobilities;
};

bool operator==(const ComplexForm& a, const ComplexForm& b) noexcept;
bool operator!=(const ComplexForm& a, const ComplexForm& b) noexcept;

} // namespace gdm

#endif // GDM_COMPLEXFORM_H
