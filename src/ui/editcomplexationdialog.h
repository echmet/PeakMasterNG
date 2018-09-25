#ifndef EDITCOMPLEXATIONDIALOG_H
#define EDITCOMPLEXATIONDIALOG_H

#include <QDialog>
#include <QMap>

#include "complexation_models/complexationrelationshipsmodel.h"

namespace Ui {
  class EditComplexationDialog;
}

class QAbstractItemModel;
class ComplexationParametersDelegate;

class EditComplexationDialog : public QDialog
{
  Q_OBJECT

public:
  typedef QMap<QString, ComplexationRelationship::RelationshipsMap> AllComplexationRelationships;
  explicit EditComplexationDialog(QWidget *parent = nullptr);
  ~EditComplexationDialog();
  void addRelationship(const ComplexationRelationship &relationship);
  AllComplexationRelationships allRelationships() const;
  void setComplexationModel(QAbstractItemModel *model);
  void showLigand(const QString &name);

private:
  ComplexationRelationship::RelationshipsMap relationshipsForTree(const std::shared_ptr<ComplexationRelationshipsModel::RootTreeItem> &root) const;
  Ui::EditComplexationDialog *ui;

  ComplexationRelationshipsModel *_crModel;
  QMap<QString, QPair<std::shared_ptr<ComplexationRelationshipsModel::RootTreeItem>, ComplexationRelationship>> _relationships;
  ComplexationParametersDelegate *_delegate;

  static QSize m_lastDlgSize;

private slots:
  void onAcceptClicked();
  void onLigandChanged(const int idx);
  void onRejectClicked();
  void onRemoveLigandClicked();
};

#endif // EDITCOMPLEXATIONDIALOG_H
