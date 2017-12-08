#include "calculatorinterface.h"

#include "../gdm/core/gdm.h"
#include "../gdm/conversion/conversion.h"

#include <cassert>
#include <lemng.h>
#include <memory>
#include <QPointF>

void destroyCZESystem(ECHMET::LEMNG::CZESystem *czeSystem)
{
  ECHMET::LEMNG::releaseCZESystem(czeSystem);
}

void destroyInConcentrationMap(ECHMET::LEMNG::InAnalyticalConcentrationsMap *map)
{
  map->destroy();
}

typedef std::unique_ptr<ECHMET::LEMNG::InAnalyticalConcentrationsMap, decltype(&destroyInConcentrationMap)> IACMapWrapper;
typedef std::unique_ptr<ECHMET::LEMNG::CZESystem, decltype(&destroyCZESystem)> CZESystemWrapper;

void applyAnalyticalConcentrations(const gdm::GDM &gdm, ECHMET::LEMNG::InAnalyticalConcentrationsMap *acMap)
{
  for (auto it = gdm.cbegin(); it != gdm.cend(); it++) {
    auto cVec = gdm.concentrations(it);
    assert(cVec.size() == 1);

    acMap->item(it->name().c_str()) = cVec.at(0);
  }
}

double mobilityFromTime(const double t, const double totalLength, const double detectorPosition, const double drivingVoltage)
{
  const double E = drivingVoltage / totalLength;
  return detectorPosition / (t * E);
}

double EOFMobilityFromInput(const double EOFValue, const CalculatorInterface::EOFValueType EOFvt,
                            const double totalLength, const double detectorPosition, const double drivingVoltage)
{
  if (EOFvt == CalculatorInterface::EOFValueType::MOBILITY)
    return EOFValue;
  return mobilityFromTime(EOFValue, totalLength, detectorPosition, drivingVoltage) * 1.0e9;
}

CalculatorInterface::CalculatorInterface(gdm::GDM &backgroundGDM, gdm::GDM &sampleGDM, ResultsData resultsData) :
  QObject{nullptr},
  m_backgroundGDM{backgroundGDM},
  m_sampleGDM{sampleGDM},
  m_resultsData{resultsData}
{
}

CalculatorInterface::CalculatorInterface(const CalculatorInterface &other) :
  QObject{nullptr},
  m_backgroundGDM{other.m_backgroundGDM},
  m_sampleGDM{other.m_sampleGDM},
  m_resultsData{other.m_resultsData},
  m_ctx{other.m_ctx}
{
}

CalculatorInterface::CalculatorInterface(CalculatorInterface &&other) noexcept :
  QObject{nullptr},
  m_backgroundGDM{other.m_backgroundGDM},
  m_sampleGDM{other.m_sampleGDM},
  m_resultsData{std::move(other.m_resultsData)},
  m_ctx{std::move(other.m_ctx)}
{
}

CalculatorInterface::~CalculatorInterface() noexcept
{ 
}

QVector<QString> CalculatorInterface::allConstituents() const
{
  QVector<QString> _ctuents{};

  if (!m_ctx.isValid())
    return _ctuents;

  for (const auto &c : m_sampleGDM)
    _ctuents.append(QString::fromStdString(c.name()));

  return _ctuents;
}

QVector<QString> CalculatorInterface::analytes() const
{
  QVector<QString> _analytes{};

  if (!m_ctx.isValid())
    return _analytes;

  for (const auto &a : m_ctx.analytes)
    _analytes.append(QString::fromStdString(a));

  return _analytes;
}

