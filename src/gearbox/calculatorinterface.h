#ifndef CALCULATORINTERFACE_H
#define CALCULATORINTERFACE_H

#include "results_models/resultsdata.h"
#include "calculatorcontext.h"

#include <QObject>
#include <stdexcept>

namespace gdm {
  class GDM;
}

class CalculatorInterfaceException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class CalculatorInterface : public QObject
{
  Q_OBJECT

public:
  enum class SignalTypes {
    CONDUCTIVITY,
    PH_RESPONSE,
    ALL_ANALYTES,
    CONCENTRATION,
  };
  Q_ENUM(SignalTypes)

  enum class EOFValueType {
    MOBILITY,
    MARKER_TIME
  };

  class Signal
  {
  public:
    SignalTypes type;
    QString constituentName;
    QString plotCaption;
  };

  CalculatorInterface(gdm::GDM &backgroundGDM, gdm::GDM &sampleGDM, ResultsData resultsData);
  CalculatorInterface(const CalculatorInterface &other);
  CalculatorInterface(CalculatorInterface &&other) noexcept;
  ~CalculatorInterface() noexcept;
  QVector<QString> allConstituents() const;
  QVector<QString> analytes() const;
  void calculate(bool ionicStrengthCorrection);
  void exportElectrophoregram(double totalLength, double detectorPosition,
                              double drivingVoltage, const bool positiveVoltage,
                              double EOFValue, const EOFValueType EOFvt,
                              double injectionZoneLength, double plotToTime,
                              const Signal &signal,
                              const QString &filepath);
  QVector<QPointF> plotElectrophoregram(double totalLength, double detectorPosition,
                                        double drivingVoltage, const bool positiveVoltage,
                                        double EOFValue, const EOFValueType EOFvt,
                                        double injectionZoneLength, double plotToTime,
                                        const Signal &signal);
  void publishResults(double totalLength, double detectorPosition, double drivingVoltage,
                      const double EOFValue, const EOFValueType EOFvt,
                      bool positiveVoltage);
  void recalculateTimes(double totalLength, double detectorPosition, double drivingVoltage,
                        const double EOFValue, const EOFValueType EOFvt, bool positiveVoltage);
  bool resultsAvailable() const;

public slots:
  void onInvalidate();

private:
  void fillAnalytesList();
  void mapResults(const double totalLength, const double detectorPosition, const double drivingVoltage, const double EOFMobility);
  double mobilityToTime(const double totalLength, const double detectorPosition, const double drivingVoltage, const double EOFMobility, const double u);
  QVector<QPointF> plotAllAnalytes(const double totalLength, const double detectorPosition, const double drivingVoltage,
                                   const double EOFMobility, const double injectionZoneLength, const double plotToTime);
  QVector<QPointF> plotElectrophoregramInternal(double totalLength, double detectorPosition,
                                                double drivingVoltage, const bool positiveVoltage,
                                                double EOFValue, const EOFValueType EOFvt,
                                                double injectionZoneLength, double plotToTime,
                                                const Signal &signal);
  void recalculateEigenzoneDetails(const double totalLength, const double detectorPosition, const double drivingVoltage, const double EOFMobility);
  void recalculateTimesInternal(const double totalLength, const double detectorPosition, const double drivingVoltage, const double EOFMobility);

  const gdm::GDM &m_backgroundGDM;
  const gdm::GDM &m_sampleGDM;
  ResultsData m_resultsData;
  CalculatorContext m_ctx;
};

Q_DECLARE_METATYPE(CalculatorInterface::Signal)

#endif // CALCULATORINTERFACE_H
