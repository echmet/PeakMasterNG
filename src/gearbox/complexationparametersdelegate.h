#ifndef COMPLEXATIONPARAMETERSDELEGATE_H
#define COMPLEXATIONPARAMETERSDELEGATE_H

#include <QItemDelegate>

class ComplexationParametersDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  explicit ComplexationParametersDelegate(QObject *parent = nullptr);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  void forceCommit();
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private slots:
  void onTextChanged(const QString &);

};

#endif // COMPLEXATIONPARAMETERSDELEGATE_H
