#include "crashhandlerdialog.h"
#include "ui_crashhandlerdialog.h"
#include "../globals.h"
#include <QSysInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <thread>

const QString CrashHandlerDialog::s_reportToDevsCaption{QObject::tr("Report to developers")};
const QString CrashHandlerDialog::s_dialogCaptionDuring{"Well, I have been stiff and smiling 'till I crashed..."};
const QString CrashHandlerDialog::s_dialogCaptionPostCrash{"Well, I have been stiff and smiling 'till I crashed..."};

CrashHandlerDialog::CrashHandlerDialog(const bool postCrash, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::CrashHandlerDialog},
  m_apologyMessagePartOneDuring{QString(QObject::tr("We are sorry, but %1 has encountered an internal error from which it cannot recover.")).arg(Globals::SOFTWARE_NAME())},
  m_apologyMessagePartOnePostCrash{QString(QObject::tr("We are sorry, but it seems that %1 crashed last time it was run.")).arg(Globals::SOFTWARE_NAME())},
  m_apologyMessagePartTwo{QObject::tr("\n"
                                      "You may want to report the backtrace below along with a description (in English or Czech) "
                                      "of what exactly happened to the developers.")}
{
  ui->setupUi(this);
  setWindowTitle(QObject::tr("Crash handler"));
  if (postCrash) {
    ui->ql_fallApart->setText(s_dialogCaptionPostCrash);
    m_apologyMessage = m_apologyMessagePartOnePostCrash;
  } else {
    ui->ql_fallApart->setText(s_dialogCaptionDuring);
    m_apologyMessage = m_apologyMessagePartOneDuring;
  }

  m_apologyMessage += m_apologyMessagePartTwo;

  connect(ui->qpb_ok, &QPushButton::clicked, this, &CrashHandlerDialog::onOkClicked);
  connect(ui->qpb_reportToDevelopers, &QPushButton::clicked, this, &CrashHandlerDialog::onReportToDevelopersClicked);
}

CrashHandlerDialog::~CrashHandlerDialog()
{
  delete ui;
}

void CrashHandlerDialog::onOkClicked()
{
  accept();
}

void CrashHandlerDialog::onReportToDevelopersClicked()
{
  std::thread t([&]() {
    QDesktopServices::openUrl(QUrl(m_mailToDevelopers));
  });
  t.detach();
}

void CrashHandlerDialog::setBacktrace(const QString &backtrace)
{
  QString mails{};

  for (const auto &dev : Globals::DEVELOPERS) {
    if (dev.reportBugs) {
      if (!mails.isEmpty())
        mails.append(",");
      mails.append(QString("%1").arg(dev.mail.toHtmlEscaped()));
    }
  }

  auto debugInfo = QString("Version: %1\n\nBacktrace:\n%2").arg(Globals::VERSION_STRING()).arg(backtrace);
  m_mailToDevelopers = QString("mailto:%1?subject=%2&body=%3")
                               .arg(mails)
                               .arg(QString("%1 (%2 %3) crash report").arg(Globals::SOFTWARE_NAME()).arg(QGuiApplication::platformName()).arg(QSysInfo::buildCpuArchitecture()))
                               .arg(debugInfo.toHtmlEscaped());

  ui->ql_message->setText(m_apologyMessage);
  ui->qte_backtrace->setText(backtrace);
}
