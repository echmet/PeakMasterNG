#include "uihelpers.h"

#include <QWindow>
#include <QScreen>

QScreen * UIHelpers::findScreenForWidget(const QWidget *widget)
{
  for (;;) {
    QWindow *wh = widget->window()->windowHandle();
    if (wh != nullptr) {
      QScreen *scr = wh->screen();
      if (scr != nullptr)
        return scr;
    }

    widget = qobject_cast<const QWidget *>(widget->parent());
    if (widget == nullptr)
      return nullptr;
  }
}
