#ifndef ADDITIONALVALIDATOR_H
#define ADDITIONALVALIDATOR_H

#include <functional>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVector>

class AdditionalFloatingValidator {
public:
  class InvalidValidatorException : public std::runtime_error {
  public:
    InvalidValidatorException();
  };

  using ValidFunc = std::function<bool (const double)>;

  AdditionalFloatingValidator();
  AdditionalFloatingValidator(ValidFunc &&func);
  virtual ~AdditionalFloatingValidator() = default;

  virtual bool validate(const double d);

  static bool additionalValidatorsOk(const QObject *obj, const double dv);

  static const char * PROPERTY_NAME;

private:
  const bool m_valid;
  const ValidFunc m_func;

};
using AdditionalFloatingValidatorVec = QVector<AdditionalFloatingValidator>;

Q_DECLARE_METATYPE(AdditionalFloatingValidator);
Q_DECLARE_METATYPE(AdditionalFloatingValidatorVec);



#endif // ADDITIONALVALIDATOR_H
