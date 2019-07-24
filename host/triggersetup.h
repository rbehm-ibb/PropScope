// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/15/2015 by behm
// ******************************************************

#ifndef TRIGGERSETUP_H
#define TRIGGERSETUP_H

#include "triggerdata.h"
#include "propscope.h"
#include "channelgraph.h"

class SelectBox;
class ScopeScreen;
class ScopeSetup;
class LedIcon;

class TriggerSetup : public QWidget
{
	Q_OBJECT
public:
	explicit TriggerSetup(ScopeScreen *screen, ScopeSetup *setScope, QWidget *parent = nullptr);
	void init(const TriggerData &t);
	const TriggerData get() const;
signals:
	void changed();
	void triggerChanged(uint ch, qreal value);
public slots:
	void changedScaling();
	void minMax(int ch, qreal min, qreal max, qreal avg, qreal rms);
	void triggerred();
private slots:
	void triggerValueChanged(qreal v);
	void triggerMoved(int ch, qreal level);
	void triggerChannelChanged(int ch);
	void set50();
	void triggeredReset();
private:
	ScopeSetup *m_setup;
	SelectBox *m_channel;
	SelectBox *m_mode;
	SelectBox *m_edge;
	QDoubleSpinBox *m_level;
	QSpinBox *m_post;
	QPointer<ScopeScreen> m_screen;
	qreal val50[2];
	LedIcon *m_active;
	QTimer *m_trigTimer;
};

#endif // TRIGGERSETUP_H
