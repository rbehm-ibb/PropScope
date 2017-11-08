// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 11/14/2015 by behm
// ******************************************************

#include "utils.h"

Utils::Utils()
{
}

QString Utils::formatValue(qreal x, const QString &unit)
{
	if (qIsNaN(x))
	{
		return QString();
	}
	UnitScale unitF = unitPrefix(x, unit);
	x /= unitF.first;
	int prec = 0;
	if (x >= 100)
		prec = 0;
	else if (x >= 10)
		prec = 1;
	else prec = 2;
	return QString::number(x, 'f', prec) + unitF.second;
}

QString Utils::formatValue(qreal x, int prec, const QString &unit)
{
	if (qIsNaN(x))
	{
		return QString();
	}
	UnitScale unitF = unitPrefix(x, unit);
	x /= unitF.first;
	if (x >= 100)
		prec -= 3;
	else if (x >= 10)
		prec -= 2;
	return QString::number(x, 'f', prec) + unitF.second;
}

Utils::UnitScale Utils::unitPrefix(qreal x, const QString unit)
{
	static const struct Prefix
	{
		const char * prfx;
		qreal lower;
		qreal upper;
	} prefix[] =
	{
	{ "p", 1e-12, 1e-9 },
	{ "n", 1e-9, 1e-6 },
	{ "µ", 1e-6, 1e-3 },
	{ "m", 1e-3, 1e0 },
	{ " ", 1e0, 1e3 },
	{ "k", 1e3, 1e6 },
	{ "M", 1e6, 1e9 },
	{ "G", 1e9, 1e12 },
	{ 0, 0, 0 }
	};
	UnitScale result;
	result.first = 1;
	result.second = " ";
	x = qAbs(x);
	for (int i = 0; prefix[i].lower > 0; ++i)
	{
		if (prefix[i].lower <= x && x < prefix[i].upper)
		{
			result.first = prefix[i].lower;
			result.second = QString::fromLatin1(prefix[i].prfx);
			break;
		}
	}
	result.second = " " + result.second + unit;
	return result;
}