void CalculatorInterface::calculate(double totalLength, double detectorPosition, double drivingVoltage,
                                    const double EOFValue, const EOFValueType EOFvt,
                                    bool positiveVoltage, bool ionicStrengthCorrection)
{
  ECHMET::LEMNG::CZESystem *czeSystemRaw;
  ECHMET::LEMNG::InAnalyticalConcentrationsMap *backgroundMapRaw;
  ECHMET::LEMNG::InAnalyticalConcentrationsMap *sampleMapRaw;

  if (totalLength <= 0)
    throw CalculatorInterfaceException{"Invalid value of \"total length\""};
  if (detectorPosition > totalLength)
    throw CalculatorInterfaceException{"Invalid value of \"detector position\""};
  if (drivingVoltage <= 0)
    throw CalculatorInterfaceException{"Invalid value of \"driving voltage\""};

  if (!positiveVoltage)
    drivingVoltage *= -1;

  totalLength /= 100.0;
  detectorPosition /= 100.0;

  const double EOFMobility = EOFMobilityFromInput(EOFValue, EOFvt, totalLength, detectorPosition, drivingVoltage);

  m_ctx.invalidate();

  fillAnalytesList();

  auto backgroundIcVec = conversion::makeECHMETInConstituentVec(m_backgroundGDM);
  auto sampleIcVec = conversion::makeECHMETInConstituentVec(m_sampleGDM);

  ECHMET::LEMNG::RetCode tRet = ECHMET::LEMNG::makeCZESystem(backgroundIcVec.get(), sampleIcVec.get(), czeSystemRaw);
  if (tRet != ECHMET::LEMNG::RetCode::OK)
    throw CalculatorInterfaceException{ECHMET::LEMNG::LEMNGerrorToString(tRet)};

  CZESystemWrapper czeSystem{czeSystemRaw, destroyCZESystem};

  tRet = czeSystem->makeAnalyticalConcentrationsMaps(backgroundMapRaw, sampleMapRaw);
  if (tRet != ECHMET::LEMNG::RetCode::OK)
    throw CalculatorInterfaceException{czeSystem->lastErrorString()};

  IACMapWrapper backgroudMap{backgroundMapRaw, destroyInConcentrationMap};
  IACMapWrapper sampleMap{sampleMapRaw, destroyInConcentrationMap};

  applyAnalyticalConcentrations(m_backgroundGDM, backgroundMapRaw);
  applyAnalyticalConcentrations(m_sampleGDM, sampleMapRaw);

  tRet = czeSystem->evaluate(backgroundMapRaw, sampleMapRaw, ionicStrengthCorrection, *m_ctx.results);
  if (tRet != ECHMET::LEMNG::RetCode::OK)
    throw CalculatorInterfaceException{czeSystem->lastErrorString()};

  mapResults(totalLength, detectorPosition, drivingVoltage, EOFMobility);

  m_ctx.makeValid();
}

void CalculatorInterface::fillAnalytesList()
{
  for (const auto &it : m_sampleGDM) {
    if (m_backgroundGDM.find(it.name()) == m_backgroundGDM.cend())
      m_ctx.analytes.emplace_back(it.name());
  }
}

void CalculatorInterface::mapResults(const double totalLength, const double detectorPosition, const double drivingVoltage,
                                     const double EOFMobility)
{
  /* Map BGE properties */
  {
    auto &data = m_resultsData.backgroundPropsData();

    data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::BUFFER_CAPACITY)] = m_ctx.results->BGEProperties.bufferCapacity;
    data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::CONDUCTIVITY)] = m_ctx.results->BGEProperties.conductivity;
    data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::RESISTIVITY)] = 1.0 / m_ctx.results->BGEProperties.conductivity;
    data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::IONIC_STRENGTH)] = m_ctx.results->BGEProperties.ionicStrength * 1000.0;
    data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::PH)] = m_ctx.results->BGEProperties.pH;
    data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::EOF_MOBILITY)] = EOFMobility;
    data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::EOF_MARKER_TIME)] = detectorPosition / (EOFMobility * 1.0e-9 * drivingVoltage / totalLength) / 60.0;
    m_resultsData.backgroundPropsRefresh();
  }

  /* Map system eigenzones */
  recalculateTimesInternal(totalLength, detectorPosition, drivingVoltage, EOFMobility);

  /* Update eigenzone details */
  recalculateEigenzoneDetails(totalLength, detectorPosition, drivingVoltage, EOFMobility);
}

double CalculatorInterface::mobilityToTime(const double totalLength, const double detectorPosition, const double drivingVoltage, const double EOFMobility, const double u)
{
  const double E = drivingVoltage / totalLength;

  return detectorPosition / ((u + EOFMobility) * E * 1.0e-9) / 60.0;
}

