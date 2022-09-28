// vim: sw=2 ts=2 sts=2 expandtab

#ifndef UEFFOVERKBGECALCULATORINTERFACE_H
#define UEFFOVERKBGECALCULATORINTERFACE_H

#include <map>
#include <stdexcept>
#include <string>

class BackgroundGDMProxy;
class CalculationContext;

class uEffOverkBGECalculatorInterface {
public:
  class Exception : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
  };

  uEffOverkBGECalculatorInterface(const BackgroundGDMProxy &GDMProxy,
                              const bool debyeHuckel, const bool onsagerFuoss);
  uEffOverkBGECalculatorInterface(const uEffOverkBGECalculatorInterface &) = delete;
  uEffOverkBGECalculatorInterface(uEffOverkBGECalculatorInterface &&) = delete;
  ~uEffOverkBGECalculatorInterface();

  std::map<std::string, double> currentuEkBs();
  double findConcentration(const std::string &constituentName, const double targetuEkB);

  uEffOverkBGECalculatorInterface & operator=(const uEffOverkBGECalculatorInterface &) = delete;
  uEffOverkBGECalculatorInterface & operator=(uEffOverkBGECalculatorInterface &&) = delete;

private:
  CalculationContext *m_ctx;

  const BackgroundGDMProxy &h_GDMProxy;
  int m_corrections;

};

#endif // UEFFOVERKBGECALCULATORINTERFACE_H
