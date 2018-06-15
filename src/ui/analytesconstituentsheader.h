#ifndef ANALYTESCONSTITUENTSHEADER_H
#define ANALYTESCONSTITUENTSHEADER_H

#include <QHeaderView>

class AnalytesConstituentsHeader : public QHeaderView
{
public:
  AnalytesConstituentsHeader(Qt::Orientation orientation, QWidget *parent = nullptr);

  virtual int sizeHintForColumn(int column) const override;
};

#endif // ANALYTESCONSTITUENTSHEADER_H
