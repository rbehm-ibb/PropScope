// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/9/2015 by behm
// ******************************************************

#include "scopescreen.h"
#include "channelgraph.h"
#include "scopecursor.h"

ScopeScreen::ScopeScreen(QWidget *parent)
	: QCustomPlot(parent)
{
	const QColor colorBack(Qt::black) ;
	const QColor colorCh0(Qt::green);
	const QColor colorCh1(Qt::yellow);
	const QColor colorTime(Qt::red);

	setMinimumSize(640, 480);
	setCursor(Qt::ArrowCursor);
	setNoAntialiasingOnDrag(true);
	setBackground(QBrush(colorBack));

	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectPlottables |  QCP::iSelectItems);

	connect(this, &QCustomPlot::selectionChangedByUser, this, &ScopeScreen::selectionChanged);
	connect(this, &QCustomPlot::mousePress, this, &ScopeScreen::mousePressSlot);
	connect(this, &QCustomPlot::mouseRelease, this, &ScopeScreen::mouseReleaseSlot);
	connect(this, &QCustomPlot::mouseWheel, this, &ScopeScreen::mouseWheelSlot);
	connect(this, &QCustomPlot::mouseMove, this, &ScopeScreen::mouseMoveSlot);
	connect(this, &QCustomPlot::plottableClick, this, &ScopeScreen::graphClicked);
	connect(this, &QCustomPlot::itemClick, this, &ScopeScreen::itemClicked);
	connect(this, &QCustomPlot::axisDoubleClick, this, &ScopeScreen::axisDoubleClicked);
	connect(this, &QCustomPlot::axisClick, this, &ScopeScreen::axisClicked);

	m_channel.append(new ChannelGraph(0, colorCh0, yAxis, this));
	m_channel.append(new ChannelGraph(1, colorCh1, yAxis2, this));
	ChannelGraph::setAxisColor(xAxis, colorTime);
	xAxis->setLabel("Time [s]");

	foreach (ChannelGraph *g, m_channel)
	{
		connect(g, &ChannelGraph::triggerMoved, this, &ScopeScreen::triggerMoved);
	}
	axisRect()->setRangeDragAxes(0, 0);

//	qDebug() << Q_FUNC_INFO << endl << yAxis << yAxis->graphs() << yAxis->items() << yAxis->plottables()
//		 << endl << yAxis2 << yAxis2->graphs() << yAxis2->items() << yAxis2->plottables()
//		 << endl << xAxis << xAxis->graphs() << xAxis->items() << xAxis->plottables()
//		    ;
}

void ScopeScreen::init(const ConfigData &conf)
{
	foreach (ChannelGraph *g, m_channel)
	{
		g->init(conf);
	}
}

void ScopeScreen::saveData(QSettings &file)
{
	m_config.save(&file);
	const QCPDataMap *data0 = channels()[0]->data();
	const QCPDataMap *data1 = channels()[1]->data();
	file.beginWriteArray("value", data0->count());
	int i = 0;
	foreach (double k, data0->keys())
	{
		file.setArrayIndex(i++);
		file.setValue("time", k);
		file.setValue("ch0", data0->value(k).value);
		file.setValue("ch1", data1->value(k).value);
	}
	file.endArray();
	qDebug() << "save" << m_config;
}

AnalogDataSet ScopeScreen::loadData(ConfigData &config, QSettings &file)
{
	m_config.load(&file);
	config = m_config;
	int size = file.beginReadArray("value");
	AnalogDataSet d { AnalogData(size), AnalogData(size), AnalogData(size) };
	for (int i = 0; i < size; ++i)
	{
		file.setArrayIndex(i);
		d.time[i] = file.value("time").toDouble();
		d.ch0[i] = file.value("ch0").toDouble();
		d.ch1[i] = file.value("ch1").toDouble();
	}
	file.endArray();
	m_config = config;
	xAxis->setRange(m_config.time().range() /*/ m_config.time().unitScale().first*/);
	xAxis->setLabel(QString("Time [%1]").arg(m_config.time().unitScale().second));
	emit timeUnitChanged(m_config.time().unitScale().first);
	analogRxd(config, d);
	qDebug() << "load" << m_config;
	return d;
}

void ScopeScreen::analogRxd(const ConfigData &config, const AnalogDataSet &data)
{
//	AnalogData tt = m_trigger.evalTrigger(time, ch0, ch1);
	emit timeUnitChanged(m_config.time().unitScale().first);
	m_channel[0]->setData(config.ch(0), data.time, data.ch0);
	m_channel[1]->setData(config.ch(1), data.time, data.ch1);
	if (m_config.time().rangeChanged(config.time()))
	{
		m_config = config;
		xAxis->setRange(m_config.time().range() /*/ m_config.time().unitScale().first*/);
		xAxis->setLabel(QString("Time [%1]").arg(m_config.time().unitScale().second));
	}
	yAxis->setVisible(config.ch(0).active());
	yAxis2->setVisible(config.ch(1).active());
	replot();
}

void ScopeScreen::setTrigger(const TriggerData &td)
{
	triggerChanged(td.channel(), td.level());
}

void ScopeScreen::triggerChanged(uint ch, qreal value)
{
	foreach (ChannelGraph *g, m_channel)
	{
		g->setTrigger(ch, value);
	}
}

void ScopeScreen::redraw()
{
	replot();
}

