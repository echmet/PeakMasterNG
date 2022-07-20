#ifndef EXTRAINFOMODELBASE_H
#define EXTRAINFOMODELBASE_H


#include <QObject>
#include <QVector>

// ---
// DCL

namespace detail {

//https://forum.qt.io/topic/13692/template-class-not-supported-by-q_object/
class ExtraInfoModelBaseSignals : public QObject
{
  Q_OBJECT

protected:
  explicit ExtraInfoModelBaseSignals(QObject *parent = nullptr) : QObject(parent) {}

signals:
  void dataChanged();
};

}

template<class EXI>
class ExtraInfoModelBase : public detail::ExtraInfoModelBaseSignals
{
public:
  using ExtraInfo = EXI;

  ExtraInfoModelBase() = delete;
  explicit ExtraInfoModelBase(QObject *parent = nullptr);

  const ExtraInfo & operator[](const QString &analyte) const noexcept;
  void setData(QVector<ExtraInfo> newData);

private:

  QVector<ExtraInfo> m_data;

};

// ---
// DEF

template<class EXI>
ExtraInfoModelBase<EXI>::ExtraInfoModelBase(QObject *parent) :
  ExtraInfoModelBaseSignals{parent}
{
}

template<class EXI>
const EXI & ExtraInfoModelBase<EXI>::operator[](const QString &name) const noexcept
{
  static const EXI null("",{});

  for (const auto &item : m_data)
    if (item.name() == name) return item;

  return null;
}

template<class EXI>
void ExtraInfoModelBase<EXI>::setData(QVector<EXI> newData)
{
  m_data = std::move(newData);

  emit dataChanged();
}

#endif // EXTRAINFOMODELBASE_H
