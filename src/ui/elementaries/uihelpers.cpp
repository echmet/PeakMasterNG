#include "uihelpers.h"

#include <QWindow>
#include <QScreen>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif // Q_OS_WIN

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

UIHelpers::WindowsTheme UIHelpers::detectWindowsTheme()
{
#ifdef Q_OS_WIN
  static const auto winTheme = []() {
    HMODULE hUxLib = LoadLibrary(TEXT("UxTheme.dll"));
    if (!hUxLib)
      return WindowsTheme::WINDOWS_CLASSIC;
    else {
      BOOL (WINAPI *pIsThemeActive)(VOID) = (BOOL (WINAPI *)(VOID))GetProcAddress(hUxLib, "IsThemeActive");
      BOOL (WINAPI *pIsAppThemed)(VOID) = (BOOL (WINAPI *)(VOID))GetProcAddress(hUxLib, "IsAppThemed");
      if (pIsThemeActive && pIsAppThemed) {
        BOOL active = pIsThemeActive();
        BOOL themed = pIsAppThemed();
        FreeLibrary(hUxLib);

        if (!(active && themed))
          return WindowsTheme::WINDOWS_CLASSIC;
      } else {
        FreeLibrary(hUxLib);
        return WindowsTheme::WINDOWS_AERO;
      }
    }

    HMODULE hDwmLib = LoadLibrary(TEXT("dwmapi.dll"));
    if (!hDwmLib)
      return WindowsTheme::WINDOWS_XP;  /* Cannot import dwmapi.dll, we are probably on WinXP */
    else {
      bool isBasic = false;
      HRESULT (WINAPI *pFn)(BOOL *enabled) = (HRESULT (WINAPI *)(BOOL *))GetProcAddress(hDwmLib, "DwmIsCompositionEnabled");
      if (pFn) {
        BOOL compositionEnabled;
        HRESULT ret = pFn(&compositionEnabled);

        isBasic = (SUCCEEDED(ret)) && (compositionEnabled == FALSE);
      }
      FreeLibrary(hDwmLib);
      return isBasic ? WindowsTheme::WINDOWS_BASIC : WindowsTheme::WINDOWS_AERO;
    }
  }();

  return winTheme;
#else
  return WindowsTheme::NOT_WINDOWS;
#endif // Q_OS_WIN
}
