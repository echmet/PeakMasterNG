#include "compositionwidget.h"
#include "ui_compositionwidget.h"

#include "constituentwidget.h"
#include "editconstituentdialog.h"
#include "../gearbox/gdmproxy.h"
#include "../gdm/core/constituent/constituent.h"
#include "../gdm/core/common/gdmexcept.h"

#include <QMessageBox>

CompositionWidget::CompositionWidget(GDMProxy &gdmProxy, QWidget *parent) :
  QWidget{parent},
  ui{new Ui::CompositionWidget{}},
  h_gdmProxy{gdmProxy}
{
  ui->setupUi(this);
  ui->qw_constituentList->setLayout(new QVBoxLayout{});

  connect(ui->qpb_add, &QPushButton::clicked, this, &CompositionWidget::onAddClicked);
}

CompositionWidget::~CompositionWidget()
{
  delete ui;
}


void CompositionWidget::addConstituent(const EditConstituentDialog *dlg, ConstituentWidget *w)
{
  try {
    gdm::Constituent ctuent = userInputToConstituent(dlg);

    h_gdmProxy.insert(std::move(ctuent));
  } catch (gdm::InvalidArgument &ex) {
    QMessageBox mbox{QMessageBox::Critical, tr("Runtime error"), QString{tr("Failed to add constituent: %1")}.arg(ex.what())};
    mbox.exec();
    delete w;

    return;
  }

  connect(w, &ConstituentWidget::editMe, this, &CompositionWidget::onEditConstituent);
  connect(w, &ConstituentWidget::removeMe, this, &CompositionWidget::onRemoveConstituent);
  ui->qw_constituentList->layout()->addWidget(w);

  emit invalidated();
}

void CompositionWidget::onEditConstituent(ConstituentWidget *w)
{
  const std::string name = w->name().toStdString();

  try {
    gdm::Constituent ctuent = h_gdmProxy.get(name);
    EditConstituentDialog::ConstituentType type = [](gdm::ConstituentType _type) {
      switch (_type) {
      case gdm::ConstituentType::Nucleus:
        return EditConstituentDialog::ConstituentType::NUCLEUS;
      case gdm::ConstituentType::Ligand:
        return EditConstituentDialog::ConstituentType::LIGAND;
      }
    }(ctuent.type());

    EditConstituentDialog dlg{w->name(), type, ctuent.physicalProperties()};
    connect(&dlg, &EditConstituentDialog::validateInput, this, &CompositionWidget::onValidateConstituentInputUpdate);

    if (dlg.exec() == QDialog::Accepted) {
      gdm::Constituent updatedCtuent = userInputToConstituent(&dlg);

      if (!h_gdmProxy.update(name, updatedCtuent)) {
        QMessageBox mbox{QMessageBox::Warning, tr("Operation failed"), tr("Failed to update the constituent properties")};
        mbox.exec();
      }

      emit invalidated();
    }
  } catch (GDMProxyException &ex) {
    QMessageBox mbox{QMessageBox::Critical, tr("Runtime error"), ex.what()};
    mbox.exec();
  }
}

void CompositionWidget::onRemoveConstituent(ConstituentWidget *w)
{
  const QString &name = w->name();
  h_gdmProxy.erase(name.toStdString());
  ui->qw_constituentList->layout()->removeWidget(w);

  w->deleteLater();

  emit invalidated();
}
