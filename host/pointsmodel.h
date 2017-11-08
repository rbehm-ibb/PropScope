// ******************************************************
// * copyright (C) 2016 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 5/26/2016 by behm
// ******************************************************

#ifndef POINTSMODEL_H
#define POINTSMODEL_H

#include "analogdata.h"
#include "channelgraph.h"

struct PointData
{
	QPointer<ScopeCursor> cursor;
	QPointer<QCPItemTracer> tracer0, tracer1;
	QString name;
	qreal time;
	qreal y0;
	qreal y1;
	PointData() : time(qQNaN()), y0(qQNaN()), y1(qQNaN()) {}
};

class PointsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	PointsModel(QObject *parent);
	~PointsModel();
	enum Columns { Id = 0, Time, Value0, Value1, NCols };
	Q_ENUM(Columns)
	void setGraphs(ChannelGraphVector graphs);
	QModelIndex addVCursor(VCursor *cursor);
	QModelIndex addHCursor(HCursor *cursor);
signals:
	void cursorDeleted();
	void selected(const QModelIndex idx);
	void empty(bool);
public slots:
	void saveData(QSettings *file);
//	void delPoint(int idx);
	void cursorSelected(ScopeCursor *cursor);
//	void sort();
	void posXchanged(VCursor *cursor, qreal x);
	void posYchanged(HCursor *cursor, qreal y);
private slots:
	void cursorRemoved(ScopeCursor * cursor);
	void updateVCursors();
public:
	void clear();
	Qt::ItemFlags flags(const QModelIndex &) const Q_DECL_OVERRIDE;
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

	bool allowEditY() const { return m_allowEditY; }
	void setAllowEditY(bool allowEditY) { m_allowEditY = allowEditY; }
	void setHeader(const QStringList &header);

	QStringList units() const { return m_units; }
	void setUnits(const QStringList &units);

protected:
private:
//	const int m_nY;	// number of y values
	const QVector<int> m_stdRoles;	///< display+edit
	QVector<PointData> m_data;
	QStringList m_header;
	QStringList m_units;
	ChannelGraphVector m_graphs;
	bool m_allowEditY;
	bool validIndex(const QModelIndex &idx) const { return idx.isValid() && (idx.row() < rowCount()) && (idx.column() < columnCount()); }
	int indexOf(ScopeCursor *cursor) const;
	//	static int channel(ScopeCursor *cursor);
	void updateVCursor(int row);
};

inline int PointsModel::rowCount(const QModelIndex &) const
{
	return m_data.size();
}

inline int PointsModel::columnCount(const QModelIndex &) const
{
	return NCols;
}

#endif // POINTSMODEL_H
