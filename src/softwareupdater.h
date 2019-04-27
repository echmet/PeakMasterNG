#ifndef SOFTWAREUPDATER_H
#define SOFTWAREUPDATER_H

#include "softwareupdateresult.h"

#include <QObject>
#include <mutex>

class AutoUpdateCheckDialog;

class SoftwareUpdateWorker : public QObject {
  Q_OBJECT
public:
  SoftwareUpdateWorker(const QStringList &links, const bool automatic) :
    m_automatic(automatic),
    m_links(std::move(links))
  {}

public slots:
  void process();

signals:
  void checkComplete(bool automatic, const SoftwareUpdateResult &result);
  void finished();

private:
  const bool m_automatic;
  const QStringList m_links;
};


class SoftwareUpdater : public QObject
{
  Q_OBJECT
public:
  explicit SoftwareUpdater(QObject *parent = nullptr);
  ~SoftwareUpdater();

  void checkAutomatically();
  void checkForUpdate(const bool automatic);

private:
  void automaticCheckComplete(const SoftwareUpdateResult &result);

  AutoUpdateCheckDialog *m_autoDlg;

  std::mutex m_checkInProgressLock;
  bool m_checkInProgress;

  static const QStringList UPDATE_LINKS;
  static const QString CHECK_AUTOMATICALLY_SETTINGS_TAG;

signals:
  void checkComplete(const SoftwareUpdateResult &result);

public slots:
  void onCheckForUpdate();

private slots:
  void onUpdateCheckComplete(const bool automatic, const SoftwareUpdateResult &result);
};

#endif // SOFTWAREUPDATER_H
