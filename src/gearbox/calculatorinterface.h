#ifndef CALCULATORINTERFACE_H
#define CALCULATORINTERFACE_H

#include "results_models/resultsdata.h"
#include "calculatorcontext.h"

#include <QObject>
#include <stdexcept>

namespace gdm {
  class Constituent;
  class GDM;
}

class CalculatorInterfaceException : public std::runtime_error {
public:
  enum class SolutionState {
    INVALID,
    BGE_ONLY,
    PARTIAL_EIGENZONES
  };

  explicit CalculatorInterfaceException(const char *message, const SolutionState state = SolutionState::INVALID);

  const SolutionState state;
};

class CalculatorInterface : public QObject {
  Q_OBJECT

public:
  using ConstituentPack = std::pair<QString, bool>; // { Constituent name, is constituent analyte? }
  using ConstituentPackVec = QVector<ConstituentPack>;

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
    QString signalName;
    bool isAnalyteSignal;

    bool operator==(const Signal &other) const noexcept {
      return type == other.type &&
             constituentName == other.constituentName &&
             signalName == other.signalName &&
             isAnalyteSignal == other.isAnalyteSignal;
    }

    bool operator!=(const Signal &other) const noexcept {
      return !(*this == other);
    }
  };

  class TimeDependentZoneInformation {
  public:
    TimeDependentZoneInformation() :
      isSystemZone{false},
      timeTrue{0},
      timeMax{0},
      beginsAt{0},
      endsAt{0},
      name{},
      concentrationMax{0},
      conductivityMax{0}
    {}
    TimeDependentZoneInformation(const bool isSystemZone, const double timeTrue, const double timeMax,
                                 const double beginsAt, const double endsAt, QString &&name,
                                 const double concentrationMax, const double conductivityMax) :
      isSystemZone{isSystemZone},
      timeTrue{timeTrue},
      timeMax{timeMax},
      beginsAt{beginsAt},
      endsAt{endsAt},
      name{name},
      concentrationMax{concentrationMax},
      conductivityMax{conductivityMax}
    {}
    TimeDependentZoneInformation(const TimeDependentZoneInformation &other) :
      isSystemZone{other.isSystemZone},
      timeTrue{other.timeTrue},
      timeMax{other.timeMax},
      beginsAt{other.beginsAt},
      endsAt{other.endsAt},
      name{other.name},
      concentrationMax{other.concentrationMax},
      conductivityMax{other.conductivityMax}
    {}

    TimeDependentZoneInformation & operator=(const TimeDependentZoneInformation &other)
    {
      const_cast<bool&>(isSystemZone) = other.isSystemZone;
      const_cast<double&>(timeTrue) = other.timeTrue;
      const_cast<double&>(timeMax) = other.timeMax;
      const_cast<double&>(beginsAt) = other.beginsAt;
      const_cast<double&>(endsAt) = other.endsAt;
      const_cast<QString&>(name) = other.name;
      const_cast<double&>(concentrationMax) = other.concentrationMax;
      const_cast<double&>(conductivityMax) = other.conductivityMax;

      return *this;
    }

    const bool isSystemZone;
    const double timeTrue;
    const double timeMax;
    const double beginsAt;
    const double endsAt;
    const QString name;
    const double concentrationMax;
    const double conductivityMax;
  };

  class TracepointInfo {
  public:
    TracepointInfo(const int32_t TPID, QString &&description) noexcept :
      TPID{TPID},
      description{description}
    {}

    TracepointInfo(const TracepointInfo &other) :
      TPID{other.TPID},
      description{other.description}
    {}

    TracepointInfo & operator=(const TracepointInfo &other)
    {
      const_cast<int32_t&>(TPID) = other.TPID;
      const_cast<QString&>(description) = other.description;

      return *this;
    }

    const int32_t TPID;
    const QString description;
  };

  class TracepointState {
  public:
    TracepointState(const int32_t TPID, const bool enabled) :
      TPID{TPID},
      enabled{enabled}
    {}

    TracepointState(const TracepointState &other) :
      TPID{other.TPID},
      enabled{other.enabled}
    {}

    TracepointState & operator=(const TracepointState &other)
    {
      const_cast<int32_t&>(TPID) = other.TPID;
      const_cast<bool&>(enabled) = other.enabled;

      return *this;
    }

    const int32_t TPID;
    const bool enabled;
  };

  CalculatorInterface(gdm::GDM &backgroundGDM, gdm::GDM &sampleGDM, ResultsData resultsData);
  CalculatorInterface(const CalculatorInterface &other);
  CalculatorInterface(CalculatorInterface &&other) noexcept;
  ~CalculatorInterface() noexcept;
  ConstituentPackVec allConstituents() const;
  ConstituentPackVec analytes() const;
  void calculate(const bool correctForDebyeHuckel, const bool correctForOnsagerFuoss, const bool correctForViscosity);
  void disableAllTracepoints() noexcept;
  static double minimumConcentration() noexcept;
  static double minimumAnalyteConcentration() noexcept;
  QVector<QPointF> plotElectrophoregram(double totalLength, double detectorPosition,
                                        double drivingVoltage, const bool positiveVoltage,
                                        double EOFValue, const EOFValueType EOFvt,
                                        double injectionZoneLength, double plotToTime,
                                        const Signal &signal);
  void publishResults(double totalLength, double detectorPosition, double drivingVoltage,
                      const double EOFValue, const EOFValueType EOFvt,
                      const bool positiveVoltage);
  void recalculateTimes(double totalLength, double detectorPosition, double drivingVoltage,
                        const double EOFValue, const EOFValueType EOFvt, bool positiveVoltage);
  bool resultsAvailable() const;
  void setTracepoints(const std::vector<TracepointState> &tracepointStates) noexcept;
  std::vector<TimeDependentZoneInformation> timeDependentZoneInformation(double totalLength, double detectorPosition, double drivingVoltage,
                                                                         const double EOFValue, const EOFValueType EOFvt, bool positiveVoltage,
                                                                         const double injectionZoneLength, const double plotToTime) const;
  std::vector<TracepointInfo> tracepointInformation() const;
  bool writeTrace(const QString &traceOutputFile) noexcept;

public slots:
  void onInvalidate();

private:
  bool constituentIsAnalyte(const gdm::Constituent &c) const;
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
