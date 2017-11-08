// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/9/2015 by behm
// ******************************************************

#ifndef CHANNELSETUP_H
#define CHANNELSETUP_H

#include "configdata.h"

class SelectBox;

class ChannelSetup : public QObject
{
	Q_OBJECT
public:
	explicit ChannelSetup(int channel, QGridLayout *grid, int row);

	ConfigChannelData getSettings() const;
	void init(const ConfigChannelData &conf);
signals:
	void changed();
private slots:
	void probeChanged(bool div10);
private:
	const int m_channel;
	QCheckBox *m_acdc;
	SelectBox *m_res;
	QCheckBox *m_probe10;
	QCheckBox *m_active;
};

#endif // CHANNELSETUP_H
