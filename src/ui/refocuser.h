#ifndef REFOCUSER_H
#define REFOCUSER_H

#include <QObject>
#include <QWidget>

class RefocuserWorker : public QObject {
  Q_OBJECT
public:
  RefocuserWorker();

public slots:
  void schedule();

private:
  QWidget *m_focused;
  QObject *m_focusedParent;

private slots:
  void onFocusedDestroyed();
  void onFocusedParentDestroyed();
};

class Refocuser {
public:
  Refocuser();
  ~Refocuser();

private:
  RefocuserWorker *m_worker;
};

#endif // REFOCUSER_H
