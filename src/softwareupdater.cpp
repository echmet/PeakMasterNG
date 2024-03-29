#include "softwareupdater.h"
#include "globals.h"
#include "ui/autoupdatecheckdialog.h"
#include "persistence/swsettings.h"

#include <QVariant>
#include <QThread>
#include <echmetupdatecheck.h>

#include <cstring>

const QStringList SoftwareUpdater::UPDATE_LINKS{"https://echmet.natur.cuni.cz/echmet/download/public/eupd_manifest.json",
                                                "https://devoid-pointer.net/echmet/eupd_manifest.json"};
const QString SoftwareUpdater::CHECK_AUTOMATICALLY_SETTINGS_TAG("CheckAutomatically");

template <size_t N>
static
void cpyrev(char dst[N], const char *src)
{
  // Some very strict compilers will not like the use of strncpy so we just provide this function
  std::memset(dst, 0, N);

  const size_t srclen = std::strlen(src);
  size_t idx = 0;
  while (idx < srclen && idx < N) {
    dst[idx] = src[idx];
    idx++;
  }
}

void SoftwareUpdateWorker::process()
{
  EUPDResult res;

  EUPDInSoftware inSw;
  strncpy(inSw.name, SOFTWARE_NAME_INTERNAL_S, sizeof(EUPDInSoftware::name));
  inSw.version.major = Globals::VERSION_MAJ;
  inSw.version.minor = Globals::VERSION_MIN;
  cpyrev<sizeof(EUPDVersion::revision)>(inSw.version.revision, Globals::VERSION_REV().toLatin1().data());

  int linkIdx = 0;
  EUPDRetCode ret;
  do {
    ret = updater_check(m_links[linkIdx].toLatin1(), &inSw, &res, 0);
  } while (EUPD_IS_NETWORK_ERROR(ret) && linkIdx++ < m_links.size() - 1);

  if (EUPD_IS_ERROR(ret)) {
    QString err(updater_error_to_str(ret));

    if (EUPD_IS_NETWORK_ERROR(ret))
      emit checkComplete(m_automatic, SoftwareUpdateResult(SoftwareUpdateResult::State::NETWORK_ERROR, std::move(err)));
    else
      emit checkComplete(m_automatic, SoftwareUpdateResult(SoftwareUpdateResult::State::CHECK_ERROR, std::move(err)));
  } else {
    if (res.status == EUST_UNKNOWN) {
      QString err(updater_error_to_str(ret));

      emit checkComplete(m_automatic, SoftwareUpdateResult(SoftwareUpdateResult::State::NO_DATA, std::move(err)));
    } else if (res.status == EUST_UP_TO_DATE) {
      emit checkComplete(m_automatic, SoftwareUpdateResult());
    } else {
      try {
        auto s = [](const EUPDUpdateStatus status) {
                    switch (status) {
                    case EUST_UPDATE_AVAILABLE:
                      return SoftwareUpdateResult::Severity::AVAILABLE;
                    case EUST_UPDATE_RECOMMENDED:
                      return SoftwareUpdateResult::Severity::RECOMMENDED;
                    case EUST_UPDATE_REQUIRED:
                      return SoftwareUpdateResult::Severity::REQUIRED;
                    default:
                      throw std::runtime_error("Invalid update state");
                    }
        }(res.status);

        emit checkComplete(m_automatic,
                           SoftwareUpdateResult(
                             res.version.major,
                             res.version.minor,
                             [](const char rev[4]) {
                               if (rev[3] != '\0')
                                 return QString::fromLatin1(rev, 4);
                               return QString::fromLatin1(rev);
                             }(res.version.revision),
                             s,
                             QString::fromUtf8(res.link)
                           )
                          );
      } catch (const std::runtime_error &) {
        emit checkComplete(m_automatic, SoftwareUpdateResult(SoftwareUpdateResult::State::CHECK_ERROR, "Invalid update state value"));
      }
    }

    updater_free_result(&res);

    emit finished();
  }
}

SoftwareUpdater::SoftwareUpdater(QObject *parent) : QObject(parent),
  m_checkInProgress(false)
{
  m_autoDlg = new AutoUpdateCheckDialog();
}

SoftwareUpdater::~SoftwareUpdater()
{
  delete m_autoDlg;
}
void SoftwareUpdater::checkAutomatically()
{
  if (persistence::SWSettings::get<int>(persistence::SWSettings::KEY_AUTOUPDATE_ENABLED) < 1)
    return;

  checkForUpdate(true);
}

void SoftwareUpdater::checkForUpdate(const bool automatic)
{
#ifdef UNSTABLE_VERSION
  emit checkComplete(SoftwareUpdateResult(SoftwareUpdateResult::State::DISABLED,
                                          QString()));
  return;
#endif // UNSTABLE_VERSION

  std::lock_guard<std::mutex> lk(m_checkInProgressLock);
  if (m_checkInProgress)
    return;
  m_checkInProgress = true;

  auto thr = new QThread();
  auto worker = new SoftwareUpdateWorker(UPDATE_LINKS, automatic);

  worker->moveToThread(thr);

  connect(thr, &QThread::started, worker, &SoftwareUpdateWorker::process);
  connect(worker, &SoftwareUpdateWorker::checkComplete, this, &SoftwareUpdater::onUpdateCheckComplete);
  connect(worker, &SoftwareUpdateWorker::finished, thr, &QThread::quit);
  connect(worker, &SoftwareUpdateWorker::finished, worker, &SoftwareUpdateWorker::deleteLater);
  connect(thr, &QThread::finished, thr, &QThread::deleteLater);

  thr->start();
}

void SoftwareUpdater::automaticCheckComplete(const SoftwareUpdateResult &result)
{
  if (!result.updateAvailable)
    return;

  m_autoDlg->setDisplay(result);
  m_autoDlg->exec();
}

void SoftwareUpdater::onCheckForUpdate()
{
  checkForUpdate(false);
}

void SoftwareUpdater::onUpdateCheckComplete(const bool automatic, const SoftwareUpdateResult &result)
{
  std::lock_guard<std::mutex> lk(m_checkInProgressLock);
  m_checkInProgress = false;

  if (automatic)
    automaticCheckComplete(result);
  else
    emit checkComplete(result);
}
