// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/21/2015 by behm
// ******************************************************

#include "scopecursor.h"
#include "datalabel.h"
#include "utils.h"

ScopeCursor::ScopeCursor(QCPGraph *graph, ScopeCursor::Side side, QPen pen)
	: QCPItemStraightLine(graph->parentPlot())
	, m_side(side)
	, m_id(0)
	, m_text(0)
{
	if (parentPlot())
	{
//		parentPlot()->addItem(this);
	}
	point1->setAxes(graph->keyAxis(), graph->valueAxis());
	point1->setType(QCPItemPosition::ptPlotCoords);
	point2->setAxes(graph->keyAxis(), graph->valueAxis());
	point2->setType(QCPItemPosition::ptPlotCoords);
	setPen(pen);
//	pen.setColor(pen.color().lighter(150));
	pen.setWidth(3);
	setSelectedPen(pen);
}

ScopeCursor::~ScopeCursor()
{
//	qDebug() << Q_FUNC_INFO << objectName();
	emit deleted(this);
	if (m_text)
	{
		delete m_text;
//		m_text = nullptr;
	}
}

void ScopeCursor::addText(const QString baseText)
{
	m_baseText = baseText;
//	qDebug() << Q_FUNC_INFO << text << m_baseText;
	if (m_text)
	{
		qWarning() << Q_FUNC_INFO << "already has text";
	}
	m_text = new QCPItemText(parentPlot());
	QColor color = pen().color();
	m_text->setPen(QPen(color));
	m_text->setBrush(Qt::NoBrush);
	m_text->setColor(color);
	m_text->setText(m_baseText);
	QFont font("helvetica", 12);
	m_text->setFont(font);
	m_text->setClipToAxisRect(false);
	switch (m_side)
	{
	case Left:
		m_text->position->setParentAnchor(point1);
		m_text->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
		break;
	case Right:
		m_text->position->setParentAnchor(point2);
		m_text->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		break;
	case Top:
		m_text->position->setParentAnchor(point2);
		m_text->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		m_text->setRotation(90);
		break;
	case Bottom:
		m_text->position->setParentAnchor(point1);
		m_text->setRotation(90);
//		m_text->setPositionAlignment(Qt::AlignRight /*| Qt::AlignVCenter*/);
		m_text->setPositionAlignment(Qt::AlignRight | Qt::AlignBottom);
		break;
	}
}

void ScopeCursor::setText(const QString text)
{
	if (! m_text)
	{
		addText(objectName());
	}
	if (! m_baseText.isEmpty() && ! m_baseText.endsWith(' ') && m_baseText.endsWith('\n'))
	{
		m_text->setText(m_baseText + " " + text);
	}
	else
	{
		if (m_baseText != "T>")
			m_text->setText(m_baseText + text);
	}
//	qDebug() << Q_FUNC_INFO << objectName() << m_baseText << text << m_text->text();
}

void ScopeCursor::setVisible(bool on)
{
	QCPItemStraightLine::setVisible(on);
	if (m_text)
	{
		m_text->setVisible(on);
	}
	if (! on)
	{
		setSelected(false);
	}
}

void ScopeCursor::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
	QCPItemStraightLine::selectEvent(event, additive, details, selectionStateChanged);
	emit cursorSelected(this);
//	qDebug() << Q_FUNC_INFO << objectName() << event << additive << details << event->button();
	if (event->button() == Qt::RightButton)
	{
		deleteLater();
	}
	else
	{
	}
}

HCursor::HCursor(QCPGraph *graph, int gid, Side side, QPen pen)
	: ScopeCursor(graph, side, pen)
	, m_graphId(gid)
{
	setY(0);
	setSelectable(true);
}

void HCursor::setY(qreal y)
{
	QCPAxis *xAxis = point1->keyAxis();
	qreal left = xAxis->range().lower;
	qreal right = xAxis->range().upper;
	qreal lastY = point1->coords().y();
	point1->setCoords(left, y);
	point2->setCoords(right, y);
	if (parentPlot() != nullptr)
	{
		parentPlot()->replot();
	}
	if (y != lastY)
	{
		emit posYchanged(this, y);
		emit posYchangedId(m_graphId, y);
//		if (m_text)
//		{
//			setText(Utils::formatValue(y, "V"));
//		}
	}
}

void HCursor::mouseMoveEvent(QMouseEvent *event)
{
	if (! visible())
	{
		setSelected(false);
	}
//	qDebug() << Q_FUNC_INFO << objectName() << selected();
	if (selected())
	{
		if (event->buttons() == Qt::LeftButton)
		{
			qreal newY = point1->valueAxis()->pixelToCoord(event->localPos().y());
			setY(newY);
		}
	}
}


VCursor::VCursor(QCPGraph *graph, QPen pen)
    : ScopeCursor(graph, Bottom, pen)
{
	setX(0);
}

void VCursor::setX(qreal x)
{
	QCPAxis *yAxis = point1->valueAxis();
	qreal lastX = point1->coords().x();
	qreal lower = yAxis->range().lower;
	qreal upper = yAxis->range().upper;
	point1->setCoords(x, lower);
	point2->setCoords(x, upper);
	if (x != lastX)
	{
//		qDebug() << Q_FUNC_INFO << objectName() << x;
		emit posXchanged(this, x);
//		if (m_text)
//		{
//			QString txt = Utils::formatValue(x * m_unitScale, "s");
//			qDebug() << Q_FUNC_INFO << x << m_unitScale << x*m_unitScale << txt;
//			setText(txt);
//		}
	}
}

void VCursor::mouseMoveEvent(QMouseEvent *event)
{
	if (! visible())
	{
		setSelected(false);
	}
	if (selected())
	{
		if (event->buttons() == Qt::LeftButton)
		{
			qreal newX = point1->keyAxis()->pixelToCoord(event->localPos().x());
			setX(newX);
		}
	}
}

void VCursor::timeUnitChanged(qreal unitScale)
{
	if (m_unitScale != unitScale)
	{
		m_unitScale = unitScale;
//		qreal x = point1->coords().x();
//		QString txt = Utils::formatValue(x * m_unitScale, "s");
//		setText(txt);
	}
}


