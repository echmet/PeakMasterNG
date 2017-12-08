#include "complexationrelationshipsmodel.h"
#include "../../gearbox/doubletostringconvertor.h"

const char * ComplexationRelationshipsModel::TreeItem::InvalidChildException::what() const noexcept
{
  return "Invalid child item";
}

ComplexationRelationshipsModel::TreeItem::TreeItem(TreeItem *parent, const Type itype) :
  itype(itype),
  parent(parent)
{
}

ComplexationRelationshipsModel::RootTreeItem::RootTreeItem() :
  TreeItem(nullptr, Type::ROOT)
{
}

ComplexationRelationshipsModel::TreeItem & ComplexationRelationshipsModel::TreeItem::operator=(const TreeItem &other)
{
  const_cast<Type&>(itype) = other.itype;
  const_cast<TreeItem *&>(parent) = other.parent;

  return *this;
}

bool ComplexationRelationshipsModel::RootTreeItem::addNucleusForm(const int charge, const ComplexationRelationship &relationship)
{
  try {
    const auto ligand = relationship.ligand();
    auto f = std::make_shared<NucleusTreeItem>(this, charge);

    for (int ligandCharge = ligand->lowCharge; ligandCharge <= ligand->highCharge; ligandCharge++) {
      if (!f->addLigandCharge(ligandCharge, relationship.properties(charge, ligandCharge)))
        return false;
    }

    _children.push_back(f);
  } catch (std::bad_alloc &) {
    return false;
  }

  return true;
}

int ComplexationRelationshipsModel::RootTreeItem::childIndex(const TreeItem *child) const
{
  if (child->itype != Type::LIGAND)
    throw InvalidChildException();

  for (int idx = 0; idx < _children.size(); idx++) {
    if (_children.at(idx).get() == child)
      return idx;
  }

  throw InvalidChildException();
}

ComplexationRelationshipsModel::TreeItem * ComplexationRelationshipsModel::RootTreeItem::childAt(const int idx)
{
  if (idx < 0 || idx >= _children.size())
    return nullptr;

  return _children.at(idx).get();
}

int ComplexationRelationshipsModel::RootTreeItem::childrenCount() const
{
  return _children.size();
}

ComplexationRelationshipsModel::NucleusTreeItem::NucleusTreeItem(RootTreeItem *parent, const int charge) :
  TreeItem(parent, Type::NUCLEUS),
  charge(charge)
{
}

bool ComplexationRelationshipsModel::NucleusTreeItem::addLigandCharge(const int charge, const ComplexationRelationship::ComplexFormProperties &props)
{
  try {
    auto f = std::make_shared<LigandTreeItem>(this, charge);
    f->mobilities = std::get<0>(props);
    f->pBs = std::get<1>(props);

    _children.push_back(f);
  } catch (std::bad_alloc &) {
    return false;
  }

  return true;
}

ComplexationRelationshipsModel::TreeItem * ComplexationRelationshipsModel::NucleusTreeItem::childAt(const int idx)
{
  if (idx < 0 || idx >= _children.size())
    return nullptr;

  return _children.at(idx).get();
}

int ComplexationRelationshipsModel::NucleusTreeItem::childIndex(const TreeItem *child) const
{
  if (child->itype != Type::LIGAND)
    throw InvalidChildException();

  for (int idx = 0; idx < _children.size(); idx++) {
    if (_children.at(idx).get() == child)
      return idx;
  }

  throw InvalidChildException();
}

int ComplexationRelationshipsModel::NucleusTreeItem::childrenCount() const
{
  return _children.size();
}

ComplexationRelationshipsModel::LigandTreeItem::LigandTreeItem(NucleusTreeItem *parent, const int charge) :
  TreeItem(parent, Type::LIGAND),
  charge(charge),
  count(0)
{
}

ComplexationRelationshipsModel::TreeItem * ComplexationRelationshipsModel::LigandTreeItem::childAt(const int idx)
{
  Q_UNUSED(idx);

  return nullptr;
}

int ComplexationRelationshipsModel::LigandTreeItem::childIndex(const TreeItem *child) const
{
  Q_UNUSED(child);

  throw InvalidChildException();
}

int ComplexationRelationshipsModel::LigandTreeItem::childrenCount() const
{
  return 0;
}

ComplexationRelationshipsModel::ComplexationRelationshipsModel(QObject *parent) :
  QAbstractItemModel(parent),
  _currentRoot(std::make_shared<RootTreeItem>())
{
}

std::shared_ptr<ComplexationRelationshipsModel::RootTreeItem> ComplexationRelationshipsModel::buildTree(const ComplexationRelationship &relationship)
{
  auto root = std::make_shared<RootTreeItem>();

  for (int charge = relationship.nucleus()->lowCharge; charge <= relationship.nucleus()->highCharge; charge++) {
    if (!root->addNucleusForm(charge, relationship))
      return nullptr;
  }

  return root;
}

int ComplexationRelationshipsModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return 4;
}

