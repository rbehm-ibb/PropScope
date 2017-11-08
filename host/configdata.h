// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/17/2015 by behm
// ******************************************************

#ifndef CONFIGDATA_H
#define CONFIGDATA_H

#include "triggerdata.h"
#include "utils.h"

QDebug operator<<(QDebug dbg, const QCPRange &d);

class ConfigChannelData
{
	friend QDebug operator<<(QDebug dbg, const ConfigChannelData &d);
public:
	ConfigChannelData(int ch);
	int ch() const { return m_ch; }

	bool dc() const { return m_dc; }
	void setDc(bool dc) { m_dc = dc; }

	int res() const { return m_res; }
	void setRes(int res);

	int probe() const { return m_probe; }
	void setProbe(int probe);

	qreal scale() const { return m_scale; }
	QCPRange range() const { return m_range; }
	bool rangeChanged(const ConfigChannelData &d) const { return m_range != d.m_range; }

	void save(QSettings *conf) const;
	void load(QSettings *conf);

	bool active() const { return m_active; }
	void setActive(bool active) { m_active = active; }

private:
	void setData();
	int m_ch;	/// 0 or 1
	bool m_active;
	bool m_dc;	/// DC
	int m_res;	/// resolution settings 0..3
	int m_probe;	/// probe attenuation 1 or 10
	qreal m_scale;	/// Volt/lsb, derived from other data
	QCPRange m_range;
};

class ConfigTimeData
{
	friend QDebug operator<<(QDebug dbg, const ConfigTimeData &d);
public:
	ConfigTimeData();

	QCPRange range() const { return m_range; }
	void setRange(qreal low, qreal high) { m_range.lower = low; m_range.upper = high; }

	uint depth() const { return m_depth; }
	void setDepth(const uint &depth);

	qreal sampleRate() const { return m_sampleRate; }
	void setSampleRate(const qreal &sampleRate);

	Utils::UnitScale unitScale() const;
	void setUnitScale(const Utils::UnitScale &unitScale);

	bool rangeChanged(const ConfigTimeData &d) const { return m_range != d.m_range; }

	void save(QSettings *conf) const;
	void load(QSettings *conf);

private:
	void setData();
	uint m_depth;		/// sample depth
	quint32 m_sampleRate;	/// samples/s
	QCPRange m_range;
	Utils::UnitScale m_unitScale;
};

class ConfigData
{
	friend QDebug operator<<(QDebug dbg, const ConfigData &d);
public:
	enum PCAFlag { None = 0, AC0 = 0x01, AC1 = 0x02, DIV0 = 0x04, DIV1 = 0x08,
		   REF0 = 0x10, REF1 = 0x20 };
	Q_DECLARE_FLAGS(PCAFlags, PCAFlag)
	ConfigData();
	ConfigChannelData &ch(int ch) { return m_chConf[ch]; }
	const ConfigChannelData &ch(int ch) const { return m_chConf[ch]; }
	ConfigTimeData &time() { return m_timeData; }
	const ConfigTimeData &time() const { return m_timeData; }

	TriggerData trigger() const { return m_trigger; }
	void setTrigger(const TriggerData &trigger) { m_trigger = trigger; }

	void save(QSettings *conf) const;
	void load(QSettings *conf);
	void setData();

	PCAFlags pca() const;
private:
	ConfigChannelData m_chConf[2];	/// for each channel
	ConfigTimeData m_timeData;
	TriggerData m_trigger;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ConfigData::PCAFlags)
#endif // CONFIGDATA_H
