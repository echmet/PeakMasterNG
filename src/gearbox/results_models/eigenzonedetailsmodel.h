#ifndef EIGENZONEDETAILSMODEL_H
#define EIGENZONEDETAILSMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class EigenzoneDetailsModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  class EigenzoneProps {
  public:
    bool tainted;
    bool isAnalyte;
    double mobility;
    double time;
    double conductivity;
    double pH;
    double uEMD;
    QVector<double> concentrations;
    QVector<double> cDeltas;
  };

  explicit EigenzoneDetailsModel(QObject *parent = nullptr);
  bool displayConcentrationDeltasState() const;

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void refreshData(QVector<QString> &&constituents, QVector<EigenzoneProps> &&eigenzones) noexcept;

public slots:
  void displayConcentrationDeltas(const bool status);

private:
  double constituentsConcentrations(const EigenzoneProps &zone, const int idx) const;
  QString constituentsHeader(const int idx) const;

  QVector<QString> m_constituents;
  QVector<EigenzoneProps> m_eigenzones;
  bool m_displayCDeltas;

  static const QString s_typeStr;
  static const QString s_mobilityStr;
  static const QString s_timeStr;
  static const QString s_uEMDStr;
  static const QString s_conductivityStr;
  static const QString s_pHStr;
};

#endif // EIGENZONEDETAILSMODEL_H
