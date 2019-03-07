#include "toggletracepointsdialog.h"
#include "ui_toggletracepointsdialog.h"

#include <QFileDialog>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

ToggleTracepointsDialog::ToggleTracepointsDialog(const std::vector<CalculatorInterface::TracepointInfo> &tracepointInformation, const TracingSetup &tracingSetup,
                                                 QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ToggleTracepointsDialog)
{
  ui->setupUi(this);

  m_tracepointsWidget = new QWidget{this};
  m_tracepointsWidget->setLayout(new QVBoxLayout{m_tracepointsWidget});

  ui->qscrArea_tracepoints->setWidget(m_tracepointsWidget);
  m_tracepointsWidget->show();

  if (!tracepointInformation.empty())
    setupTracepointList(tracepointInformation, tracingSetup);
  else {
    QLabel *l = new QLabel{tr("No tracepoints are available.\nLEMNG library was probably built without tracing support."), this};
    l->setWordWrap(true);
    m_tracepointsWidget->layout()->addWidget(l);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ToggleTracepointsDialog::reject);
  }

  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ToggleTracepointsDialog::reject);
  connect(ui->qpb_disableAll, &QPushButton::clicked, this, &ToggleTracepointsDialog::onDisableAllClicked);
  connect(ui->qpb_enableAll, &QPushButton::clicked, this, &ToggleTracepointsDialog::onEnableAllClicked);
}

ToggleTracepointsDialog::~ToggleTracepointsDialog()
{
  delete ui;
}

void ToggleTracepointsDialog::onAccepted()
{
  if (m_outputFilePath.length() < 1 && ui->qcb_enableTracing->isChecked()) {
    QMessageBox errBox{QMessageBox::Warning, tr("Invalid input"), tr("Output file must be set")};
    errBox.exec();
    return;
  }

  for (auto &item : m_tracepoints) {
    const auto cbox = std::get<0>(item);
    std::get<1>(item) = cbox->isChecked();
  }

  accept();
}

void ToggleTracepointsDialog::onDisableAllClicked()
{
  for (auto &item : m_tracepoints) {
    const auto cbox = std::get<0>(item);
    cbox->setChecked(false);
  }
}

void ToggleTracepointsDialog::onEnableAllClicked()
{
  for (auto &item : m_tracepoints) {
    const auto cbox = std::get<0>(item);
    cbox->setChecked(true);
  }
}

void ToggleTracepointsDialog::onEnableTracingToggled(const bool enabled)
{
  ui->qle_outputFile->setEnabled(enabled);
  ui->qpb_chooseOutputFile->setEnabled(enabled);
  m_tracepointsWidget->setEnabled(enabled);
}

void ToggleTracepointsDialog::onFilterTextChanged(const QString &text)
{
  const bool showAll = text.length() < 1;

  for (auto &&item : m_tracepoints) {
    QCheckBox *cb = std::get<0>(item);
    const QString cbText = cb->text().remove('&');

    cb->setVisible(showAll ? true : cbText.contains(text, Qt::CaseInsensitive));
  }
}

void ToggleTracepointsDialog::onSetOutputFile()
{
  if (!ui->qcb_enableTracing->isChecked())
    return;

  QFileDialog dlg{this};

  dlg.setAcceptMode(QFileDialog::AcceptSave);
  if (dlg.exec() != QDialog::Accepted)
    return;

  m_outputFilePath = dlg.selectedFiles().constFirst();
  ui->qle_outputFile->setText([](QString s) {
#ifdef Q_OS_WIN
    return s.replace('/', '\\');
#else
    return s;
#endif // Q_OS_WIN
  }(m_outputFilePath));
}

ToggleTracepointsDialog::TracingSetup ToggleTracepointsDialog::result() const
{
  TracingSetup ts{
    m_outputFilePath,
    tracepointStates(),
    ui->qcb_enableTracing->isChecked()
  };

  return ts;
}

void ToggleTracepointsDialog::setupTracepointList(const std::vector<CalculatorInterface::TracepointInfo> &tracepointInformation, const TracingSetup &tracingSetup)
{
  auto lay = qobject_cast<QVBoxLayout *>(m_tracepointsWidget->layout());
  Q_ASSERT(lay != nullptr);

  for (const auto &item : tracepointInformation) {
    auto cbox = new QCheckBox{this};
    cbox->setText(item.description);
    cbox->setProperty("TPID", item.TPID);
    m_tracepoints.emplace_back(cbox, false);

    lay->addWidget(cbox);
  }
  lay->addStretch();

  if (!tracingSetup.tracepointStates.empty()) {
    Q_ASSERT(tracepointInformation.size() == tracingSetup.tracepointStates.size());

    for (size_t idx = 0; idx < tracingSetup.tracepointStates.size(); idx++) {
      const auto &item = tracingSetup.tracepointStates.at(idx);
      auto cbox = std::get<0>(m_tracepoints.at(idx));

      Q_ASSERT(item.TPID == cbox->property("TPID"));
      cbox->setChecked(item.enabled);
      std::get<1>(m_tracepoints.at(idx)) = item.enabled;
    }
  }

  ui->qle_outputFile->setText(tracingSetup.outputFilePath);
  ui->qcb_enableTracing->setChecked(tracingSetup.tracingEnabled);
  ui->qle_outputFile->setEnabled(tracingSetup.tracingEnabled);
  m_outputFilePath = tracingSetup.outputFilePath;

  m_tracepointsWidget->setEnabled(ui->qcb_enableTracing->checkState() == Qt::Checked);

  connect(ui->qpb_chooseOutputFile, &QPushButton::clicked, this, &ToggleTracepointsDialog::onSetOutputFile);
  connect(ui->qcb_enableTracing, &QCheckBox::toggled, this, &ToggleTracepointsDialog::onEnableTracingToggled);
  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ToggleTracepointsDialog::onAccepted);
  connect(ui->qle_filter, &QLineEdit::textChanged, this, &ToggleTracepointsDialog::onFilterTextChanged);
  connect(ui->qle_outputFile, &QLineEdit::editingFinished, this, [this]() { m_outputFilePath = ui->qle_outputFile->text();});

  onEnableTracingToggled(ui->qcb_enableTracing->checkState() == Qt::Checked);

  setMinimumHeight(minimumHeight() + m_tracepointsWidget->fontMetrics().height() *30);
}

std::vector<CalculatorInterface::TracepointState> ToggleTracepointsDialog::tracepointStates() const
{
  std::vector<CalculatorInterface::TracepointState> states{};

  states.reserve(m_tracepoints.size());

  for (size_t idx = 0; idx < m_tracepoints.size(); idx++) {
    const auto item = m_tracepoints.at(idx);
    const auto TPID = std::get<0>(item)->property("TPID").value<int32_t>();

    states.emplace_back(TPID, std::get<1>(item));
  }

  return states;
}
