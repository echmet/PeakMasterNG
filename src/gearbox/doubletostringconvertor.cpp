#include "doubletostringconvertor.h"
#include "inumberformatchangeable.h"

DoubleToStringConvertor *DoubleToStringConvertor::s_me{nullptr};


DoubleToStringConvertor::DoubleToStringConvertor() :
  QObject{nullptr},
  m_locale{QLocale::system()},
  m_digits{5},
  m_type{'g'}
{
}

DoubleToStringConvertor::~DoubleToStringConvertor()
{
}

double DoubleToStringConvertor::back(QString value, bool *ok)
{
  value.remove(s_me->m_locale.groupSeparator());

  return s_me->m_locale.toDouble(value, ok);
}

QString DoubleToStringConvertor::convert(const double d)
{
  return s_me->m_locale.toString(d, s_me->m_type, s_me->m_digits);
}

QString DoubleToStringConvertor::convert(const double d, const int digits)
{
  return s_me->m_locale.toString(d, s_me->m_type, digits);
}

int DoubleToStringConvertor::digits()
{
  return s_me->m_digits;
}

void DoubleToStringConvertor::initialize()
{
  s_me = new DoubleToStringConvertor();
}

const QLocale &DoubleToStringConvertor::locale()
{
  return s_me->m_locale;
}

void DoubleToStringConvertor::notifyOnFormatChanged(QObject *o)
{
  INumberFormatChangeable *nco = qobject_cast<INumberFormatChangeable *>(o);
  Q_ASSERT(nco != nullptr);

  s_me->m_listeners.push_back(nco);
}

void DoubleToStringConvertor::setParameters(const char type, const int digits, const QString locName)
{
  s_me->m_type = type;
  s_me->m_digits = digits;

  const QLocale *oldLocale = new QLocale(s_me->m_locale);

  s_me->m_locale = QLocale(locName);
  QLocale::setDefault(s_me->m_locale);

  for (INumberFormatChangeable *nco : s_me->m_listeners)
    nco->onNumberFormatChanged(oldLocale);

  delete oldLocale;
}

char DoubleToStringConvertor::type()
{
  return s_me->m_type;
}

