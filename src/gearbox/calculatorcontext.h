#ifndef CALCULATORCONTEXT_H
#define CALCULATORCONTEXT_H

#include <string>
#include <vector>

namespace ECHMET {
  namespace LEMNG {
    class Results;
  }
}

class CalculatorContext
{
public:
  enum class CompleteResultsValidity {
    INVALID,
    OK,
    PARTIAL
  };

  CalculatorContext();
  CalculatorContext(const CalculatorContext &other);
  CalculatorContext(CalculatorContext &&other) noexcept;
  ~CalculatorContext() noexcept;
  void invalidate();
  bool isBGEValid() const;
  CompleteResultsValidity isValid() const;
  void makeBGEValid();
  void makeValid(const bool fullyValid);

  std::vector<std::string> analytes;
  ECHMET::LEMNG::Results *results;

private:
  bool m_isBGEValid;
  CompleteResultsValidity m_isValid;
};

#endif // CALCULATORCONTEXT_H
