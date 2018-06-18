#include "globals.h"

#include <QPixmap>

const QString Globals::CIMEC_WEB_LINK{"http://www.cimec.org.ar/twiki/bin/view/Cimec/"};
const QString Globals::ECHMET_WEB_LINK{"http://echmet.natur.cuni.cz"};
const QString Globals::ORG_NAME{"ECHMET"};
const QString Globals::SOFTWARE_NAME{"PeakMaster"};
const int Globals::VERSION_MAJ = 6;
const int Globals::VERSION_MIN = 0;
const QString Globals::VERSION_REV{"d"};

const QVector<Globals::DeveloperID> Globals::DEVELOPERS = {
                                                            Globals::DeveloperID{u8"Milan \xC5\xA0t\xC4\x9B""dr\xC3\xBD", "stedry@natur.cuni.cz", false},
                                                            Globals::DeveloperID{u8"Vlastimil Hru\xC5\xA1""ka", "hruska2@natur.cuni.cz", false},
                                                            Globals::DeveloperID{u8"Michal Jaro\xC5\xA1", "mjaros@fulbrightweb.org", false},
                                                            Globals::DeveloperID{u8"Iva Zuskov\xC3\xA1", "zuskova@natur.cuni.cz", false},
                                                            Globals::DeveloperID{u8"Bob Ga\xC5\xA1", "gas@natur.cuni.cz", false},
                                                            Globals::DeveloperID{u8"Pavel Dubsk\xC3\xBD", "pavel.dubsky@natur.cuni.cz", true},
                                                            Globals::DeveloperID{u8"Gabriel Gerlero", "gabrielgerlero@gmail.com", false},
                                                          };


QIcon Globals::icon()
{
  static const QPixmap PROGRAM_ICON{":/images/res/PeakMaster_icon.png"};

  if (PROGRAM_ICON.isNull())
    return QIcon{};

  return QIcon{PROGRAM_ICON};
}

QString Globals::DeveloperID::linkString() const
{
  return QString("%1 (<a href=\"mailto:%2\">%2</a>)").arg(name).arg(mail.toHtmlEscaped());
}

QString Globals::DeveloperID::prettyString() const
{
  return QString("%1 (%2)").arg(name).arg(mail);
}

QString Globals::VERSION_STRING()
{
  QString s = QString("%1 %2.%3%4").arg(SOFTWARE_NAME).arg(VERSION_MAJ).arg(VERSION_MIN).arg(VERSION_REV);
  s.append(" Build date: [" + QString(__DATE__) + " - " + QString(__TIME__)  + "]");

  return s;
}
