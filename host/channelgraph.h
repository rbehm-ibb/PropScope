// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/24/2015 by behm
// ******************************************************

#ifndef CHANNELGRAPH_H
#define CHANNELGRAPH_H

#include "project.h"
#include "analogdata.h"
#include "configdata.h"

class HCursor;
class VCursor;
class ScopeCursor;

class ChannelGraph;
class PointsModel;

typedef QVector<ChannelGraph*> ChannelGraphVector;

class ChannelGraph : public QCPGraph
{
	Q_OBJECT
public:
	ChannelGraph(uint idx, QColor color, QCPAxis *valueAxis, QCustomPlot *plot);
	~ChannelGraph();
	void init(const ConfigData &conf);
	void setPointsModel(PointsModel *model);
	void setData(const ConfigChannelData &chConf, const AnalogData time, const AnalogData values);
	qreal dataAt(qreal time);
	int idx() const { return m_idx; }
	const QCPDataMap *data() const { return QCPGraph::data(); }

	HCursor *addHCursor(qreal y, QString name = QString(), QPen pen = Qt::NoPen);	// NoPen gives default
	VCursor *addVCursor(qreal x, QString name = QString(), QPen pen = Qt::NoPen);	// NoPen gives default

	QCPRange range() const { return m_range; }
	void setRange(const QCPRange &range);


	QColor color() const { return m_color; }
	static void setAxisColor(QCPAxis *axis, QColor color);

	void clicked(QMouseEvent *event);
	HCursor *trigger() const;

signals:
	void updated();
	void triggerMoved(uint ch, qreal y);
public slots:
	void setTrigger(uint ch, qreal vt);

private slots:
	void stationaryTime();

private:
	const uint m_idx;
	QCPRange m_range;
	const QColor m_color;
	HCursor *m_trigger;
	QTimer *m_widthTimer;
	QPointer<PointsModel> m_model;
};

#endif // CHANNELGRAPH_H
