#ifndef DOUBLETOSTRINGCONVERTOR_H
#define DOUBLETOSTRINGCONVERTOR_H

#include <QLocale>
#include <QVector>

class INumberFormatChangeable;

class DoubleToStringConvertor : public QObject
{
public:
  virtual ~DoubleToStringConvertor();
  static double back(QString value, bool *ok);
  static QString convert(const double d);
  static QString convert(const double d, const int digits);
  static QString convert(const double d, const char format, const int digits);
  static int decimalDigits(QString value);
  static int digits();
  static int guessPrecision(const double d);
  static void initialize();
  static void loadUserSettings(const QVariant &settings);
  static const QLocale &locale();
  static void notifyOnFormatChanged(QObject *o);
  static QVariant saveUserSettings();
  static void setParameters(const char type, int digits, const QString locName);
  static char type();

private:
  DoubleToStringConvertor();
  QString correctTralingDecSep(const QString &s);

  QLocale m_locale;
  int m_digits;
  char m_type;

  QVector<INumberFormatChangeable *> m_listeners;

  static DoubleToStringConvertor *s_me;
};

#endif // DOUBLETOSTRINGCONVERTOR_H
