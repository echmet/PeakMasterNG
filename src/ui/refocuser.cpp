#include "refocuser.h"

#include <QApplication>
#include <QTimer>

RefocuserWorker::RefocuserWorker() :
  m_focused(qApp->focusWidget())
{
  if (m_focused != nullptr) {
    m_focusedParent = m_focused->parent();

    connect(m_focused, &QWidget::destroyed, this, &RefocuserWorker::onFocusedDestroyed);
    if (m_focusedParent != nullptr)
      connect(m_focusedParent, &QObject::destroyed, this, &RefocuserWorker::onFocusedParentDestroyed);
  }
}

void RefocuserWorker::refocus()
{
  QTimer::singleShot(0, nullptr, [this]() {
    QWidget *target = [this]() {
      if (m_focused != nullptr)
        return m_focused;
      return qobject_cast<QWidget *>(m_focusedParent);
    }();

    if (target != nullptr)
      target->setFocus();

    this->deleteLater();
  });
}

void RefocuserWorker::onFocusedDestroyed()
{
  m_focused = nullptr;
}

void RefocuserWorker::onFocusedParentDestroyed()
{
  m_focusedParent = nullptr;
  m_focused = nullptr;
}

Refocuser::Refocuser()
{
  m_worker = new RefocuserWorker{};
}

Refocuser::~Refocuser()
{
  QTimer::singleShot(0, m_worker, &RefocuserWorker::refocus);
}
