#ifndef REFOCUSER_H
#define REFOCUSER_H

#include <QObject>
#include <QPointer>
#include <QWidget>

class RefocuserWorker : public QObject {
  Q_OBJECT
public:
  RefocuserWorker();
  RefocuserWorker(const RefocuserWorker &) = delete;
  RefocuserWorker(RefocuserWorker &&) = delete;
  RefocuserWorker & operator=(const RefocuserWorker &) = delete;
  RefocuserWorker & operator=(RefocuserWorker &&) = delete;

public slots:
  void refocus();

private:
  QPointer<QWidget> m_focused;
  QPointer<QObject> m_focusedParent;
};

class Refocuser {
public:
  Refocuser();
  Refocuser(const Refocuser &) = delete;
  Refocuser(Refocuser &&) = delete;
  ~Refocuser();
  Refocuser & operator=(const Refocuser &) = delete;
  Refocuser & operator=(Refocuser &&) = delete;

private:
  RefocuserWorker *m_worker;
};

#endif // REFOCUSER_H
