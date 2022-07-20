#ifndef EXTRAINFOBASE_H
#define EXTRAINFOBASE_H

#include "../../globals.h"

#include <utility>

#include <QString>
#include <QVariant>
#include <QVector>

// ---
// DCL

template<class CRTPINFO>
class ExtraInfoBase
{
public:
  // ! CRTPINFO shall declare its own enum
  // The first item value shall equal _CUSTOM_BEGIN
  // and the last (one-past-the-end) item shall read _CUSTOM_END
  enum {
    U_EFF = 0,
    U_EFF_OVER_KAPPA_BGE,
    _CUSTOM_BEGIN
  };

  struct Initializer {
    Initializer() = default;
    Initializer(const Initializer &) = delete;
    Initializer(Initializer &&) = default;

    QVariant uEff;
    QVariant kappaBGE;

  private:
    // preventing brace-initialization
    Initializer(QVariant, QVariant);
  };

  ExtraInfoBase() = delete;
  ExtraInfoBase(const ExtraInfoBase &) = default;
  ExtraInfoBase(ExtraInfoBase &&) = default;

  ExtraInfoBase(QString name, Initializer &&data);

  ExtraInfoBase & operator=(const ExtraInfoBase &) = default;
  ExtraInfoBase & operator=(ExtraInfoBase &&) = default;

  static const QString &header(int index) noexcept;
  static int size() noexcept;

  const QString &name() const noexcept;
  QVariant data(int index) const noexcept;

  // ! CRTPINFO shall declared this function
  // index is guaranteed to be within <_CUSTOM_BEGIN, _CUSTOM_END)
  // static const QString &customHeader(int index) noexcept;

protected:
  // ! CRTPINFO shall use this function
  // to populate data corresponding to
  // its enum items in the ctor
  //
  // The r-value reference is chosen so that the caller
  // does not forget to move the value (which is highly intended here)
  void set(int index, QVariant &&data);

  // Shifts the index so that
  // _CUSTOM_BEGIN == 0
  static int base(int index) noexcept;

private:
  QString m_name;
  QVector<QVariant> m_data;

  static const QString &callCustomHeader(int index) noexcept;

};

// ---
// DEF

template<class CRTPINFO>
ExtraInfoBase<CRTPINFO>::ExtraInfoBase(QString name, Initializer &&data)
  : m_name{name}
{
  m_data.reserve(CRTPINFO::_CUSTOM_END);

  m_data.push_back(data.uEff);
  if (data.uEff.isNull() || data.kappaBGE.isNull() || data.kappaBGE.toDouble() == 0) m_data.push_back({});
  else m_data.push_back({(data.uEff.toDouble() / data.kappaBGE.toDouble()) * 1E-9});

  assert(m_data.size() == _CUSTOM_BEGIN);
  m_data.resize(CRTPINFO::_CUSTOM_END);
}

template<class CRTPINFO>
const QString &ExtraInfoBase<CRTPINFO>::name() const noexcept
{
  return m_name;
}

template<class CRTPINFO>
const QString &ExtraInfoBase<CRTPINFO>::header(int index) noexcept
{
  static const QString null;
  static QVector<QString> strings;
  if (strings.empty())
  {
    strings.resize(_CUSTOM_BEGIN);
    strings[U_EFF] = Globals::isZombieOS() ? QObject::tr("u Eff (. 1e-9)") : QObject::tr("\xCE\xBC Eff (\xE2\x8B\x85 1e-9)");
    strings[U_EFF_OVER_KAPPA_BGE] = Globals::isZombieOS() ? QObject::tr("u Eff / kappa BGE (1/A)") : QObject::tr("\xCE\xBC Eff/\xCE\xBA BGE (A\xE2\x81\xBB\xC2\xB9)");
  }

  if (index < 0 || index >= CRTPINFO::_CUSTOM_END) return null;

  return (index < _CUSTOM_BEGIN ? strings[index] : callCustomHeader(index));
}

template<class CRTPINFO>
int ExtraInfoBase<CRTPINFO>::size() noexcept
{
  return CRTPINFO::_CUSTOM_END;
}

template<class CRTPINFO>
QVariant ExtraInfoBase<CRTPINFO>::data(int index) const noexcept
{
  if (index < 0 || index >= CRTPINFO::_CUSTOM_END) return {};

  return m_data[index];
}

template<class CRTPINFO>
void ExtraInfoBase<CRTPINFO>::set(int index, QVariant &&data)
{
  assert(index >= 0 && index < m_data.size());

  m_data[index] = std::move(data);
}

template<class CRTPINFO>
int ExtraInfoBase<CRTPINFO>::base(int index) noexcept
{
  return index - _CUSTOM_BEGIN;
}

template<class CRTPINFO>
const QString &ExtraInfoBase<CRTPINFO>::callCustomHeader(int index) noexcept
{
  return CRTPINFO::customHeader(index);
}


#endif // EXTRAINFOBASE_H
