#ifndef RETURNACCEPTABLETABLEVIEW_H
#define RETURNACCEPTABLETABLEVIEW_H

#include "copiableitemstableview.h"

class ReturnAcceptableTableView : public CopiableItemsTableView
{
  Q_OBJECT
public:
  explicit ReturnAcceptableTableView(QWidget *parent = nullptr);

protected:
  virtual void keyPressEvent(QKeyEvent *evt) override;

signals:
  void returnPressed();
};

#endif // RETURNACCEPTABLETABLEVIEW_H