void CalculatorInterface::onInvalidate()
{
  m_ctx.invalidate();

  auto &data = m_resultsData.backgroundPropsData();
  data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::BUFFER_CAPACITY)] = 0.0;
  data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::CONDUCTIVITY)] = 0.0;
  data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::RESISTIVITY)] = 0.0;
  data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::IONIC_STRENGTH)] = 0.0;
  data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::PH)] = 0.0;
  data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::EOF_MOBILITY)] = 0.0;
  data[m_resultsData.backgroundPropsIndex(BackgroundPropertiesMapping::Items::EOF_MARKER_TIME)] = 0.0;
  m_resultsData.backgroundPropsRefresh();

  m_resultsData.systemEigenzonesRefresh({});

  m_resultsData.eigenzoneDetailsRefresh({}, {});
}

QVector<QPointF> CalculatorInterface::plotAllAnalytes(const double totalLength, const double detectorPosition, const double drivingVoltage,
                                                      const double EOFMobility, const double injectionZoneLength, const double plotToTime)
{
  QVector<QPointF> plot{};

  for (const auto &a : m_ctx.analytes) {
    ECHMET::LEMNG::RetCode tRet;
    ECHMET::LEMNG::EFGPairVec *efg;

    tRet = ECHMET::LEMNG::plotElectrophoregram(efg, *m_ctx.results, drivingVoltage, totalLength, detectorPosition, EOFMobility, injectionZoneLength,
                                               ECHMET::LEMNG::EFGResponseType::RESP_CONCENTRATION, a.c_str(),
                                               plotToTime);

    if (tRet != ECHMET::LEMNG::RetCode::OK)
      throw CalculatorInterfaceException{ECHMET::LEMNG::LEMNGerrorToString(tRet)};

    if (plot.size() < 1)
      plot.resize(efg->size());

    for (size_t idx = 0; idx < efg->size(); idx++) {
      const ECHMET::LEMNG::EFGPair &pair = efg->at(idx);
      QPointF &pt = plot[idx];
      pt.setX(pair.time / 60.0);
      pt.setY(pt.y() + pair.value);
    }

    efg->destroy();
  }

  return plot;
}

QVector<QPointF> CalculatorInterface::plotElectrophoregram(double totalLength, double detectorPosition,
                                                           double drivingVoltage, const bool positiveVoltage,
                                                           double EOFValue, const EOFValueType EOFvt,
                                                           double injectionZoneLength, double plotToTime,
                                                           const Signal &signal)
{
  QVector<QPointF> plot{};

  if (!m_ctx.isValid())
    return plot;

  totalLength /= 100.0;
  detectorPosition /= 100.0;
  injectionZoneLength /= 1000.0;
  plotToTime *= 60.0;

  const double EOFMobility = EOFMobilityFromInput(EOFValue, EOFvt, totalLength, detectorPosition, drivingVoltage);

  if (!positiveVoltage)
    drivingVoltage *= -1.0;

  if (signal.type == SignalTypes::ALL_ANALYTES)
    return plotAllAnalytes(totalLength, detectorPosition, drivingVoltage, EOFMobility, injectionZoneLength, plotToTime);

  std::string ctuentName;
  ECHMET::LEMNG::EFGResponseType respType;

  switch (signal.type) {
  case SignalTypes::CONDUCTIVITY:
    respType = ECHMET::LEMNG::EFGResponseType::RESP_CONDUCTIVITY;
    break;
  case SignalTypes::PH_RESPONSE:
    respType = ECHMET::LEMNG::EFGResponseType::RESP_PH;
    break;
  case SignalTypes::CONCENTRATION:
    respType = ECHMET::LEMNG::EFGResponseType::RESP_CONCENTRATION;
    ctuentName = signal.constituentName.toStdString();
    break;
  default:
    throw CalculatorInterfaceException{"Internal error"};
  }

  ECHMET::LEMNG::RetCode tRet;
  ECHMET::LEMNG::EFGPairVec *efg;

  tRet = ECHMET::LEMNG::plotElectrophoregram(efg, *m_ctx.results, drivingVoltage, totalLength, detectorPosition, EOFMobility, injectionZoneLength,
                                             respType, ctuentName.c_str(), plotToTime);
  if (tRet != ECHMET::LEMNG::RetCode::OK)
    throw CalculatorInterfaceException{ECHMET::LEMNG::LEMNGerrorToString(tRet)};

  plot.reserve(efg->size());

  for (size_t idx = 0; idx < efg->size(); idx++) {
    const ECHMET::LEMNG::EFGPair &pair = efg->at(idx);
    plot.append(QPointF{pair.time / 60.0, pair.value});
  }

  efg->destroy();

  return plot;
}

