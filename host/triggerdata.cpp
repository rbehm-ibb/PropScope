// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/20/2015 by behm
// ******************************************************

#include "triggerdata.h"

TriggerData::TriggerData()
	: m_channel(0)
	, m_mode(TrigAuto)
	, m_edge(Rising)
	, m_post(0)
	, m_level(0)
{
}

void TriggerData::save(QSettings *conf) const
{
//	qDebug() << Q_FUNC_INFO << *this;
	conf->setValue("trigger/channel", m_channel);
	conf->setValue("trigger/mode", m_mode);
	conf->setValue("trigger/edge", m_edge);
	conf->setValue("trigger/level", m_level);
	conf->setValue("trigger/pos", m_post);
}

void TriggerData::load(QSettings *conf)
{
	m_channel = conf->value("trigger/channel", 0).toUInt();
	m_mode = Mode(conf->value("trigger/mode", 0).toUInt());
	m_edge = Edge(conf->value("trigger/edge", 0).toUInt());
	m_level = conf->value("trigger/level", 0).toDouble();
	m_post = conf->value("trigger/pos", 0).toInt();
//	qDebug() << Q_FUNC_INFO << *this;
}


QDebug operator<<(QDebug dbg, const TriggerData &d)
{
	dbg.nospace() << "Trig{ Ch:" << d.m_channel
		      << " M:" << d.m_mode
		      << " E:" << d.m_edge
		      << " L:" << d.m_level
		      << " P:" << d.m_post
		      << " }";
	return dbg.maybeSpace();
}
