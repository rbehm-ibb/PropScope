// ******************************************************
// * copyright (C) 2016 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 5/26/2016 by behm
// ******************************************************

#include "pointsmodel.h"
#include "valuedelegate.h"
#include "channelgraph.h"
#include "scopecursor.h"

PointsModel::PointsModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_stdRoles( { Qt::DisplayRole, Qt::EditRole })
	, m_allowEditY(false)
{
	m_header << "Id" << "Time" << "Ch0" << "Ch1";
	m_units << "" << "s" << "V" << "V";
}

PointsModel::~PointsModel()
{
}

void PointsModel::setGraphs(ChannelGraphVector graphs)
{
	m_graphs = graphs;
	foreach (ChannelGraph *g, graphs)
	{
		g->setPointsModel(this);
		connect(g, &ChannelGraph::updated, this, &PointsModel::updateVCursors);
	}
}

QModelIndex PointsModel::addVCursor(VCursor *cursor)
{
	if (insertRow(rowCount()))
	{
		const QModelIndex idx = index(rowCount()-1, Time);
		qreal x = cursor->posX();
		setData(idx, x);
		m_data[idx.row()].cursor = cursor;
		m_data[idx.row()].name = cursor->objectName();
		connect(cursor, &ScopeCursor::deleted, this, &PointsModel::cursorRemoved);
		connect(cursor, &ScopeCursor::cursorSelected, this, &PointsModel::cursorSelected);
		connect(cursor, &VCursor::posXchanged, this, &PointsModel::posXchanged);
//		qDebug() << Q_FUNC_INFO << cursor << x;
		QCPItemTracer *tr = new QCPItemTracer(cursor->parentPlot());
		tr->setInterpolating(true);
		tr->setGraph(m_graphs[0]);
		tr->setGraphKey(x);
		tr->setSelectable(false);
		tr->setPen(cursor->pen());
		tr->setStyle(QCPItemTracer::tsCircle);
		tr->setSize(10);
		tr->setVisible(true);
		m_data[idx.row()].tracer0 = tr;
		tr = new QCPItemTracer(cursor->parentPlot());
		tr->setInterpolating(true);
		tr->setGraph(m_graphs[1]);
		tr->setGraphKey(x);
		tr->setSelectable(false);
		tr->setPen(cursor->pen());
		tr->setStyle(QCPItemTracer::tsCircle);
		tr->setSize(10);
		tr->setVisible(true);
		m_data[idx.row()].tracer1 = tr;
		updateVCursor(idx.row());
		return idx;
	}
	return QModelIndex();
}

QModelIndex PointsModel::addHCursor(HCursor *cursor)
{
	if (insertRow(rowCount()))
	{
		const QModelIndex idx = index(rowCount()-1, Value0 + cursor->graphId());
		m_data[idx.row()].cursor = cursor;
		qreal y = cursor->posY();
		setData(idx, y);
		m_data[idx.row()].name = cursor->objectName();
		connect(cursor, &ScopeCursor::deleted, this, &PointsModel::cursorRemoved);
		connect(cursor, &ScopeCursor::cursorSelected, this, &PointsModel::cursorSelected);
		connect(cursor, &HCursor::posYchanged, this, &PointsModel::posYchanged);
//		qDebug() << Q_FUNC_INFO << cursor << y;
		return idx;
	}
	return QModelIndex();
}

void PointsModel::saveData(QSettings *file)
{
	file->beginWriteArray("cursor", rowCount());
	for (int row = 0; row < rowCount(); ++row)
	{
		file->setArrayIndex(row);
		file->setValue("Id", m_data[row].name);
		file->setValue("Time", m_data[row].time);
		file->setValue("Ch0", m_data[row].y0);
		file->setValue("Ch1", m_data[row].y1);
	}
	file->endArray();
}

//void PointsModel::delPoint(int idx)
//{
//	removeRow(idx);
//}

void PointsModel::cursorSelected(ScopeCursor *cursor)
{
//	qDebug() << Q_FUNC_INFO << cursor;
	int row = indexOf(cursor);
	if (row >= 0)
	{
		emit selected(index(row, 0));
	}

}

void PointsModel::posXchanged(VCursor *cursor, qreal x)
{
	int row = indexOf(cursor);
	if (row >= 0)
	{
		setData(index(row, Time), x);
		m_data[row].tracer0->setGraphKey(x);
		m_data[row].tracer1->setGraphKey(x);
		updateVCursor(row);
	}
}

void PointsModel::updateVCursors()
{
	for (int row = 0; row < rowCount(); ++row)
	{
		updateVCursor(row);
	}
}

void PointsModel::updateVCursor(int row)
{
	QCPItemTracer *tr = m_data[row].tracer0;
	if (tr)
	{
		QCPItemPosition *y0 = tr->position;
		setData(index(row, Value0), y0->coords().y());
	}
	tr = m_data[row].tracer1;
	if (tr)
	{
		QCPItemPosition *y0 = tr->position;
		setData(index(row, Value1), y0->coords().y());
	}
}

