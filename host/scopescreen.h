// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/9/2015 by behm
// ******************************************************

#ifndef SCOPESCREEN_H
#define SCOPESCREEN_H

#include "utils.h"
#include "analogdata.h"
#include "configdata.h"
#include "channelgraph.h"

class ScopeScreen : public QCustomPlot
{
	Q_OBJECT
public:
	ScopeScreen(QWidget *parent = nullptr);
	ChannelGraphVector channels() const { return m_channel; }
	void saveData(QSettings &file);
	AnalogDataSet loadData(ConfigData &config, QSettings &file);
signals:
	void cursorPos(int id, QCPData data);
	void triggerMoved(uint ch, qreal level);
	void timeUnitChanged(qreal unitScale);
public slots:
	void init(const ConfigData &conf);
	void analogRxd(const ConfigData &config, const AnalogDataSet &data);
	void setTrigger(const TriggerData &td);
	void triggerChanged(uint ch, qreal value);
	void redraw();
private slots:
	void mouseWheelSlot(QWheelEvent *event);
	void mousePressSlot(QMouseEvent *event);
	void mouseReleaseSlot(QMouseEvent *event);
	void mouseMoveSlot(QMouseEvent *event);
//	void mouseDoubleClickSlot(QMouseEvent *event);
	void selectionChanged();
	void itemClicked(QCPAbstractItem *item, QMouseEvent *event);
	void graphClicked(QCPAbstractPlottable *graph, QMouseEvent *ev);
	void axisDoubleClicked(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event);
	void axisClicked(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event);

private:
	ConfigData m_config;
	static const int N_CHANNEL = 2;
	ChannelGraphVector m_channel;
};

#endif // SCOPESCREEN_H
