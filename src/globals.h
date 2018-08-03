#ifndef GLOBALS_H
#define GLOBALS_H

#include <QPointF>
#include <QString>
#include <QVector>
#include <QIcon>

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

  static const QString CIMEC_WEB_LINK;
  static const QString ECHMET_WEB_LINK;
  static const QString ORG_NAME;
  static const QString SOFTWARE_NAME;
  static const int VERSION_MAJ;
  static const int VERSION_MIN;
  static const QString VERSION_REV;

  static const QVector<DeveloperID> DEVELOPERS;
};

#endif // GLOBALS_H
