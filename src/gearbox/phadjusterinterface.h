#ifndef PHADJUSTERINTERFACE_H
#define PHADJUSTERINTERFACE_H

#include <stdexcept>
#include <string>

class BackgroundGDMProxy;
class CalculationContext;

class pHAdjusterInterface {
public:
  class Exception : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
  };

  pHAdjusterInterface(std::string constituentName, BackgroundGDMProxy &GDMProxy,
                      const bool debyeHuckel, const bool onsagerFuoss);
  pHAdjusterInterface(const pHAdjusterInterface &) = delete;
  pHAdjusterInterface(pHAdjusterInterface &&) = delete;
  ~pHAdjusterInterface();

  double adjustpH(const double targetpH);

  pHAdjusterInterface & operator=(const pHAdjusterInterface &) = delete;
  pHAdjusterInterface & operator=(pHAdjusterInterface &&) = delete;

private:
  CalculationContext *m_ctx;

  const std::string m_constituentName;
  BackgroundGDMProxy &h_GDMProxy;
  const bool m_debyeHuckel;
  const bool m_onsagerFuoss;

};

#endif // PHADJUSTERINTERFACE_H
