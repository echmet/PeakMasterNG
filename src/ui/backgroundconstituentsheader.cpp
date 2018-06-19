#include "backgroundconstituentsheader.h"

BackgroundConstituentsHeader::BackgroundConstituentsHeader(Qt::Orientation orientation, QWidget *parent) :
  QHeaderView{orientation, parent}
{
}

int BackgroundConstituentsHeader::sizeHintForColumn(int column) const
{
  if (model() == nullptr)
    return - 1;

  const QString text = model()->headerData(column, Qt::Horizontal).toString();
  const QFontMetrics qfm = this->fontMetrics();
  const int spacers = qfm.width("_") * 4;

  return qfm.width(text) + spacers;
}
