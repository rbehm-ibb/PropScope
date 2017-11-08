// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/17/2015 by behm
// ******************************************************

#include "configdata.h"

ConfigData::ConfigData()
	: m_chConf( { ConfigChannelData(0), ConfigChannelData(1) })
{
	setData();
}

void ConfigData::save(QSettings *conf) const
{
//	qDebug() << Q_FUNC_INFO << *this;
	m_timeData.save(conf);
	m_chConf[0].save(conf);
	m_chConf[1].save(conf);
	m_trigger.save(conf);
}

void ConfigData::load(QSettings *conf)
{
	m_timeData.load(conf);
	m_chConf[0].load(conf);
	m_chConf[1].load(conf);
	m_trigger.load(conf);
	setData();
//	qDebug() << Q_FUNC_INFO << *this;
}

ConfigData::PCAFlags ConfigData::pca() const
{
	PCAFlags f(None);
	if (! ch(0).dc())
		f |= AC0;
	switch (ch(0).res())
	{
	case 0:
		break;
	case 1:
		f |= REF0;
		break;
	case 2:
		f |= DIV0;
		break;
	case 3:
		f |= REF0 | DIV0;
		break;
	}

	if (! ch(1).dc())
		f |= AC1;
	switch (ch(1).res())
	{
	case 0:
		break;
	case 1:
		f |= REF1;
		break;
	case 2:
		f |= DIV1;
		break;
	case 3:
		f |= REF1 | DIV1;
		break;
	}
	return f;
}

void ConfigData::setData()
{
}

ConfigChannelData::ConfigChannelData(int ch)
	: m_ch(ch)
	, m_active(true)
	, m_dc(true)
	, m_res(0)
	, m_probe(1)
{
	setData();
}

void ConfigChannelData::setRes(int res)
{
	m_res = res;
	setData();
}


void ConfigChannelData::setProbe(int probe)
{
	m_probe = probe;
	setData();
}

void ConfigChannelData::save(QSettings *conf) const
{
	QString sch(QString("channel%1/").arg(m_ch));
	conf->setValue(sch + "active", m_active);
	conf->setValue(sch + "dc", m_dc);
	conf->setValue(sch + "res", m_res);
	conf->setValue(sch + "probe", m_probe);
}

void ConfigChannelData::load(QSettings *conf)
{
	QString sch(QString("channel%1/").arg(m_ch));
	m_active = conf->value(sch + "active", true).toBool();
	m_dc = conf->value(sch + "dc", true).toBool();
	m_res = conf->value(sch + "res", 0).toInt();
	m_probe = conf->value(sch + "probe", 10).toInt();
	setData();
}

void ConfigChannelData::setData()
{
	static QMap<int, qreal> rangeMap;
	if (rangeMap.isEmpty())
	{
		rangeMap.insert(0, 10);
		rangeMap.insert(1, 5);
		rangeMap.insert(2, 1);
		rangeMap.insert(3, 0.5);
	}
	qreal maxValue = rangeMap.value(m_res, 0) * m_probe;
	m_range.upper = maxValue;
	m_range.lower = -maxValue;
	m_scale = maxValue / 512;
}

QDebug operator<<(QDebug dbg, const ConfigData &d)
{
    dbg.nospace() << "SConf{ D:" << d.m_timeData
		  << d.m_chConf[0] << d.m_chConf[1]
		      << d.m_trigger
		      << " }";
	return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const ConfigChannelData &d)
{
	dbg.nospace() << " Ch{" << d.m_ch << " " << (d.m_dc ? "DC" : "AC") << " R:" << d.m_res
		      << " P:" << d.m_probe << " S:" << d.m_scale << d.m_range << "}";
	return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const QCPRange &d)
{
	dbg.nospace()  << "R(" << d.lower << "," << d.upper << ")";
	return dbg.maybeSpace();

}


QDebug operator<<(QDebug dbg, const ConfigTimeData &d)
{
	dbg.nospace() << "(D:" << d.m_depth << " SR:" << d.m_sampleRate << " T:" << d.m_range << ")";
	return dbg.maybeSpace();

}

ConfigTimeData::ConfigTimeData()
	: m_depth(512)
	, m_sampleRate(1000000)
{
	setData();
}

void ConfigTimeData::setDepth(const uint &depth)
{
	m_depth = depth;
	setData();
}

void ConfigTimeData::setSampleRate(const qreal &sampleRate)
{
	m_sampleRate = sampleRate;
	setData();
}

void ConfigTimeData::save(QSettings *conf) const
{
	conf->setValue("scope/depth", m_depth);
	conf->setValue("scope/rate", m_sampleRate);
	conf->setValue("scope/unit1", m_unitScale.first);
	conf->setValue("scope/unit2", m_unitScale.second);
}

void ConfigTimeData::load(QSettings *conf)
{
	m_depth = conf->value("scope/depth", 512).toUInt();
	m_sampleRate = conf->value("scope/rate", 1000000).toUInt();
	m_unitScale.first = conf->value("scope/unit1").toDouble();
	m_unitScale.second = conf->value("scope/unit2").toString();
}

void ConfigTimeData::setData()
{
	m_range.upper = qreal(m_depth) / qreal(m_sampleRate);
	m_range.lower = -m_range.upper;	//0.0;
}

Utils::UnitScale ConfigTimeData::unitScale() const
{
	return m_unitScale;
}

void ConfigTimeData::setUnitScale(const Utils::UnitScale &unitScale)
{
	m_unitScale = unitScale;
	setData();
}

