#include "../../gearbox/doubletostringconvertor.h"
#include "floatingvaluelineedit.h"
#include <QLocale>

FloatingValueLineEdit::FloatingValueLineEdit(QWidget *parent) :
  QLineEdit(parent)
{
  this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  connect(this, &FloatingValueLineEdit::textChanged, this, &FloatingValueLineEdit::ensureSanity);
  connect(this, &FloatingValueLineEdit::editingFinished, this, &FloatingValueLineEdit::onEditingFinished);
  DoubleToStringConvertor::notifyOnFormatChanged(this);
}

void FloatingValueLineEdit::ensureSanity(QString text)
{
  bool ok;

  QString _text = text.replace(QChar::Nbsp, QString(""), Qt::CaseInsensitive);

  DoubleToStringConvertor::back(text, &ok);
  if (ok)
    this->setPalette(QPalette());
  else {
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, Qt::red);
    this->setPalette(palette);
  }
}

void FloatingValueLineEdit::onEditingFinished()
{
  bool ok;
  double dv;

  dv = DoubleToStringConvertor::back(text(), &ok);
  if (ok)
    setNumberText(dv);
}

void FloatingValueLineEdit::onNumberFormatChanged(const QLocale *oldLocale)
{
  bool ok;
  double dv;

  dv = oldLocale->toDouble(this->text(), &ok);
  if (ok)
    setNumberText(dv);
}

void FloatingValueLineEdit::setNumberText(const double dv)
{
  blockSignals(true);

  const int prec = DoubleToStringConvertor::decimalDigits(text());
  const QString t = DoubleToStringConvertor::convert(dv, 'f', prec);
  this->setText(t);

  blockSignals(false);
}
