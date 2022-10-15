#ifndef SYSTEMCOMPOSITIONWIDGET_H
#define SYSTEMCOMPOSITIONWIDGET_H

#include <QModelIndexList>
#include <QWidget>

namespace Ui {
  class SystemCompositionWidget;
}

class AbstractConstituentsModelBase;
class AnalytesConstituentsModel;
class AnalytesExtraInfoModel;
class BackgroundConstituentsModel;
class BackgroundEffectiveMobilitiesModel;
class ComplexationColorizerDelegate;
class ComplexationManager;
class CompositionEditorWidget;
class EditConstituentDialog;
class DatabaseProxy;
class GDMProxy;
class FloatingValueDelegate;

class SystemCompositionWidget : public QWidget
{
  Q_OBJECT

public:
  explicit SystemCompositionWidget(GDMProxy &backgroundGDM, GDMProxy &sampleGDM, ComplexationManager &cpxMgr, DatabaseProxy &dbProxy,
                                   const AnalytesExtraInfoModel * const analytesEXIModel,
                                   const BackgroundEffectiveMobilitiesModel * const BGEEffMobsModel,
                                   QWidget *parent = nullptr);
  ~SystemCompositionWidget();
  AbstractConstituentsModelBase * analytesModel() noexcept;
  AbstractConstituentsModelBase * backgroundModel() noexcept;
  void commit();
  void setViscosityCorrectionEnabled(const bool enabled);

private:
  bool addConstituent(GDMProxy &proxy, AbstractConstituentsModelBase *model);
  void editComplexation(const QString &name);
  void editConstituent(const QString &name, GDMProxy &proxy, AbstractConstituentsModelBase *model);
  void handleDoubleClick(const int column, const QVariant &v, GDMProxy &GDM, AbstractConstituentsModelBase *model);
  void removeConstituent(const QModelIndexList &indexes, GDMProxy &proxy, AbstractConstituentsModelBase *model);
  void setControlsIcons();

  Ui::SystemCompositionWidget *ui;

  GDMProxy &h_backgroundGDM;
  GDMProxy &h_sampleGDM;
  ComplexationManager &h_cpxMgr;
  DatabaseProxy &h_dbProxy;

  AnalytesConstituentsModel *m_analytesModel;
  BackgroundConstituentsModel *m_backgroundConstituentsModel;
  CompositionEditorWidget *m_background;
  CompositionEditorWidget *m_analytes;

  bool m_viscosityCorrectionEnabled;

private slots:
  void onAddAnalyte();
  void onAddBGE();
  void onAddToDatabase(const EditConstituentDialog *dlg);
  void onAnalytesDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
  void onBackgroundChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
  void onAnalytesEditRequested(const QModelIndex &idx);
  void onBGEEditRequested(const QModelIndex &idx);
  void onCompositionChanged();
  void onRemoveAnalyte(const QModelIndexList &indices);
  void onRemoveBGE(const QModelIndexList &indices);

signals:
  void compositionChanged();
};

#endif // SYSTEMCOMPOSITIONWIDGET_H
