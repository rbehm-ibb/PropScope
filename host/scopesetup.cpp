// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/10/2015 by behm
// ******************************************************

#include "scopesetup.h"
#include "selectbox.h"
#include "channelsetup.h"
#include "scopescreen.h"

static const SelectBox::ItemsInit itemsFreq[] =
{
	{ "1Hz", 1},
	{ "2Hz", 2},
	{ "5Hz", 5},
	{ "10Hz", 10 },
	{ "20Hz", 20},
	{ "50Hz", 50},
	{ "100Hz", 100 },
	{ "200Hz", 200 },
	{ "500Hz", 500 },
	{ "1kHz", 1000},
	{ "2kHz", 2000},
	{ "5kHz", 5000},
	{ "10kHz", 10000 },
	{ "20kHz", 20000 },
	{ "50kHz", 50000 },
	{ "100kHz", 100000 },
	{ "200kHz", 200000 },
	{ "500kHz", 500000 },
	{ "1MHz", 1000000},
//	{ "2MHz", 2000000 },
	{ 0, 0 }
};

static const SelectBox::ItemsInit itemsDepth[] =
{
	{ "10", 10},
	{ "100", 100},
	{ "200", 200},
	{ "300", 300},
	{ "400", 400},
	{ "500", 500},
	{ "1000", 1000},
	{ "1500", 1500},
	{ "2000", 2000},
	{ "2500", 2500},
	{ "3000", 3000},
	{ 0, 0 }
};

ScopeSetup::ScopeSetup(PropScope *scope, ScopeScreen *screen, QWidget *parent)
	: QWidget(parent)
{
	Q_UNUSED(screen)
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(0);
	QGridLayout *grid = new QGridLayout;
	lay->addLayout(grid);

	int row = 0;
	grid->addWidget(new QLabel("Sampl."), row, 0);
	grid->addWidget(m_freq = new QSpinBox, row, 1);
	grid->addWidget(m_freqCb = new SelectBox(itemsFreq), row, 2);
	m_freq->setRange(1, 10000000);

	++row;
	grid->addWidget(new QLabel("Depth"), row, 0);
	grid->addWidget(m_depth = new QSpinBox, row, 1);
	grid->addWidget(m_depthCb = new SelectBox(itemsDepth), row, 2);
	m_depth->setRange(10, 4096);

	++row;
	grid->addWidget(new QLabel("Range"), row+1, 0);
	grid->addWidget(new QLabel("AC/DC"), row+2, 0);
	grid->addWidget(new QLabel("Probe"), row+3, 0);
	m_ch0 = new ChannelSetup(0, grid, row);
	m_ch1 = new ChannelSetup(1, grid, row);
	row += 4;

	connect(m_freq, SIGNAL(valueChanged(int)), m_freqCb, SLOT(setValue(int)));
	connect(m_freqCb, SIGNAL(valueChanged(int)), m_freq, SLOT(setValue(int)));
	connect(m_depth, SIGNAL(valueChanged(int)), m_depthCb, SLOT(setValue(int)));
	connect(m_depthCb, SIGNAL(valueChanged(int)), m_depth, SLOT(setValue(int)));
	connect(scope, &PropScope::analogRxd, this, &ScopeSetup::analogRxd);

	connect(m_freq, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
	connect(m_depth, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
	connect(m_ch0, &ChannelSetup::changed, this, &ScopeSetup::changed);
	connect(m_ch1, &ChannelSetup::changed, this, &ScopeSetup::changed);
	init(scope->config());

//	grid = new QGridLayout;
//	lay->addLayout(grid);
	grid->addWidget(new QLabel("Resol"), ++row, 0);
	grid->addWidget(m_step[0] = new DataLabel("V"), row, 1);
	grid->addWidget(m_step[1] = new DataLabel("V"), row, 2);
	grid->addWidget(new QLabel("Sample"), ++row, 0);
	grid->addWidget(m_sampleDiff = new DataLabel("s"), row, 1);
	grid->addWidget(m_sampleFreq = new DataLabel("Hz"), row, 2);
	grid->addWidget(new QLabel("Total"), ++row, 0);
	grid->addWidget(m_sampleDepth = new DataLabel(""), row, 1);
	grid->addWidget(m_time = new DataLabel("s"), row, 2);

	lay->addStretch();
}

void ScopeSetup::init(const ConfigData &conf)
{
	m_freq->setValue(conf.time().sampleRate());
	m_depth->setValue(conf.time().depth());
	m_ch0->init(conf.ch(0));
	m_ch1->init(conf.ch(1));
}

const ConfigData ScopeSetup::getSettings() const
{
	ConfigData conf;
	conf.ch(0) = m_ch0->getSettings();
	conf.ch(1) = m_ch1->getSettings();
	conf.time().setSampleRate(m_freq->value());
	conf.time().setDepth(m_depth->value());
	return conf;
}

void ScopeSetup::analogRxd(const ConfigData &config, const AnalogDataSet &data)
{
	m_step[0]->setData(config.ch(0).scale());
	m_step[1]->setData(config.ch(1).scale());
	m_sampleDepth->setText(QString::number(data.time.size()));
	m_time->setData(config.time().depth() / config.time().sampleRate());
	m_sampleDiff->setData(1. / qreal(config.time().sampleRate()));
	m_sampleFreq->setData(qreal(config.time().sampleRate()));

}

//void ScopeSetup::freqSelectChange(int value)
//{
//	qDebug() << Q_FUNC_INFO << value;
//	m_freq->setValue(value);
//	emit changed();
//}

//void ScopeSetup::freqSpinChange(int value)
//{
//	qDebug() << Q_FUNC_INFO << value;
//	m_freqCb->setValue(value);
//	emit changed();
//}
