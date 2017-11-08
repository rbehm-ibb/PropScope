// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/9/2015 by behm
// ******************************************************

#include "channelsetup.h"
#include "selectbox.h"

static const SelectBox::ItemsInit initRes1[] =
{
	{ "10 V", 0 },
	{ "5 V", 1 },
	{ "1 V", 2 },
	{ "0.5 V", 3 },
	{ 0, 0}
};

static const SelectBox::ItemsInit initRes10[] =
{
	{ "100 V", 0 },
	{ "50 V", 1 },
	{ "10 V", 2 },
	{ "5 V", 3 },
	{ 0, 0}
};

ChannelSetup::ChannelSetup(int channel, QGridLayout *grid, int row)
	: m_channel(channel)
{
	m_res = new SelectBox(initRes1);
	m_acdc = new QCheckBox("AC");
	m_probe10 = new QCheckBox("div 10");
	m_active = new QCheckBox("Show");
	QLabel *chLab = new QLabel(QString("Channel %1").arg(channel));
	chLab->setAlignment(Qt::AlignCenter);
	QString color = QStringList( { "green", "yellow" })[channel];
	chLab->setStyleSheet(QString("* { border: 2px solid %1; margin: 1px; padding: 2px; }").arg(color));
	grid->addWidget(chLab, row+0, m_channel+1);
	grid->addWidget(m_res, row+1, m_channel+1);
	grid->addWidget(m_acdc, row+2, m_channel+1);
	grid->addWidget(m_probe10, row+3, m_channel+1);
	grid->addWidget(m_active, row+4, m_channel+1);
	connect(m_res, &SelectBox::valueChanged, this, &ChannelSetup::changed);
	connect(m_acdc, &QCheckBox::toggled, this, &ChannelSetup::changed);
	connect(m_probe10, &QCheckBox::toggled, this, &ChannelSetup::changed);
	connect(m_probe10, &QCheckBox::toggled, this, &ChannelSetup::probeChanged);
	connect(m_active, &QCheckBox::toggled, this, &ChannelSetup::changed);
}

ConfigChannelData ChannelSetup::getSettings() const
{
	ConfigChannelData conf(m_channel);
	conf.setActive(m_active->isChecked());
	conf.setDc(! m_acdc->isChecked());
	conf.setRes(m_res->value());
	conf.setProbe(m_probe10->isChecked() ? 10 : 1);
	return conf;
}

void ChannelSetup::init(const ConfigChannelData &conf)
{
	m_active->setChecked(conf.active());
	m_acdc->setChecked(! conf.dc());
	m_res->setValue(conf.res());
	bool probe10 = conf.probe() == 10;
	m_probe10->setChecked(probe10);
	probeChanged(probe10);
}

void ChannelSetup::probeChanged(bool div10)
{
	m_res->setItems(div10 ? initRes10 : initRes1);
}
