#ifndef FLOATINGVALUEDELEGATE_H
#define FLOATINGVALUEDELEGATE_H

#include <QLocale>
#include <QItemDelegate>

class FloatingValueDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  explicit FloatingValueDelegate(bool fixedPrecision, QObject *parent = nullptr);

  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  void forceCommit();
  virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
  virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
  bool m_fixedPrecision;

private slots:
  void onTextChanged(const QString &);

signals:
  void editorCommit();

};

#endif // FLOATINGVALUEDELEGATE_H
