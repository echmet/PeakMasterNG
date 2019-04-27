#ifndef SWSETTINGS_H
#define SWSETTINGS_H

#include <cassert>
#include <QSettings>

namespace persistence {

class SWSettings {
public:
  static const QString KEY_AUTOUPDATE_ENABLED; /* int: 0 = disabled, 1 = enabled */
  static const QString KEY_USER_DB_PATH;       /* QString: "" = No user database */

  static void destroy();
  static void initialize();

  template <typename T>
  static
  T get(const QString &key)
  {
    assert(s_me != nullptr);

    if (s_me->m_settings.contains(key)) {
      const auto &v = s_me->m_settings.value(key);

      if (v.canConvert<T>())
        return v.value<T>();
    }

    if (s_defaults.contains(key)) {
      const auto &v = s_defaults[key];
      if (v.canConvert<T>())
        return v.value<T>();
    }

    return {};
  }

  template <typename T>
  static
  void set(const QString &key, const T &value)
  {
    assert(s_me != nullptr);

    QVariant v = QVariant::fromValue<T>(value);

    assert(v.canConvert<T>());

    s_me->m_settings.setValue(key, v);
  }

private:
  SWSettings();

  QSettings m_settings;

  static QMap<QString, QVariant> s_defaults;
  static SWSettings *s_me;
};

} // namespace persistence

#endif // SWSETTINGS_H
