#include "floatingvaluedelegate.h"
#include "doubletostringconvertor.h"
#include "additionalfloatingvalidator.h"
#include <QLineEdit>
#include <QEvent>

#include <QDebug>

FloatingValueDelegate::FloatingValueDelegate(QObject *parent) : QItemDelegate(parent)
{
}

QWidget *FloatingValueDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  Q_UNUSED(option);

  QLineEdit *lineEdit = new QLineEdit(parent);

  const auto &current = index.model()->data(index);

  lineEdit->setText(DoubleToStringConvertor::convert(current.toDouble()));
  lineEdit->selectAll();

  connect(lineEdit, &QLineEdit::textChanged, this, &FloatingValueDelegate::onTextChanged);
  connect(this, &FloatingValueDelegate::editorCommit, lineEdit, &QLineEdit::clearFocus);

  return lineEdit;
}

void FloatingValueDelegate::forceCommit()
{
  emit editorCommit();
}

void FloatingValueDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  if (!index.isValid())
    return;

  const QVariant v = index.model()->data(index, Qt::EditRole);
  if (!v.isValid())
    return;

  bool ok;
  double value = v.toDouble(&ok);
  if (!ok)
    return;

  QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
  if (lineEdit == nullptr)
    return;

  const int prec = index.model()->data(index, Qt::UserRole + 1).toInt();

  lineEdit->setText(DoubleToStringConvertor::convert(value, 'f', prec));
}

void FloatingValueDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  double value;
  bool ok;
  QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
  if (lineEdit == nullptr)
    return;

  QString s(lineEdit->text());
  s = s.replace(QChar::Nbsp, QString(""));
  value = DoubleToStringConvertor::back(s, &ok);
  if (!ok)
    return;
  if (!AdditionalFloatingValidator::additionalValidatorsOk(this, value))
    return;

  const int prec  = DoubleToStringConvertor::decimalDigits(s);
  /* Calling order matters. See FloatingMapperModel::setData() for details */
  model->setData(index, prec, Qt::UserRole + 1);
  model->setData(index, value, Qt::EditRole);
}

void FloatingValueDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  Q_UNUSED(index);

  editor->setGeometry(option.rect);
}

void FloatingValueDelegate::onTextChanged(const QString &)
{
  bool ok;
  QLineEdit *lineEdit = qobject_cast<QLineEdit *>(QObject::sender());
  if (lineEdit == nullptr)
    return;

  QString s(lineEdit->text());
  s = s.replace(QChar::Nbsp, QString(""));
  const double dv = DoubleToStringConvertor::back(s, &ok);
  ok = ok || lineEdit->text().length() == 0;

  if (ok)
    ok = AdditionalFloatingValidator::additionalValidatorsOk(lineEdit, dv);
  if (ok)
    ok = AdditionalFloatingValidator::additionalValidatorsOk(this, dv);

  if (ok)
    lineEdit->setPalette(QPalette());
  else {
    QPalette palette = lineEdit->palette();

    palette.setColor(QPalette::Base, QColor(Qt::red));
    lineEdit->setPalette(palette);
  }
}
