#include "complexationparametersdelegate.h"
#include "doubletostringconvertor.h"
#include "../ui/complexation_models/complexationrelationshipsmodel.h"
#include "../mappers/userroles.h"

#include <QLineEdit>

ComplexationParametersDelegate::ComplexationParametersDelegate(QObject *parent) : QItemDelegate (parent)
{
}

QWidget *ComplexationParametersDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  Q_UNUSED(option);

  const QVariant itemType = index.model()->data(index, UserRoles::PrecissionRole);
  switch (itemType.value<ComplexationRelationshipsModel::ItemType>()) {
  case ComplexationRelationshipsModel::ItemType::PARAMETER_LIST:
    break;
  default:
    return QItemDelegate::createEditor(parent, option, index);
  }

  auto lineEdit = new QLineEdit{parent};

  connect(lineEdit, &QLineEdit::textChanged, this, &ComplexationParametersDelegate::onTextChanged);

  return lineEdit;
}

void ComplexationParametersDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  if (!index.isValid())
    return;

  const QVariant v = index.model()->data(index, Qt::EditRole);
  if (!v.isValid())
    return;

  auto lineEdit = qobject_cast<QLineEdit *>(editor);
  if (lineEdit == nullptr)
    return;

  lineEdit->setText(v.toString());
}

void ComplexationParametersDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  auto lineEdit = qobject_cast<QLineEdit *>(editor);
  if (lineEdit == nullptr)
    return;

  model->setData(index, lineEdit->text(), Qt::EditRole);
}

void ComplexationParametersDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  Q_UNUSED(index);

  editor->setGeometry(option.rect);
}

void ComplexationParametersDelegate::onTextChanged(const QString &)
{
  auto lineEdit = qobject_cast<QLineEdit *>(QObject::sender());
  if (lineEdit == nullptr)
    return;

  bool ok = true;

  const QStringList blocks =  lineEdit->text().split(ComplexationRelationshipsModel::ELEMENTS_SPLITTER, QString::SkipEmptyParts);
  for (const auto &s : blocks) {
    DoubleToStringConvertor::back(s, &ok);
    if (!ok)
        break;
  }

  if (ok || blocks.empty())
    lineEdit->setPalette(QPalette{});
  else {
    QPalette palette = lineEdit->palette();

    palette.setColor(QPalette::Base, QColor(Qt::red));
    lineEdit->setPalette(palette);
  }
}
