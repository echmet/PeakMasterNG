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
  CalculatorContext();
  CalculatorContext(const CalculatorContext &other);
  CalculatorContext(CalculatorContext &&other) noexcept;
  ~CalculatorContext() noexcept;
  void invalidate();
  bool isAnalytesDissociationValid() const;
  bool isBGEValid() const;
  bool isValid() const;
  void makeAnalytesDissociationValid();
  void makeBGEValid();
  void makeValid();

  std::vector<std::string> analytes;
  ECHMET::LEMNG::Results *results;

private:
  bool m_isAnalytesDissociationValid;
  bool m_isBGEValid;
  bool m_isValid;
};

#endif // CALCULATORCONTEXT_H
