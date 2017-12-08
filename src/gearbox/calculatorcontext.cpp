#include "calculatorcontext.h"

#include <lemng.h>

CalculatorContext::CalculatorContext() :
  results{new ECHMET::LEMNG::Results{}},
  m_isValid{false}
{
}

CalculatorContext::CalculatorContext(const CalculatorContext &other) :
  results{new ECHMET::LEMNG::Results{}}, /* TODO: Look into copying the actual results */
  m_isValid{other.m_isValid}
{
}

CalculatorContext::CalculatorContext(CalculatorContext &&other) noexcept :
  results{other.results},
  m_isValid{other.m_isValid}
{
  other.results = nullptr;
}

CalculatorContext::~CalculatorContext() noexcept
{
 if (results != nullptr) {
    if (m_isValid)
      ECHMET::LEMNG::releaseResults(*results);
    delete results;
  }
}

void CalculatorContext::invalidate()
{
  if (m_isValid) {
    ECHMET::LEMNG::releaseResults(*results);
    analytes.clear();
  }

  m_isValid = false;
}

bool CalculatorContext::isValid() const
{
  return m_isValid;
}

void CalculatorContext::makeValid()
{
  m_isValid = true;
}
