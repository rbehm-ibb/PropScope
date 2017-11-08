// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 11/14/2015 by behm
// ******************************************************

#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include "utils.h"
#include "analogdata.h"
#include "configdata.h"
#include "datalabel.h"

class Measurements : public QWidget
{
	Q_OBJECT
public:
	explicit Measurements(QWidget *parent = 0);
	~Measurements();
signals:
	void minMax(int ch, qreal min, qreal max, qreal avg, qreal rms);
public slots:
	void analogRxd(const ConfigData &config, const AnalogDataSet &data);
	void overRange(bool l0, bool h0, bool l1, bool h1);
private:
	DataLabel *m_min[2];
	DataLabel *m_max[2];
	DataLabel *m_pp[2];
	DataLabel *m_avg[2];
	DataLabel *m_rms[2];
	DataLabel *m_freq[2];
	DataLabel *m_period[2];
	void calcValues(int ic, const ConfigData &config, const AnalogData &time, const AnalogData &ch);
	void getMinMax(const AnalogData d, qreal &min, qreal &max, qreal &avg, qreal &rms) const;
	qreal getFreq(qreal avg, const AnalogData &time, const AnalogData &ch0) const;
//	qreal autoCorrelate(const AnalogData d, const qreal avg, int delta);
//	qreal findAuto(const AnalogData d, const qreal avg, qreal rate, QCPGraph *graph);
};

#endif // MEASUREMENTS_H
