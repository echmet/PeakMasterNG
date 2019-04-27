#ifndef SWSETTINGS_H
#define SWSETTINGS_H

#include <cassert>
#include <QSettings>

namespace persistence {

template <typename T>
class SWSItem {
public:
  const QString tag;
  const T defaultVal;

  friend class SWSettings;

private:
  SWSItem(const QString & _tag, T _defaultVal) :
    tag{_tag},
    defaultVal{std::move(_defaultVal)}
  {}
};

class SWSettings {
public:
  static const SWSItem<int> KEY_AUTOUPDATE_ENABLED; /* 0 = disabled, 1 = enabled */
  static const SWSItem<QString> KEY_USER_DB_PATH;   /* "" = No user database */

  static void destroy();
  static void initialize();

  template <typename T>
  static
  T get(const SWSItem<T> &key)
  {
    assert(s_me != nullptr);

    const QString &tag = key.tag;

    if (s_me->m_settings.contains(tag)) {
      const auto &v = s_me->m_settings.value(tag);

      if (v.template canConvert<T>())
        return v.template value<T>();
    }

    return key.defaultVal;
  }

  template <typename T>
  static
  void set(const SWSItem<T> &key, const T &value)
  {
    assert(s_me != nullptr);

    QVariant v = QVariant::fromValue<T>(value);

    assert(v.canConvert<T>());

    s_me->m_settings.setValue(key.tag, v);
  }

private:
  SWSettings();

  QSettings m_settings;

  static SWSettings *s_me;
};

} // namespace persistence

#endif // SWSETTINGS_H