void ScopeScreen::itemClicked(QCPAbstractItem *item, QMouseEvent *event)
{
	Q_UNUSED(item)
	Q_UNUSED(event)
#if 0
	VCursor *vc = qobject_cast<VCursor*>(item);
	if (vc)
	{
		qDebug() << Q_FUNC_INFO << vc << vc->objectName() << vc->posX() << event;
		return;
	}
	HCursor *hc = qobject_cast<HCursor*>(item);
	if (hc)
	{
		qDebug() << Q_FUNC_INFO << hc << hc->objectName() << hc->posY() << event;
		return;
	}
#endif
}

void ScopeScreen::graphClicked(QCPAbstractPlottable *graph, QMouseEvent *ev)
{
//	qDebug() << Q_FUNC_INFO << graph->name() << ev;
	ChannelGraph *g = qobject_cast<ChannelGraph*>(graph);
	if (g)
	{
		g->clicked(ev);
	}
}

void ScopeScreen::axisDoubleClicked(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event)
{
	Q_UNUSED(part);
	Q_UNUSED(event)
	if (axis->orientation() == Qt::Vertical)
	{
		QList<QCPGraph*> graphs = axis->graphs();
//		qDebug() << Q_FUNC_INFO << event << graphs;
		if (graphs.isEmpty())
		{
			qWarning() << Q_FUNC_INFO << axis << "no graphs";
		}
		else
		{
			ChannelGraph *graph = qobject_cast<ChannelGraph*>(graphs.first());
			if (graph)
			{
				axis->setRange(graph->range());
			}
			else
			{
				qWarning() << Q_FUNC_INFO << axis << "no ChannelGraph";
			}
		}
	}
	else
	{
		axis->setRange(m_config.time().range() /*/ m_config.time().unitScale().first*/);
		axis->setLabel(QString("Time [%1]").arg(m_config.time().unitScale().second));
	}
}

void ScopeScreen::axisClicked(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event)
{
//	qDebug() << Q_FUNC_INFO << axis << part << event;
	if (axis->orientation() == Qt::Horizontal && part == QCPAxis::spAxis && event->button() == Qt::RightButton)
	{
		qreal x = axis->pixelToCoord(event->localPos().x());
		VCursor *vc = channels().first()->addVCursor(x, QString(), QPen(axis->basePen()));
		connect(this, &ScopeScreen::timeUnitChanged, vc, &VCursor::timeUnitChanged);
		vc->timeUnitChanged(m_config.time().unitScale().first);
	}
}



void ScopeScreen::selectionChanged()
{
//	qDebug() << Q_FUNC_INFO
//		 << itemCount() << "items" << mItems
//		 << graphCount() << "graphs"
//		 << plottableCount() << "plottables"
//		    ;
	// make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
	if (xAxis->selectedParts().testFlag(QCPAxis::spAxis) || xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
			xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
	{
		xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
		xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
	}
	// handle axis and tick labels as one selectable object:
	if (yAxis->selectedParts().testFlag(QCPAxis::spAxis) || yAxis->selectedParts().testFlag(QCPAxis::spTickLabels))
	{
		yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
	}
	if (yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
	{
		yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
	}

#if 0
	// synchronize selection of graphs with selection of corresponding legend items:
	for (int i=0; i < graphCount(); ++i)
	{
		QCPGraph *g = graph(i);
		QCPPlottableLegendItem *item = legend->itemWithPlottable(g);
		if (item->selected() || g->selected())
		{
			item->setSelected(true);
			g->setSelected(true);
		}
	}
#endif
	if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
	{
//		qDebug() << Q_FUNC_INFO << "xAxis";
		axisRect()->setRangeDragAxes(xAxis, 0);
	}
	else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
	{
//		qDebug() << Q_FUNC_INFO << "yAxis" << axisRectCount();
		axisRect()->setRangeDragAxes(0, yAxis);
	}
	else if (yAxis2->selectedParts().testFlag(QCPAxis::spAxis))
	{
//		qDebug() << Q_FUNC_INFO << "yAxis2" << axisRectCount();
		axisRect()->setRangeDragAxes(0, yAxis2);
	}
	else
	{
		axisRect()->setRangeDragAxes(0, 0);
	}
	if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
	{
		axisRect()->setRangeZoomAxes(xAxis, 0);
	}
	else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
	{
		axisRect()->setRangeZoomAxes(0, yAxis);
	}
	else if (yAxis2->selectedParts().testFlag(QCPAxis::spAxis))
	{
		axisRect()->setRangeZoomAxes(0, yAxis2);
	}
	else
	{
		axisRect()->setRangeZoomAxes(0, 0);
	}
}

void ScopeScreen::mousePressSlot(QMouseEvent *event)
{
//	qDebug() << Q_FUNC_INFO << event;
	Q_UNUSED(event)
}

void ScopeScreen::mouseReleaseSlot(QMouseEvent *event)
{
//	qDebug() << Q_FUNC_INFO << event;
	Q_UNUSED(event)
}

void ScopeScreen::mouseMoveSlot(QMouseEvent *event)
{
//	qDebug() << Q_FUNC_INFO << event;
//	Q_UNUSED(event)
//	qDebug() << Q_FUNC_INFO << itemCount() << event;

	foreach (ScopeCursor *cur, findChildren<ScopeCursor*>())
	{
		if (cur->selected())
		{
			cur->mouseMoveEvent(event);
		}
	}
}

void ScopeScreen::mouseWheelSlot(QWheelEvent *event)
{
//	qDebug() << Q_FUNC_INFO << event;
	Q_UNUSED(event)
}
