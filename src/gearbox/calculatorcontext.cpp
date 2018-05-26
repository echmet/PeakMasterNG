#include "calculatorcontext.h"

#include <lemng.h>

CalculatorContext::CalculatorContext() :
  results{new ECHMET::LEMNG::Results{}},
  m_isBGEValid{false},
  m_isValid{CompleteResultsValidity::INVALID}
{
}

CalculatorContext::CalculatorContext(const CalculatorContext &other) :
  results{new ECHMET::LEMNG::Results{}}, /* TODO: Look into copying the actual results */
  m_isBGEValid{other.m_isBGEValid},
  m_isValid{other.m_isValid}
{
}

CalculatorContext::CalculatorContext(CalculatorContext &&other) noexcept :
  results{other.results},
  m_isBGEValid{other.m_isBGEValid},
  m_isValid{other.m_isValid}
{
  other.results = nullptr;
}

CalculatorContext::~CalculatorContext() noexcept
{
 if (results != nullptr) {
    if (m_isBGEValid)
      ECHMET::LEMNG::releaseResults(*results);
    delete results;
  }
}

void CalculatorContext::invalidate()
{
  if (m_isBGEValid) {
    ECHMET::LEMNG::releaseResults(*results);
    analytes.clear();
  }

  m_isBGEValid = false;
  m_isValid = CompleteResultsValidity::INVALID;
}

bool CalculatorContext::isBGEValid() const
{
  return m_isBGEValid;
}

CalculatorContext::CompleteResultsValidity CalculatorContext::isValid() const
{
  return m_isValid;
}

void CalculatorContext::makeBGEValid()
{
  m_isBGEValid = true;
}

void CalculatorContext::makeValid(const bool fullyValid)
{
  m_isBGEValid = true;
  m_isValid = fullyValid ? CompleteResultsValidity::OK : CompleteResultsValidity::PARTIAL;
}
