#include "globals.h"

#include <QPixmap>
#include <QSysInfo>

const std::string Globals::_CIMEC_WEB_LINK{"http://www.cimec.org.ar/twiki/bin/view/Cimec/"};
const std::string Globals::_ECHMET_WEB_LINK{"http://echmet.natur.cuni.cz"};
const std::string Globals::_ORG_DOMAIN{"echmet.natur.cuni.cz"};
const std::string Globals::_ORG_NAME{"ECHMET"};
const std::string Globals::_SOFTWARE_NAME{"PeakMaster"};
const std::string Globals::_SOFTWARE_NAME_INTERNAL{SOFTWARE_NAME_INTERNAL_S};
const int Globals::VERSION_MAJ = 6;
const int Globals::VERSION_MIN = 0;
const std::string Globals::_VERSION_REV{"g3"};

const QVector<Globals::DeveloperID> Globals::DEVELOPERS = {
                                                            Globals::DeveloperID{"Michal Jaro\xC5\xA1", "michal.jaros@gmail.com", false},
                                                            Globals::DeveloperID{"Milan \xC5\xA0t\xC4\x9B""dr\xC3\xBD", "stedry@natur.cuni.cz", false},
                                                            Globals::DeveloperID{"Vlastimil Hru\xC5\xA1""ka", "hruska2@natur.cuni.cz", false},
                                                            Globals::DeveloperID{"Iva Zuskov\xC3\xA1", "zuskova@natur.cuni.cz", false},
                                                            Globals::DeveloperID{"Bob Ga\xC5\xA1", "gas@natur.cuni.cz", false},
                                                            Globals::DeveloperID{"Pavel Dubsk\xC3\xBD", "pavel.dubsky@natur.cuni.cz", true},
                                                            Globals::DeveloperID{"Gabriel Gerlero", "gabrielgerlero@gmail.com", false},
                                                            Globals::DeveloperID{"Michal Mal\xC3\xBD", "pinksardine@organicunicornbeef.org", false}
                                                          };


QIcon Globals::icon()
{
#ifdef Q_OS_WIN
  static const QPixmap PROGRAM_ICON{":/images/res/PeakMaster_icon.ico"};
#else
  static const QPixmap PROGRAM_ICON{":/images/res/PeakMaster_icon.png"};
#endif // Q_OS_WIN

  if (PROGRAM_ICON.isNull())
    return QIcon{};

  return QIcon{PROGRAM_ICON};
}

QString Globals::DeveloperID::linkString() const
{
  return QString("%1 (<a href=\"mailto:%2\">%2</a>)").arg(name, mail.toHtmlEscaped());
}

QString Globals::DeveloperID::prettyString() const
{
  return QString("%1 (%2)").arg(name, mail);
}

bool Globals::isZombieOS()
{
#ifdef Q_OS_WIN
  static const QString WINDOWS("windows");
  static const QString XP("xp");

  const auto type = QSysInfo::productType();
  const auto ver = QSysInfo::productVersion();

  return type.toLower() == WINDOWS && ver.toLower() == XP;
#else
  return false;
#endif // Q_OS_WIN
}

QString Globals::VERSION_STRING()
{
  QString s = QString("%1 %2.%3%4").arg(_SOFTWARE_NAME.c_str()).arg(VERSION_MAJ).arg(VERSION_MIN).arg(_VERSION_REV.c_str());
#ifdef UNSTABLE_VERSION
  s.append(" Build date: [" + QString(__DATE__) + " - " + QString(__TIME__)  + "]");
#endif // UNSTABLE_VERSION

  return s;
}

QString Globals::CIMEC_WEB_LINK() { return QString::fromStdString(_CIMEC_WEB_LINK); }
QString Globals::ECHMET_WEB_LINK() { return QString::fromStdString(_ECHMET_WEB_LINK); }
QString Globals::ORG_DOMAIN() { return QString::fromStdString(_ORG_DOMAIN); }
QString Globals::ORG_NAME() { return QString::fromStdString(_ORG_NAME); }
QString Globals::SOFTWARE_NAME() { return QString::fromStdString(_SOFTWARE_NAME); }
QString Globals::SOFTWARE_NAME_INTERNAL() { return QString::fromStdString(_SOFTWARE_NAME_INTERNAL); }
QString Globals::VERSION_REV() { return QString::fromStdString(_VERSION_REV); }
