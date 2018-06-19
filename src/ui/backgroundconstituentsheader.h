#ifndef BACKGROUNDCONSTITUENTSHEADER_H
#define BACKGROUNDCONSTITUENTSHEADER_H

#include <QHeaderView>

class BackgroundConstituentsHeader : public QHeaderView
{
public:
  BackgroundConstituentsHeader(Qt::Orientation orientation, QWidget *parent = nullptr);

  virtual int sizeHintForColumn(int column) const override;
};

#endif // BACKGROUNDCONSTITUENTSHEADER_H
