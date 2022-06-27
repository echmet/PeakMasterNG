#ifndef GLOBALS_H
#define GLOBALS_H

#include <QPointF>
#include <QString>
#include <QVector>
#include <QIcon>

#include <string>

#define SOFTWARE_NAME_INTERNAL_S "PeakMasterNG"

class Globals
{
public:
  class DeveloperID {
  public:
    const QString name;
    const QString mail;
    const bool reportBugs;

    QString linkString() const;
    QString prettyString() const;
  };

  Globals() = delete;

  static QIcon icon();
  static bool isZombieOS();
  static QString VERSION_STRING();

  static QString CIMEC_WEB_LINK();
  static QString ECHMET_WEB_LINK();
  static QString ORG_DOMAIN();
  static QString ORG_NAME();
  static QString SOFTWARE_NAME();
  static QString SOFTWARE_NAME_INTERNAL();
  static const int VERSION_MAJ;
  static const int VERSION_MIN;
  static QString VERSION_REV();

  static const QVector<DeveloperID> DEVELOPERS;

  static const std::string _CIMEC_WEB_LINK;
  static const std::string _ECHMET_WEB_LINK;
  static const std::string _ORG_DOMAIN;
  static const std::string _ORG_NAME;
  static const std::string _SOFTWARE_NAME;
  static const std::string _SOFTWARE_NAME_INTERNAL;
  static const std::string _VERSION_REV;
};

#endif // GLOBALS_H
