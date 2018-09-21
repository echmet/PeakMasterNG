#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <QWidget>

class QScreen;

class UIHelpers
{
public:
  enum class WindowsTheme {
    NOT_WINDOWS,
    WINDOWS_CLASSIC,
    WINDOWS_XP,
    WINDOWS_BASIC,
    WINDOWS_AERO
  };

  UIHelpers() = delete;

  static QScreen * findScreenForWidget(const QWidget * widget);
  static WindowsTheme detectWindowsTheme();
};

#endif // UIHELPERS_H