void CalculatorInterface::recalculateEigenzoneDetails(const double totalLength, const double detectorPosition, const double drivingVoltage, const double EOFMobility)
{
  /* TODO: Calculate eigenzone times elsewhere and just once!!! */
  QVector<QString> constituents{};
  QVector<EigenzoneDetailsModel::EigenzoneProps> ezPropsVec{};

  if (m_ctx.results->eigenzones->size() < 1)
    return;

  constituents.reserve(m_ctx.results->eigenzones->size());
  ezPropsVec.reserve(m_ctx.results->eigenzones->size());

  const auto &ez = m_ctx.results->eigenzones->at(0);
  auto it = ez.solutionProperties.composition->begin();
  if (it == nullptr)
    return;

  while (it->hasNext()) {
    const auto &ctuentName = it->key();
    constituents.append(QString::fromUtf8(ctuentName));
    it->next();
  }
  it->destroy();


  for (size_t idx = 0; idx < m_ctx.results->eigenzones->size(); idx++) {
    const auto &ez = m_ctx.results->eigenzones->at(idx);

    const double t = mobilityToTime(totalLength, detectorPosition, drivingVoltage, EOFMobility, ez.mobility);
    QVector<double> concentrations{};

    auto _it = ez.solutionProperties.composition->begin();
    while (_it->hasNext()) {
      const auto &props = it->value();
      concentrations.append(props.concentration);
      _it->next();
    }
    _it->destroy();

    EigenzoneDetailsModel::EigenzoneProps ezProps{
      ez.tainted,
      ez.ztype == ECHMET::LEMNG::EigenzoneType::ANALYTE,
      t,
      ez.solutionProperties.pH,
      ez.uEMD,
      std::move(concentrations)
    };

    ezPropsVec.append(std::move(ezProps));
  }

  m_resultsData.eigenzoneDetailsRefresh(std::move(constituents), std::move(ezPropsVec));
}


void CalculatorInterface::recalculateTimes(double totalLength, double detectorPosition, double drivingVoltage,
                                           const double EOFValue, const EOFValueType EOFvt, bool positiveVoltage)
{
  if (!m_ctx.isValid())
    return;

  if (totalLength <= 0)
    throw CalculatorInterfaceException{"Invalid value of \"total length\""};
  if (detectorPosition > totalLength)
    throw CalculatorInterfaceException{"Invalid value of \"detector position\""};
  if (drivingVoltage <= 0)
    throw CalculatorInterfaceException{"Invalid value of \"driving voltage\""};

  if (!positiveVoltage)
    drivingVoltage *= -1;

  totalLength /= 100.0;
  detectorPosition /= 100.0;

  const double EOFMobility = EOFMobilityFromInput(EOFValue, EOFvt, totalLength, detectorPosition, drivingVoltage);

  recalculateTimesInternal(totalLength, detectorPosition, drivingVoltage, EOFMobility);
  recalculateEigenzoneDetails(totalLength, detectorPosition, drivingVoltage, EOFMobility);
}

void CalculatorInterface::recalculateTimesInternal(const double totalLength, const double detectorPosition, const double drivingVoltage,
                                                   const double EOFMobility)
{
  SystemEigenzonesTableModel::DataVector dataVec{};

  for (size_t idx = 0; idx < m_ctx.results->eigenzones->size(); idx++) {
    const ECHMET::LEMNG::REigenzone &ez = m_ctx.results->eigenzones->at(idx);

    if (ez.ztype != ECHMET::LEMNG::EigenzoneType::SYSTEM)
      continue;

    dataVec.emplace_back(ez.mobility, mobilityToTime(totalLength, detectorPosition, drivingVoltage, EOFMobility, ez.mobility));
  }

  m_resultsData.systemEigenzonesRefresh(std::move(dataVec));
}


bool CalculatorInterface::resultsAvailable() const
{
  return m_ctx.isValid();
}
