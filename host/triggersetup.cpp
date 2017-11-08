// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/15/2015 by behm
// ******************************************************

#include "triggersetup.h"
#include "selectbox.h"
#include "scopecursor.h"
#include "scopescreen.h"
#include "triggersetup.h"
#include "scopesetup.h"
#include "ledicon.h"

static const SelectBox::ItemsInit initChannel[] =
{
	{ "Ch 0", 0 },
	{ "Ch 1", 1 },
	{ 0, 0}
};

static const SelectBox::ItemsInit initMode[] =
{
	{ "Auto", TriggerData::TrigAuto },
	{ "Norm", TriggerData::TrigNorm },
	{ "Single", TriggerData::TrigSingle },
	{ 0, 0}
};

static const SelectBox::ItemsInit initEdge[] =
{
	{ "Rising", TriggerData::Rising },
	{ "Falling", TriggerData::Falling },
	{ 0, 0}
};


TriggerSetup::TriggerSetup(ScopeScreen *screen, ScopeSetup *setScope, QWidget *parent)
	: QWidget(parent)
	, m_setup(setScope)
	, m_screen(screen)
	, m_trigTimer(new QTimer(this))
{
	QVBoxLayout *vlay = new QVBoxLayout(this);
	QGridLayout *grid = new QGridLayout;
	vlay->addLayout(grid);
	vlay->addStretch();
	grid->setMargin(0);
	grid->setSpacing(0);
	grid->addWidget(new QLabel("Channel"), 0, 0);
	grid->addWidget(m_channel = new SelectBox(initChannel), 1, 0);
	grid->addWidget(new QLabel("Mode"), 0, 1);
	grid->addWidget(m_mode = new SelectBox(initMode), 1, 1);
	grid->addWidget(new QLabel("Edge"), 0, 2);
	grid->addWidget(m_edge = new SelectBox(initEdge), 1, 2);

	grid->addWidget(new QLabel("Level"), 2, 0);
	grid->addWidget(m_level = new QDoubleSpinBox, 2, 1);
	m_level->setRange(-200, 200);
	m_level->setSuffix(" V");
	m_level->setAlignment(Qt::AlignRight);
	m_level->setDecimals(3);
	m_level->setSingleStep(0.01);
	QPushButton *pbHalf = new QPushButton("50%");
	grid->addWidget(pbHalf, 2, 2);

	grid->addWidget(new QLabel("PostTrg"), 3, 0);
	grid->addWidget(m_post = new QSpinBox, 3, 1);
	grid->addWidget(m_active = new LedIcon(this), 3, 2);
	m_active->setText("Triggered");
	m_active->setSize(18);
	m_post->setRange(0, 100);
	m_post->setSingleStep(10);
	m_post->setAlignment(Qt::AlignRight);
	m_post->setSuffix("%");

	connect(m_level, SIGNAL(valueChanged(double)), this, SLOT(triggerValueChanged(qreal)));
	foreach (const ChannelGraph *g, m_screen->channels())
	{
		connect(g->trigger(), &HCursor::posYchangedId, this, &TriggerSetup::triggerMoved);
		connect(this, &TriggerSetup::triggerChanged, g, &ChannelGraph::setTrigger);
	}
	connect(m_channel, SIGNAL(currentIndexChanged(int)), this, SLOT(triggerChannelChanged(int)));
	connect(m_channel, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(m_mode, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(m_edge, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(m_level, SIGNAL(valueChanged(double)), this, SIGNAL(changed()));
	connect(m_post, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
	connect(pbHalf, &QPushButton::clicked, this, &TriggerSetup::set50);

	connect(m_trigTimer, &QTimer::timeout, this, &TriggerSetup::triggeredReset);
	m_trigTimer->setSingleShot(true);
	m_trigTimer->setInterval(500);
}

void TriggerSetup::init(const TriggerData &t)
{
	m_channel->setValue(t.channel());
	m_mode->setValue(t.mode());
	m_edge->setValue(t.edge());
	m_level->setValue(t.level());
	m_post->setValue(t.post());
	changedScaling();
//	qDebug() << Q_FUNC_INFO << t;
}

const TriggerData TriggerSetup::get() const
{
	TriggerData res;
	res.setChannel(m_channel->value());
	res.setMode(TriggerData::Mode(m_mode->value()));
	res.setEdge(TriggerData::Edge(m_edge->value()));
	res.setLevel(m_level->value());
	res.setPost(m_post->value());
//	qDebug() << Q_FUNC_INFO << res;
	return res;
}

void TriggerSetup::triggerMoved(int ch, qreal level)
{
//	qDebug() << Q_FUNC_INFO << ch << level;
	if (ch == m_channel->value())
	{
		m_level->setValue(level);
	}
}

void TriggerSetup::triggerChannelChanged(int /*ch*/)
{
	int ch = m_channel->value();
	foreach (const ChannelGraph *g, m_screen->channels())
	{
		bool thisOne = g->idx() == ch;
		g->trigger()->setVisible(thisOne);
		if (thisOne)
		{
			m_level->setValue(g->trigger()->posY());
			g->trigger()->setSelected(true);
		}
	}
	changedScaling();
}

void TriggerSetup::set50()
{
	m_level->setValue(val50[m_channel->value()]);
}

void TriggerSetup::changedScaling()
{
	const ConfigData cd = m_setup->getSettings();
	qreal r = cd.ch(m_channel->currentIndex()).range().upper;
//	qDebug() << Q_FUNC_INFO << r;
	m_level->setSingleStep(r * 0.01);
}

void TriggerSetup::minMax(int ch, qreal min, qreal max, qreal /*avg*/, qreal /*rms*/)
{
	val50[ch] = (min + max) / 2;
}

void TriggerSetup::triggerred()
{
	m_active->setOnOff(true);
	m_trigTimer->start();
}

void TriggerSetup::triggeredReset()
{
	m_active->setOnOff(false);
}

void TriggerSetup::triggerValueChanged(qreal v)
{
//	qDebug() << Q_FUNC_INFO << v;
	emit triggerChanged(m_channel->value(), v);
}

