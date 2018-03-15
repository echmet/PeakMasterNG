#include "doubletostringconvertor.h"
#include "inumberformatchangeable.h"

#include <cmath>

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
  const QString s = s_me->m_locale.toString(d, s_me->m_type, s_me->m_digits);

  return s_me->correctTralingDecSep(s);
}

QString DoubleToStringConvertor::convert(const double d, const char format, const int digits)
{
  QString s =s_me->m_locale.toString(d, format, digits);

  /* Covert a case where the resulting string is rounded and contains trailing zeros as a result */
  const int idx = s.lastIndexOf(s_me->m_locale.decimalPoint());

  if (idx < 0)
    return s;

  int lastNonZeroIdx = idx;
  for (int _idx = s.length() - 1; _idx > idx; _idx--) {
    if (s.at(_idx) != '0') {
      lastNonZeroIdx = _idx;
      break;
    }
  }

  s = s.mid(0, lastNonZeroIdx + 1);

  return s_me->correctTralingDecSep(s);
}

QString DoubleToStringConvertor::convert(const double d, const int digits)
{
  const QString s = s_me->m_locale.toString(d, s_me->m_type, digits);

  return s_me->correctTralingDecSep(s);
}

QString DoubleToStringConvertor::correctTralingDecSep(const QString &s)
{
  const QChar &ds = s_me->m_locale.decimalPoint();

  if (s.endsWith(ds))
    return s.mid(0, s.length() - 1);
  return s;
}

int DoubleToStringConvertor::decimalDigits(QString value)
{
  value.remove(s_me->m_locale.groupSeparator());
  value.remove(' ');

  const int idx = value.indexOf(s_me->m_locale.decimalPoint());
  if (idx < 0)
    return 0;

  int lastNonzeroChar = idx;
  for (int _idx = idx + 1; _idx < value.length(); _idx++) {
    if (value.at(_idx) != '0')
      lastNonzeroChar = _idx;
  }

  return lastNonzeroChar - idx;
}

int DoubleToStringConvertor::digits()
{
  return s_me->m_digits;
}

int DoubleToStringConvertor::guessPrecision(const double d)
{
  int finPrec = s_me->m_digits;

  if (d == 0)
    return 0;

  for (int exp = 0; exp <= finPrec; exp++) {
    const double fact = std::pow(10, exp);
    const double nd = std::floor(d * fact) / fact;

    if (std::abs(nd) < 1.0e-13) {
      finPrec++;
      if (finPrec > 17)
        return 17;
      continue;
    }

    if (std::abs(d - nd) <= std::pow(10, -(exp + 3)))
      return exp;
  }

  return finPrec;
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

