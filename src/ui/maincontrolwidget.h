#ifndef MAINCONTROLWIDGET_H
#define MAINCONTROLWIDGET_H

#include "../mappers/floatmappermodel.h"
#include "../gearbox/floatingvaluedelegate.h"
#include "../gearbox/results_models/resultsmodels.h"

#include <QWidget>

namespace Ui {
  class MainControlWidget;
}

class EigenzoneDetailsDialog;
class IonicCompositionDialog;
class NonidealityCorrectionsDialog;
class QDataWidgetMapper;

class MainControlWidget : public QWidget
{
  Q_OBJECT

public:
  enum Polarity : int {
    POLARITY_POSITIVE,
    POLARITY_NEGATIVE
  };

  enum EOF_Type : int {
    EOF_NONE,
    EOF_MARKER_TIME,
    EOF_MOBILITY
  };

  enum class RunSetupItems {
    TOTAL_LENGTH,
    DETECTOR_POSITION,
    DRIVING_VOLTAGE,
    EOF_VALUE,
    LAST_INDEX
  };

  class RunSetup {
  public:
    double totalLength;
    double detectorPosition;
    double drivingVoltage;
    bool positiveVoltage;
    bool correctForDebyeHuckel;
    bool correctForOnsagerFuoss;
    bool correctForViscosity;
  };

  explicit MainControlWidget(ResultsModels &resultsModels, QWidget *parent = nullptr);
  ~MainControlWidget();
  EOF_Type EOFInputType() const;
  double EOFValue() const;
  RunSetup runSetup() const;
  void setRunSetup(const RunSetup &rs, const QVariant &eofType, const double EOFValue);

signals:
  void runSetupChanged(bool invalidate);

private:
  void initBackgroundPropsModel(BackgroundPropertiesMapping::MapperModel *model);
  void initRunSetupModel();

  Ui::MainControlWidget *ui;

  QDataWidgetMapper *m_runSetupMapper;
  FloatMapperModel<RunSetupItems> m_runSetupMapperModel;
  QVector<double> m_runSetupMappedData;

  QDataWidgetMapper *m_backgroundPropsMapper;

  QAbstractTableModel *m_bgeIonicCompositionModel;
  QAbstractTableModel *m_eigenzoneDetailsModel;

  EigenzoneDetailsDialog *m_ezDetailsDlg;
  IonicCompositionDialog *m_bgeIonicCompDlg;
  NonidealityCorrectionsDialog *m_nonidealityCorrectionsDlg;

  FloatingValueDelegate m_fltDelegate;

private slots:
  void onBGEIonicCompositionClicked();
  void onEOFCurrentIndexChanged(const int idx);
  void onNonidealityCorrectionsClicked();
  void onRunSetupChanged();
  void onRunSetupChangedInvalidate();
  void onShowEigenzoneDetailsClicked();
};

Q_DECLARE_METATYPE(MainControlWidget::Polarity)
Q_DECLARE_METATYPE(MainControlWidget::EOF_Type)

#endif // MAINCONTROLWIDGET_H
