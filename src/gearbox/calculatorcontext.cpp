#include "calculatorcontext.h"

#include <lemng.h>

CalculatorContext::CalculatorContext() :
  results{new ECHMET::LEMNG::Results{}},
  m_isAnalytesDissociationValid{false},
  m_isBGEValid{false},
  m_isValid{false}
{
}

CalculatorContext::CalculatorContext(const CalculatorContext &other) :
  results{new ECHMET::LEMNG::Results{}}, /* TODO: Look into copying the actual results */
  m_isAnalytesDissociationValid{other.m_isAnalytesDissociationValid},
  m_isBGEValid{other.m_isBGEValid},
  m_isValid{other.m_isValid}
{
}

CalculatorContext::CalculatorContext(CalculatorContext &&other) noexcept :
  results{other.results},
  m_isAnalytesDissociationValid{other.m_isAnalytesDissociationValid},
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

  m_isAnalytesDissociationValid = false;
  m_isBGEValid = false;
  m_isValid = false;
}

bool CalculatorContext::isAnalytesDissociationValid() const
{
  return m_isAnalytesDissociationValid;
}

bool CalculatorContext::isBGEValid() const
{
  return m_isBGEValid;
}

bool CalculatorContext::isValid() const
{
  return m_isValid;
}

void CalculatorContext::makeAnalytesDissociationValid()
{
  m_isAnalytesDissociationValid = true;
}

void CalculatorContext::makeBGEValid()
{
  m_isBGEValid = true;
}

void CalculatorContext::makeValid()
{
  m_isAnalytesDissociationValid = true;
  m_isBGEValid = true;
  m_isValid = true;
}
