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
    double conductivityDelta;
    double pH;
    double pHDelta;
    double uEMD;
    QVector<double> concentrations;
    QVector<double> cDeltas;
  };

  explicit EigenzoneDetailsModel(QObject *parent = nullptr);
  bool displayDeltasState() const;

  const EigenzoneProps & eigenzonePropsAt(const int idx) const noexcept;

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void refreshData(QVector<QString> &&constituents, QVector<EigenzoneProps> &&eigenzones) noexcept;

public slots:
  void displayDeltas(const bool status);

private:
  double constituentsConcentrations(const EigenzoneProps &zone, const int idx) const;
  QString constituentsHeader(const int idx) const;

  QVector<QString> m_constituents;
  QVector<EigenzoneProps> m_eigenzones;
  bool m_displayDeltas;

  QString m_mobilityStr;
  QString m_uEMDStr;

  static const QString s_typeStr;
  static const QString s_timeStr;
  static const QString s_conductivityStr;
  static const QString s_pHStr;
  static const QVector<int> s_untaintableRows;
};

#endif // EIGENZONEDETAILSMODEL_H
