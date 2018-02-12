#ifndef COPIABLEITEMSTABLEVIEW_H
#define COPIABLEITEMSTABLEVIEW_H

#include <QTableView>

class CopiableItemsTableView : public QTableView
{
public:
  explicit CopiableItemsTableView(QWidget *parent = nullptr);

protected:
  virtual void keyPressEvent(QKeyEvent *evt) override;
  virtual void mousePressEvent(QMouseEvent *evt) override;

private:
  void selectionToClipboard(const QModelIndexList &indexes);
  QMenu *m_rightClickMenu;

private slots:
  void onCopyActionTriggered();
};

#endif // COPIABLEITEMSTABLEVIEW_H
