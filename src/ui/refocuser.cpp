#include "refocuser.h"

#include <QApplication>
#include <QTimer>

RefocuserWorker::RefocuserWorker() :
  m_focused{QPointer<QWidget>{qApp->focusWidget()}}
{
}

void RefocuserWorker::refocus()
{
  QWidget *target = [this]() {
    if (!m_focused.isNull())
      return m_focused.data();
    if (!m_focusedParent.isNull())
      return qobject_cast<QWidget *>(m_focusedParent.data());
    return static_cast<QWidget *>(nullptr);
  }();

  if (target != nullptr)
    target->setFocus();

  this->deleteLater();
}


Refocuser::Refocuser()
{
  m_worker = new RefocuserWorker{};
}

Refocuser::~Refocuser()
{
  QTimer::singleShot(0, m_worker, &RefocuserWorker::refocus);
}
