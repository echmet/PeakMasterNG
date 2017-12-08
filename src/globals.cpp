#include "globals.h"

const QString Globals::CIMEC_WEB_LINK{"http://www.cimec.org.ar/twiki/bin/view/Cimec/"};
const QString Globals::ECHMET_WEB_LINK{"http://echmet.natur.cuni.cz"};
const QString Globals::ORG_NAME{"ECHMET"};
const QString Globals::SOFTWARE_NAME{"PeakMaster"};
const int Globals::VERSION_MAJ = 6;
const int Globals::VERSION_MIN = 0;
const QString Globals::VERSION_REV{"(alpha)"};

const QVector<Globals::DeveloperID> Globals::DEVELOPERS = { Globals::DeveloperID{"Michal Mal\xC3\xBD", "malymi@natur.cuni.cz", true} ,
                                                            Globals::DeveloperID{"Gabriel Gerlero", "gabrielgerlero@gmail.com", false},
                                                            Globals::DeveloperID{"Pavel Dubsk\xC3\xBD", "pavel.dubsky@natur.cuni.cz", true}
                                                          };


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
#ifdef UNSTABLE_VERSION
  s.append("-devel [" + QString(__DATE__) + " - " + QString(__TIME__)  + "]");
#endif

  return s;
}
