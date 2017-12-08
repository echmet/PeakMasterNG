#include "abstractconstituentsmodelbase.h"

#include "../../gearbox/complexationmanager.h"

#include <cassert>
#include <QDataStream>
#include <QMimeData>
#include <QPalette>

#include <QDebug>

const QString AbstractConstituentsModelBase::MIME_FORMAT{"application/vnd.text.list"};

AbstractConstituentsModelBase::AbstractConstituentsModelBase(GDMProxy &GDMProxy, ComplexationManager &cpxMgr, QObject *parent) :
  QAbstractTableModel{parent},
  h_cpxMgr{cpxMgr},
  h_GDMProxy{GDMProxy}
{
}

void AbstractConstituentsModelBase::addConstituent(const QString &name)
{
  const int idx = m_constituentNames.size() - 1;

  beginInsertRows(QModelIndex{}, idx, idx);
  m_constituentNames.append(name);
  endInsertRows();
}

QList<QColor> AbstractConstituentsModelBase::complexationStatus(const std::string &name) const
{
  const auto hues = h_cpxMgr.complexationStatus(name);

  if (hues.empty()) {
    QPalette p{};
    return { p.base().color() };
  }

  QList<QColor> _hues{};
  _hues.reserve(hues.size());

  for (const auto h : hues)
    _hues.append(QColor::fromHsv(h, 255, 255));

  return _hues;
}

bool AbstractConstituentsModelBase::dropMimeData(const QMimeData *mdata, Qt::DropAction action, int row, int col, const QModelIndex &parent)
{
  Q_UNUSED(row); Q_UNUSED(col);

  if (action == Qt::IgnoreAction)
    return true;

  if (!mdata->hasFormat(MIME_FORMAT))
    return false;

  QByteArray encodedData = mdata->data(MIME_FORMAT);
  QDataStream stream(&encodedData, QIODevice::ReadOnly);

  QString targetName = data(parent).toString();
  QString sourceName{};
  stream >> sourceName;

  h_cpxMgr.makeComplexation(sourceName.toStdString(), targetName.toStdString());

  return true;
}

QString AbstractConstituentsModelBase::removeConstituent(const int row)
{
  QString name{};

  if (row < 0 || row >= m_constituentNames.size())
    return {};

  beginRemoveRows(QModelIndex{}, row, row);
  name = m_constituentNames.takeAt(row);
  endRemoveRows();

  return name;
}

QMimeData * AbstractConstituentsModelBase::mimeData(const QModelIndexList &indexes) const
{
  QMimeData *mdata = new QMimeData{};
  QByteArray encodedData{};

  QDataStream stream{&encodedData, QIODevice::WriteOnly};

  for (int idx = 0; idx < 1; idx++) {
    const QModelIndex &midx = indexes.at(idx);

    if (midx.isValid()) {
      const QString name = data(midx).toString();
      stream << name;
    }
  }

  mdata->setData(MIME_FORMAT, encodedData);
  return mdata;
}

QStringList AbstractConstituentsModelBase::mimeTypes() const
{
  return { MIME_FORMAT };
}

int AbstractConstituentsModelBase::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return m_constituentNames.size();
}

void AbstractConstituentsModelBase::onComplexationStatusUpdated()
{
  if (m_constituentNames.size() < 1)
    return;

  emit dataChanged(index(0, 0), index(0, rowCount() - 1));
}

void AbstractConstituentsModelBase::updateName(const QString &oldName, const QString &newName)
{
  for (auto &n : m_constituentNames) {
    if (n == oldName) {
      n = newName;
      return;
    }
  }

  assert(false); /* We should never get here */
}
