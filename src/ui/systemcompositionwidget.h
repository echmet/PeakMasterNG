#ifndef SYSTEMCOMPOSITIONWIDGET_H
#define SYSTEMCOMPOSITIONWIDGET_H

#include <QModelIndexList>
#include <QWidget>

namespace Ui {
  class SystemCompositionWidget;
}

class AbstractConstituentsModelBase;
class ComplexationColorizerDelegate;
class ComplexationManager;
class GDMProxy;
class FloatingValueDelegate;

class SystemCompositionWidget : public QWidget
{
  Q_OBJECT

public:
  explicit SystemCompositionWidget(GDMProxy &backgroundGDM, GDMProxy &sampleGDM, ComplexationManager &cpxMgr, QWidget *parent = nullptr);
  ~SystemCompositionWidget();

private:
  void addConstituent(GDMProxy &proxy, AbstractConstituentsModelBase *model);
  void editComplexation(const QString &name);
  void editConstituent(const QString &name, GDMProxy &proxy, AbstractConstituentsModelBase *model);
  void handleDoubleClick(const int column, const QVariant &v, GDMProxy &GDM, AbstractConstituentsModelBase *model);
  void removeConstituent(const QModelIndexList &indexes, GDMProxy &proxy, AbstractConstituentsModelBase *model);

  Ui::SystemCompositionWidget *ui;

  GDMProxy &h_backgroundGDM;
  GDMProxy &h_sampleGDM;
  ComplexationManager &h_cpxMgr;

  AbstractConstituentsModelBase *m_analytesModel;
  AbstractConstituentsModelBase *m_backgroundConstituentsModel;

  ComplexationColorizerDelegate *m_ccDelegateAnalytes;
  ComplexationColorizerDelegate *m_ccDelegateBGE;
  FloatingValueDelegate *m_fltDelegateAnalytes;
  FloatingValueDelegate *m_fltDelegateBGE;

private slots:
  void onAddAnalyte();
  void onAddBGE();
  void onAnalytesDoubleClicked(const QModelIndex &idx);
  void onBGEDoubleClicked(const QModelIndex &idx);
  void onCompositionChanged();
  void onRemoveAnalyte();
  void onRemoveBGE();

signals:
  void compositionChanged();
};

#endif // SYSTEMCOMPOSITIONWIDGET_H
