// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/20/2015 by behm
// ******************************************************

#ifndef TRIGGERDATA_H
#define TRIGGERDATA_H

class TriggerData
{
	Q_GADGET
	friend QDebug operator<<(QDebug dbg, const TriggerData &d);
public:
	enum Mode { TrigAuto, TrigNorm, TrigSingle };
	enum Edge { Rising, Falling };
	Q_ENUM(Mode)
	Q_ENUM(Edge)
	TriggerData();
	uint channel() const { return m_channel; }
	void setChannel(uint channel) { m_channel = channel; }

	Mode mode() const { return m_mode; }
	void setMode(const Mode &mode) { m_mode = mode; }

	Edge edge() const { return m_edge; }
	void setEdge(const Edge &edge) { m_edge = edge; }

	qreal level() const { return m_level; }
	void setLevel(const qreal &level) { m_level = level; }

	void save(QSettings *conf) const;
	void load(QSettings *conf);

	int post() const { return m_post; }
	void setPost(int post) { m_post = post; }

private:
	uint m_channel;
	Mode m_mode;
	Edge m_edge;
	int m_post;
	qreal m_level;
};

#endif // TRIGGERDATA_H
