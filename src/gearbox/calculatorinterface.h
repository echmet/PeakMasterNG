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
  explicit CalculatorInterfaceException(const char *message, const bool isBGEValid = false, const bool isAnalytesDissociationValid = false);

  const bool isBGEValid;
  const bool isAnalytesDissociationValid;
};

class CalculatorInterface : public QObject {
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

  class SpatialZoneInformation {
  public:
    SpatialZoneInformation() :
      time{0},
      width{0},
      name{}
    {}
    SpatialZoneInformation(const double time, const double width, QString &&name) :
      time{time},
      width{width},
      name{name}
    {}
    SpatialZoneInformation(const SpatialZoneInformation &other) :
      time{other.time},
      width{other.width},
      name{other.name}
    {}
    SpatialZoneInformation(SpatialZoneInformation &&other) noexcept :
      time{other.time},
      width{other.width},
      name{std::move(other.name)}
    {}

    const double time;
    const double width;
    const QString name;
  };

  CalculatorInterface(gdm::GDM &backgroundGDM, gdm::GDM &sampleGDM, ResultsData resultsData);
  CalculatorInterface(const CalculatorInterface &other);
  CalculatorInterface(CalculatorInterface &&other) noexcept;
  ~CalculatorInterface() noexcept;
  QVector<QString> allConstituents() const;
  QVector<QString> analytes() const;
  void calculate(const bool correctForDebyeHuckel, const bool correctForOnsagerFuoss, const bool correctForViscosity);
  static double minimumConcentration() noexcept;
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
  std::vector<SpatialZoneInformation> spatialZoneInformation(double totalLength, double detectorPosition, double drivingVoltage,
                                                             const double EOFValue, const EOFValueType EOFvt, bool positiveVoltage) const;

public slots:
  void onInvalidate();

private:
  void fillAnalytesList();
  void mapResultsAnalytesDissociation();
  void mapResultsBGE(const double totalLength, const double detectorPosition, const double drivingVoltage, const double EOFMobility);
  void mapResults(const double totalLength, const double detectorPosition, const double drivingVoltage, const double EOFMobility);
  double mobilityToTime(const double totalLength, const double detectorPosition, const double drivingVoltage, const double EOFMobility, const double u) const;
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
