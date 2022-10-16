// vim: sw=2 ts=2 sts=2 expandtab

#ifndef COMPOSITIONEDITORWIDGET_H
#define COMPOSITIONEDITORWIDGET_H

#include "ui_compositioneditorwidget.h"

#include <QModelIndexList>
#include <QWidget>

class AbstractConstituentsModelBase;
class FloatingValueDelegate;

class CompositionEditorWidget : public QWidget {
  Q_OBJECT
public:
  CompositionEditorWidget(const QString &title, const QString &constituentType, AbstractConstituentsModelBase *model, QWidget *parent = nullptr);
  ~CompositionEditorWidget();
  void forceCommit();
  void resizeHeader();

protected:
  Ui::CompositionEditorWidget *ui;

  FloatingValueDelegate *m_fltDelegate;

private:
  void setupIcons();

signals:
  void addConstituent();
  void editRequested(const QModelIndex &idx);
  void removeConstituent(const QModelIndexList &indices);
};

#endif // COMPOSITIONEDITORWIDGET_H
