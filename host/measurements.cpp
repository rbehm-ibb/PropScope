// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 11/14/2015 by behm
// ******************************************************

#include "measurements.h"

Measurements::Measurements(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(0);
	QGridLayout *grid = new QGridLayout;
	lay->addLayout(grid);
	lay->addStretch();
	int row = 0;
	QLabel *chLab = new QLabel(QString("Channel 0"));
	chLab->setAlignment(Qt::AlignCenter);
	chLab->setStyleSheet("* { border: 2px solid green; margin: 1px; padding: 2px; }");

	grid->addWidget(chLab, row, 1);
	chLab = new QLabel(QString("Channel 1"));
	chLab->setAlignment(Qt::AlignCenter);
	chLab->setStyleSheet("* { border: 2px solid yellow; margin: 1px; padding: 2px; }");
	grid->addWidget(chLab, row, 2);
	grid->addWidget(new QLabel("Avg"), ++row, 0);
	grid->addWidget(m_avg[0] = new DataLabel("V"), row, 1);
	grid->addWidget(m_avg[1] = new DataLabel("V"), row, 2);
	grid->addWidget(new QLabel("RMS"), ++row, 0);
	grid->addWidget(m_rms[0] = new DataLabel("V"), row, 1);
	grid->addWidget(m_rms[1] = new DataLabel("V"), row, 2);
	grid->addWidget(new QLabel("Max"), ++row, 0);
	grid->addWidget(m_max[0] = new DataLabel("V"), row, 1);
	grid->addWidget(m_max[1] = new DataLabel("V"), row, 2);
	grid->addWidget(new QLabel("Min"), ++row, 0);
	grid->addWidget(m_min[0] = new DataLabel("V"), row, 1);
	grid->addWidget(m_min[1] = new DataLabel("V"), row, 2);
	grid->addWidget(new QLabel("PP"),  ++row, 0);
	grid->addWidget(m_pp[0]  = new DataLabel("V"), row, 1);
	grid->addWidget(m_pp[1]  = new DataLabel("V"), row, 2);
	grid->addWidget(new QLabel("Freq"), ++row, 0);
	grid->addWidget(m_freq[0] = new DataLabel("Hz"), row, 1);
	grid->addWidget(m_freq[1] = new DataLabel("Hz"), row, 2);
	grid->addWidget(new QLabel("Period"), ++row, 0);
	grid->addWidget(m_period[0] = new DataLabel("s"), row, 1);
	grid->addWidget(m_period[1] = new DataLabel("s"), row, 2);
}

Measurements::~Measurements()
{
//	delete plot;
//	qDebug() << Q_FUNC_INFO;
}

void Measurements::analogRxd(const ConfigData &config, const AnalogDataSet &data)
{
	calcValues(0, config, data.time, data.ch0);
	calcValues(1, config, data.time, data.ch1);
}

void Measurements::overRange(bool l0, bool h0, bool l1, bool h1)
{
	m_min[0]->setDisabled(l0);
	m_max[0]->setDisabled(h0);
	m_min[1]->setDisabled(l1);
	m_max[1]->setDisabled(h1);
}

void Measurements::calcValues(int ic, const ConfigData &config, const AnalogData &time, const AnalogData &ch)
{
	qreal min;
	qreal max;
	qreal avg;
	qreal rms;
	if (! config.ch(ic).active())
	{
		m_min[ic]->setText("");
		m_max[ic]->setText("");
		m_pp[ic]->setText("");
		m_avg[ic]->setText("");
		m_rms[ic]->setText("");
		m_freq[ic]->setText("");
		m_period[ic]->setText("");

	}
	else
	{
		getMinMax(ch, min, max, avg, rms);
		m_min[ic]->setData(min);
		m_max[ic]->setData(max);
		m_pp[ic]->setData(max - min);
		m_avg[ic]->setData(avg);
		m_rms[ic]->setData(rms);
		if (max - min < config.ch(ic).scale() * 10)
		{
			m_freq[ic]->setText("-");
			m_period[ic]->setText("-");
		}
		else
		{
			qreal freq = getFreq(avg, time, ch) / config.time().unitScale().first;
			m_freq[ic]->setData(freq);
			m_period[ic]->setData(1/freq);
		}
		emit minMax(ic, min, max, avg, rms);
	}
}

void Measurements::getMinMax(const AnalogData d, qreal &min, qreal &max, qreal &avg, qreal &rms) const
{
	min = d[0];
	max = min;
	qreal sum = 0;
	qreal sum2 = 0;
	for (int i = 0; i < d.size(); ++i)
	{
		const qreal di = d[i];
		sum += di;
		sum2 += di * di;
		min = qMin(min, di);
		max = qMax(max, di);
	}
	avg = sum / d.size();
	rms = sqrt(sum2 / d.size());
}

qreal Measurements::getFreq(qreal avg, const AnalogData &time, const AnalogData &ch) const
{
	int cnt = 0;
	qreal last = ch.first();
	bool below = last < avg;
	int pos0 = -1;
	int pos = -1;
	for (int i = 0; i < ch.size(); ++i)
	{
		bool above = ch[i] > avg;
		if (below && above)
		{
			if (pos0 < 0)
			{
				pos0 = i;
			}
			++cnt;
			pos = i;
		}
		below = ch[i] < avg;
	}
	if (cnt < 2)
		return qQNaN();
	qreal t0 = time[pos0];
	qreal t1 = time[pos];
	qreal freq = qreal(cnt - 1) / (t1 - t0);
//	qDebug() << Q_FUNC_INFO << avg << cnt << pos0 << pos << t0 << t1 << freq;
	return freq;
}

#if 0
qreal Measurements::findAuto(const AnalogData d, const qreal avg, qreal rate, QCPGraph *graph)
{
	AnalogData x, y1;
	qreal ac1 = -10000;
	int d1 = 0;
	for (int i = 10; i < d.size()/8*7; ++i)
	{
		qreal ac = autoCorrelate(d, avg, i);
		x.append(qreal(i) * rate);
		y1.append(ac);
		if (ac > ac1)
		{
			ac1 = ac;
			d1 = i;
		}
	}
	graph->setData(x, y1);
	graph->rescaleAxes();
	return d1*rate;
}

qreal Measurements::autoCorrelate(const AnalogData d, const qreal avg, int delta)
{
	qreal sum = 0;
	int length = d.size() - delta;
	for (int i = 0; i < length; ++i)
	{
		sum += (d[i] - avg) * (d[i+delta] - avg);
	}
	return sum / length;
}
#endif
