#include "analytesconstituentsheader.h"

#include <QFontMetrics>
#include <QPalette>

AnalytesConstituentsHeader::AnalytesConstituentsHeader(Qt::Orientation orientation, QWidget *parent) :
  QHeaderView{orientation, parent}
{}

int AnalytesConstituentsHeader::sizeHintForColumn(int column) const
{
  if (model() == nullptr)
    return -1;

  const QString text = model()->headerData(column, Qt::Horizontal).toString();
  const QFontMetrics qfm = this->fontMetrics();
  const int spacers = qfm.width("_") * 4;

  return qfm.width(text) + spacers;
}
