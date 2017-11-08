// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/10/2015 by behm
// ******************************************************

#ifndef SCOPESETUP_H
#define SCOPESETUP_H

#include "propscope.h"
#include "datalabel.h"

class SelectBox;
class ChannelSetup;
class TriggerSetup;
class ScopeScreen;

class ScopeSetup : public QWidget
{
	Q_OBJECT
public:
	explicit ScopeSetup(PropScope *scope, ScopeScreen *screen, QWidget *parent = 0);
	void init(const ConfigData &conf);
	const ConfigData getSettings() const;
	void save(QSettings &conf) const { getSettings().save(&conf); }
signals:
	void changed();
public slots:
private slots:
	void analogRxd(const ConfigData &config, const AnalogDataSet &data);
//	void freqSelectChange(int value);
//	void freqSpinChange(int value);
private:
	ChannelSetup *m_ch0;
	ChannelSetup *m_ch1;
	QSpinBox *m_freq;
	SelectBox *m_freqCb;
	QSpinBox *m_depth;
	SelectBox *m_depthCb;
	DataLabel *m_step[2];
	DataLabel *m_time;
	DataLabel *m_sampleDiff;
	DataLabel *m_sampleFreq;
	DataLabel *m_sampleDepth;
};

#endif // SCOPESETUP_H
