// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/24/2015 by behm
// ******************************************************

#include "channelgraph.h"
#include "scopecursor.h"
#include "pointsmodel.h"

ChannelGraph::ChannelGraph(uint idx, QColor color, QCPAxis *valueAxis, QCustomPlot *plot)
	: QCPGraph(plot->xAxis, valueAxis)
	, m_idx(idx)
	, m_color(color)
{
	m_widthTimer = new QTimer(this);
	m_widthTimer->setSingleShot(true);
	connect(m_widthTimer, &QTimer::timeout, this, &ChannelGraph::stationaryTime);

	setName(QString("Channel %1").arg(m_idx));
	valueAxis->setLabel(name() + "  [V]");
	plot->addPlottable(this);
	QPen pen(m_color);
	pen.setWidthF(1.5);
	setPen(pen);

//	setLineStyle(lsNone);
//	setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlus));

	valueAxis->setRange(-1, 1);	// just give it some inital values

	pen.setWidth(1);
	QColor colSel(color.lighter(120));
	pen.setColor(colSel);
	setSelectedPen(pen);

	setAxisColor(valueAxis, m_color);

	pen.setColor(m_color.darker(200));
	pen.setWidth(0);

	setSelectable(false);

	pen.setColor(m_color);
	pen.setStyle(Qt::DashLine);
	m_trigger = new HCursor(this, m_idx, m_idx == 0 ? ScopeCursor::Left : ScopeCursor::Right, pen);
	m_trigger->setId(m_idx);
	m_trigger->setObjectName(QString("T>"));
	m_trigger->addText(m_trigger->objectName());
	m_trigger->setVisible(false);
	connect(m_trigger, &HCursor::posYchangedId, this, &ChannelGraph::triggerMoved);
}

ChannelGraph::~ChannelGraph()
{
//	qDebug() << Q_FUNC_INFO << name();
	qDeleteAll(parent()->findChildren<ScopeCursor*>());
//	qDeleteAll(m_vcursors);
}

void ChannelGraph::init(const ConfigData &conf)
{
	setRange(conf.ch(m_idx).range());
	setTrigger(conf.trigger().channel(), conf.trigger().level());
}

void ChannelGraph::setPointsModel(PointsModel *model)
{
	m_model = model;
}

void ChannelGraph::setData(const ConfigChannelData &chConf, const AnalogData time, const AnalogData values)
{
	if (chConf.active())
	{
		QPen pen(m_color);
		setPen(pen);
//		setAntialiased(false);
		QCPGraph::setData(time, values);
		setRange(chConf.range());
		emit updated();
	}
	else
	{
		clearData();
	}
	m_widthTimer->start(500);
}

HCursor *ChannelGraph::addHCursor(qreal y, QString name, QPen pen)
{
	static int cnt = 0;
	if (name.isEmpty())
	{
		name = QString("Ch%1#%2 ").arg(m_idx).arg(++cnt);
	}
	if (pen == Qt::NoPen)
	{
		pen = QPen(m_color);
	}
	HCursor *hc = new HCursor(this, m_idx, m_idx == 0 ? ScopeCursor::Left : ScopeCursor::Right, pen);
	hc->setObjectName(name);
	hc->addText(name);
	hc->setY(y);
	if (m_model)
	{
		m_model->addHCursor(hc);
	}
	return hc;

}

VCursor *ChannelGraph::addVCursor(qreal x, QString name, QPen pen)
{
	static int cnt = 0;
	if (name.isEmpty())
	{
		name = QString("%1T ").arg(++cnt);
	}
	if (pen == Qt::NoPen)
	{
		pen = QPen(m_color);
	}
	VCursor *vc = new VCursor(this, pen);
	vc->setObjectName(name);
	vc->addText(name);
	vc->setX(x);
	if (m_model)
	{
//		qDebug() << Q_FUNC_INFO << m_idx << name << m_cursors;
		m_model->addVCursor(vc);
	}
	return vc;
}

void ChannelGraph::setRange(const QCPRange &range)
{
	if (m_range != range)
	{
		valueAxis()->setRange(range);
		m_range = range;
	}
}

void ChannelGraph::setTrigger(uint ch, qreal vt)
{
	if (m_idx == ch)
	{
		m_trigger->setVisible(true);
		m_trigger->setY(vt);
	}
	else
	{
		m_trigger->setVisible(false);
	}
}

void ChannelGraph::stationaryTime()
{
	QPen pen(m_color);
	pen.setWidthF(1.5);
	setPen(pen);
//	setAntialiased(true);
	parentPlot()->replot();
}

void ChannelGraph::setAxisColor(QCPAxis *axis, QColor color)
{
	axis->setVisible(true);
	QPen pen(color);
	axis->setBasePen(pen);
	axis->setTickPen(pen);
	axis->setSubTickPen(pen);
	axis->setLabelColor(color);
	axis->setTickLabelColor(color);

	axis->grid()->setVisible(true);
	pen.setStyle(Qt::DotLine);
	axis->grid()->setPen(pen);
	pen.setStyle(Qt::SolidLine);
	axis->grid()->setZeroLinePen(pen);


	QColor colSel(color.lighter(120));
	pen.setColor(colSel);
	pen.setWidth(3);
	axis->setSelectedBasePen(pen);
	axis->setSelectedTickPen(pen);
	axis->setSelectedSubTickPen(pen);
	axis->setSelectedLabelColor(colSel);
	axis->setSelectedTickLabelColor(colSel);
}

void ChannelGraph::clicked(QMouseEvent *event)
{
//	qreal x = keyAxis()->pixelToCoord(event->localPos().x());
//	qreal y = valueAxis()->pixelToCoord(event->localPos().y());
//	qDebug() << Q_FUNC_INFO << this << event << x << y;
	if (event->button() == Qt::RightButton)
	{
		qreal y = valueAxis()->pixelToCoord(event->localPos().y());
		addHCursor(y);
	}

}

HCursor *ChannelGraph::trigger() const
{
	return m_trigger;
}