void ComplexationRelationshipsModel::clear()
{
  beginResetModel();
  _currentRoot = std::make_shared<RootTreeItem>();
  endResetModel();
}

const std::shared_ptr<const ComplexationRelationshipsModel::RootTreeItem> ComplexationRelationshipsModel::currentRoot() const
{
  return _currentRoot;
}

QVariant ComplexationRelationshipsModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  TreeItem *item = modelIndexToItem(index);

  return makeItemData(item, index.column());
}

Qt::ItemFlags ComplexationRelationshipsModel::flags(const QModelIndex &index) const
{
  TreeItem *item = modelIndexToItem(index);

  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  if (item->itype != TreeItem::Type::LIGAND)
    return flags;

  flags |= Qt::ItemNeverHasChildren;

  if (index.column() == 2 || index.column() == 3)
    flags |= Qt::ItemIsEditable;

  return flags;
}

QVariant ComplexationRelationshipsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal)
    return QVariant();

  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  switch (section) {
  case 0:
    return "Nucleus charge";
  case 1:
    return "Ligand charge";
  case 2:
    return "Mobilities";
  case 3:
    return "pBs";
  default:
    return QVariant();
  }
}

QModelIndex ComplexationRelationshipsModel::index(int row, int column, const QModelIndex &parent) const
{
  TreeItem *item = modelIndexToItem(parent);
  TreeItem *childItem = item->childAt(row);

  if (childItem == nullptr)
    return QModelIndex();

  return createIndex(row, column, childItem);
}

QVariant ComplexationRelationshipsModel::makeItemData(const TreeItem *item, const int column) const
{
  auto itemsToString = [](const QVector<double> &vec) -> QVariant {
    QString out;

    for (const double d : vec) {
      QString s = DoubleToStringConvertor::convert(d);
      out += s + QString(" ");
    }

    return out;
  };

  switch (item->itype) {
  case TreeItem::Type::ROOT:
    return QVariant();
  case TreeItem::Type::NUCLEUS:
    if (column == 0)
      return static_cast<const NucleusTreeItem *>(item)->charge;

    return QVariant();
  case TreeItem::Type::LIGAND:
    {
    const LigandTreeItem *ligandItem = static_cast<const LigandTreeItem *>(item);
    switch (column) {
    case 1:
      return ligandItem->charge;
    case 2:
      return itemsToString(ligandItem->mobilities);
    case 3:
      return itemsToString(ligandItem->pBs);
    default:
      return QVariant();
    }
    }
  default:
    return QVariant();
  }
}

ComplexationRelationshipsModel::TreeItem * ComplexationRelationshipsModel::modelIndexToItem(const QModelIndex &index) const
{
  if (!index.isValid() && index.column() != 0)
    return _currentRoot.get();

  void *item = index.internalPointer();
  if (item != nullptr)
    return static_cast<TreeItem *>(item);

  return _currentRoot.get();
}

QModelIndex ComplexationRelationshipsModel::parent(const QModelIndex &child) const
{
  if (!child.isValid())
    return QModelIndex();

  TreeItem *childItem = modelIndexToItem(child);
  if (childItem == nullptr)
    return QModelIndex();

  TreeItem *parentItem = childItem->parent;
  if (parentItem->itype == TreeItem::Type::ROOT)
    return QModelIndex();

  return createIndex(parentItem->childIndex(childItem), 0, parentItem);
}

int ComplexationRelationshipsModel::rowCount(const QModelIndex &parent) const
{
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    _currentRoot->childrenCount();

  TreeItem *item = static_cast<TreeItem *>(parent.internalPointer());
  if (item == nullptr) {
    if (_currentRoot == nullptr)
        return 0;
    return _currentRoot->childrenCount();
  }

  return item->childrenCount();
}

bool ComplexationRelationshipsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  auto stringToItems = [](QVector<double> &vec, const QVariant &value) {
    QStringList numbers = value.toString().split(" ");
    QVector<double> _temp;

    for (const QString &s : numbers) {
      bool ok;
      double d;

      d = DoubleToStringConvertor::back(s, &ok);
      if (!ok)
        return false;

        _temp.push_back(d);
      }

      vec = _temp;
      return true;
  };

  if (role != Qt::EditRole)
    return false;

  TreeItem *item = modelIndexToItem(index);
  if (item->itype != TreeItem::Type::LIGAND)
    return false;

  LigandTreeItem *ligandItem = static_cast<LigandTreeItem *>(item);

  switch (index.column()) {
  case 2:
    {
    bool correct = stringToItems(ligandItem->mobilities, value);

    if (!correct)
      return false; /* TODO: Display a warning message here? */

    return true;
    }
   case 3:
    {
    bool correct = stringToItems(ligandItem->pBs, value);

    if (!correct)
      return false; /* TODO: Display a warning message here? */

    return true;
    }
  default:
    return false;
  }
}

void ComplexationRelationshipsModel::setRoot(std::shared_ptr<RootTreeItem> root)
{
  beginResetModel();
  _currentRoot = root;
  endResetModel();
}
