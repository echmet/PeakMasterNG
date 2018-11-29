#include "additionalfloatingvalidator.h"

const char *AdditionalFloatingValidator::PROPERTY_NAME{"ADDITIONAL_VALIDATORS"};

AdditionalFloatingValidator::InvalidValidatorException::InvalidValidatorException() :
  std::runtime_error{"AdditionalFloatingValidator is invalid"}
{
}

AdditionalFloatingValidator::AdditionalFloatingValidator() :
  m_valid{false}
{
}

AdditionalFloatingValidator::AdditionalFloatingValidator(ValidFunc &&func) :
  m_valid{true},
  m_func(std::move(func))
{
}

bool AdditionalFloatingValidator::additionalValidatorsOk(const QObject *obj, const double dv)
{
  auto prop = obj->property(PROPERTY_NAME);
  if (prop.isValid()) {
    if (prop.canConvert<AdditionalFloatingValidatorVec>()) {
      auto validators = prop.value<AdditionalFloatingValidatorVec>();

      for (auto &&v : validators) {
        if (!v.validate(dv))
          return false;
      }
    }
  }

  return true;
}

bool AdditionalFloatingValidator::validate(const double dv)
{
  if (!m_valid)
    throw InvalidValidatorException{};

  return m_func(dv);
}
