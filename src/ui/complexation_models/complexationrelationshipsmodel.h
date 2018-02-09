#ifndef UI_COMPLEXATIONRELATIONSHIPSMODEL_H
#define UI_COMPLEXATIONRELATIONSHIPSMODEL_H

#include "constituent_ui.h"
#include <QAbstractItemModel>
#include <memory>

class ComplexationRelationshipsModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum class ItemType {
    DEFAULT,
    PARAMETER_LIST
  };

  class NucleusTreeItem;
  class LigandTreeItem;

  class TreeItem {
  public:
    enum class Type {
      INVALID,
      ROOT,
      NUCLEUS,
      LIGAND,
    };

    class InvalidChildException :public std::exception {
    public:
      const char * what() const noexcept;
    };

    virtual int childIndex(const TreeItem *child) const = 0;
    virtual int childrenCount() const = 0;
    virtual TreeItem *childAt(const int idx) = 0;
    TreeItem & operator=(const TreeItem &other);

    const Type itype;
    TreeItem * const parent;

  protected:
    explicit TreeItem(TreeItem *parent, const Type itype);

  };

  class RootTreeItem : public TreeItem {
  public:
    explicit RootTreeItem();
    bool addNucleusForm(const int charge, const ComplexationRelationship &relationship);
    TreeItem * childAt(const int idx) override;
    int childIndex(const TreeItem *child) const override;
    int childrenCount() const override;

  private:
    QList<std::shared_ptr<NucleusTreeItem>> _children;

  };

  class NucleusTreeItem : public TreeItem {
  public:
    explicit NucleusTreeItem(RootTreeItem *parent, const int charge);
    bool addLigandCharge(const int charge, const ComplexationRelationship::ComplexFormProperties &props);
    TreeItem * childAt(const int idx) override;
    int childIndex(const TreeItem *child) const override;
    int childrenCount() const override;

    const int charge;
  private:
    QList<std::shared_ptr<LigandTreeItem>> _children;

  };

  class LigandTreeItem : public TreeItem {
  public:
    explicit LigandTreeItem(NucleusTreeItem *parent, const int charge);
    TreeItem * childAt(const int idx) override;
    int childIndex(const TreeItem *child) const override;
    int childrenCount() const override;

    const int charge;
    int count;
    QVector<double> mobilities;
    QVector<double> pBs;

  };

  ComplexationRelationshipsModel(QObject *parent = nullptr);

  std::shared_ptr<RootTreeItem> buildTree(const ComplexationRelationship &relationship);
  void clear();
  int columnCount(const QModelIndex &parent) const;
  const std::shared_ptr<const RootTreeItem> currentRoot() const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &child) const;
  int rowCount(const QModelIndex &parent) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  void setRoot(std::shared_ptr<RootTreeItem> root);

  static const QString ELEMENTS_SPLITTER;

private:
  QVariant makeItemData(const TreeItem *item, const int column) const;
  TreeItem * modelIndexToItem(const QModelIndex &index) const;

  std::shared_ptr<RootTreeItem> _currentRoot;

};

Q_DECLARE_METATYPE(ComplexationRelationshipsModel::ItemType)

#endif // UI_COMPLEXATIONRELATIONSHIPSMODEL_H