void PointsModel::posYchanged(HCursor *cursor, qreal y)
{
	int row = indexOf(cursor);
	if (row >= 0)
	{
		setData(index(row, Value0 + cursor->graphId()), y);
	}
}

void PointsModel::cursorRemoved(ScopeCursor *cursor)
{
	int row = indexOf(cursor);
//	qDebug() << Q_FUNC_INFO << row << cursor;
	if (row >= 0)
	{
//		qDebug() << Q_FUNC_INFO << row << cursor << m_data[row].tracer0 << m_data[row].tracer1;
		m_data[row].cursor = nullptr;
		removeRow(row);
	}
	emit cursorDeleted();
}

void PointsModel::clear()
{
	beginResetModel();
	while (rowCount() > 0)
	{
		removeRow(0);
	}
	endResetModel();
}

Qt::ItemFlags PointsModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags =  Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
	if (index.column() != Id)
	{
		flags |= Qt::ItemIsEditable;
	}
	return flags;
}

QVariant PointsModel::data(const QModelIndex &index, int role) const
{
	if (validIndex(index))
	{
		const PointData &d = m_data[index.row()];
		switch (role)
		{
		case Qt::DisplayRole:
		case Qt::EditRole:
			switch(index.column())
			{
			case Id:
				return d.name;
			case Time:
				return d.time;
			case Value0:
				if (qIsNaN(d.y0))
				{
					return QVariant();
				}
				return d.y0;
			case Value1:
				if (qIsNaN(d.y1))
				{
					return QVariant();
				}
				return d.y1;
			default:
				;
			}
			break;
		case Qt::TextAlignmentRole:
			return Qt::AlignRight;
		}
	}
	return QVariant();
}

bool PointsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
//	qDebug() << Q_FUNC_INFO << index << value << role;
	bool ok;
	qreal v = value.toDouble(&ok);
	if (validIndex(index))
	{
		if (m_stdRoles.contains(role))
		{
			PointData &d = m_data[index.row()];
			switch(index.column())
			{
			case Id:
				if (d.name != value.toString())
				{
					d.name = value.toString();
					emit dataChanged(index, index, m_stdRoles);
				}
				return true;
			case Time:
				if (d.time != v)
				{
					d.time = v;
					emit dataChanged(index, index, m_stdRoles);
					VCursor *vc = qobject_cast<VCursor*>(d.cursor);
					if (vc)
					{
						vc->setX(v);
					}
				}
				return true;
			case Value0:
				if (d.y0 != v)
				{
					d.y0 = v;
					emit dataChanged(index, index, m_stdRoles);
					HCursor *vc = qobject_cast<HCursor*>(d.cursor);
					if (vc)
					{
						vc->setY(v);
					}
				}
				return true;
			case Value1:
				if (d.y1 != v)
				{
					d.y1 = v;
					emit dataChanged(index, index, m_stdRoles);
					HCursor *vc = qobject_cast<HCursor*>(d.cursor);
					if (vc)
					{
						vc->setY(v);
					}
				}
				return true;
			default:	// y value
				return false;
			}
		}
	}
	else
	{
		qWarning() << Q_FUNC_INFO << "bad index";
	}
	return false;
}

QVariant PointsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && (orientation == Qt::Horizontal))
	{
		if (section < m_header.size())
		{
			return m_header[section];
		}
	}
	if (role == Qt::DisplayRole && (orientation == Qt::Vertical))
	{
		return section;
	}
	return QVariant();
}

bool PointsModel::insertRows(int row, int count, const QModelIndex &parent)
{
	row = qBound(0, row, rowCount());
	PointData dummy;
	beginInsertRows(parent, row, row + count - 1);
	m_data.insert(row, count, dummy);
	endInsertRows();
	emit empty(rowCount() == 0);
	return true;
}

bool PointsModel::removeRows(int row, int count, const QModelIndex &parent)
{
//	qDebug() << Q_FUNC_INFO << row << count;
	row = qBound(0, row, rowCount()-1);

	beginRemoveRows(parent, row, row+count-1);
	for (int i = 0; i < count; ++i)
	{
//		qDebug() << Q_FUNC_INFO << i << m_data[row].cursor;
		if (m_data[row].cursor)
			m_data[row].cursor->deleteLater();
		if (m_data[row].tracer0)
			m_data[row].tracer0->deleteLater();
		if (m_data[row].tracer1)
			m_data[row].tracer1->deleteLater();
		m_data.removeAt(row);
	}
	endRemoveRows();
	emit empty(rowCount() == 0);
	return true;
}

void PointsModel::setHeader(const QStringList &header)
{
	m_header = header;
	emit headerDataChanged(Qt::Horizontal, 0, columnCount());
}


void PointsModel::setUnits(const QStringList &units)
{
	m_units = units;
}

int PointsModel::indexOf(ScopeCursor *cursor) const
{
//	qDebug() << Q_FUNC_INFO << cursor;
	for (int row = 0; row < rowCount(); ++row)
	if (m_data[row].cursor == cursor)
	{
		return row;
	}
//	qDebug() << Q_FUNC_INFO << cursor << "not found";
	return -1;
}
