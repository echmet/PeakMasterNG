#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <QWidget>

class QScreen;

class UIHelpers
{
public:
  UIHelpers() = delete;

  static QScreen * findScreenForWidget(const QWidget * widget);
};

#endif // UIHELPERS_H
