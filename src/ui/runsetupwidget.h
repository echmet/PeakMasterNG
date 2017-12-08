#ifndef RUNSETUPWIDGET_H
#define RUNSETUPWIDGET_H

#include "../mappers/floatmappermodel.h"
#include "../gearbox/floatingvaluedelegate.h"

#include <QWidget>

namespace Ui {
  class RunSetupWidget;
}

class QDataWidgetMapper;

class RunSetupWidget : public QWidget
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

  enum class Items {
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
    bool ionicStrengthCorrection;
  };

  explicit RunSetupWidget(QWidget *parent = nullptr);
  ~RunSetupWidget();
  EOF_Type EOFInputType() const;
  double EOFValue() const;
  RunSetup runSetup() const;

private:
  Ui::RunSetupWidget *ui;

  QDataWidgetMapper *m_mapper;
  FloatMapperModel<Items> m_mapperModel;
  FloatingValueDelegate m_fltDelegate;
  QVector<double> m_mappedData;

private slots:
  void onEOFCurrentIndexChanged(const int idx);
};

Q_DECLARE_METATYPE(RunSetupWidget::Polarity)
Q_DECLARE_METATYPE(RunSetupWidget::EOF_Type)

#endif // RUNSETUPWIDGET_H
