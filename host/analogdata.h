// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/17/2015 by behm
// ******************************************************

#ifndef ANALOGDATA
#define ANALOGDATA

typedef QVector<qreal> AnalogData;

struct AnalogDataSet
{
	AnalogData time;
	AnalogData ch0;
	AnalogData ch1;
};

#endif // ANALOGDATA

