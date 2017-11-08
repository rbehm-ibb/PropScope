// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 11/14/2015 by behm
// ******************************************************

#ifndef UTILS_H
#define UTILS_H

class Utils
{
public:
	Utils();
	static QString formatValue(qreal x, const QString &unit);
	static QString formatValue(qreal x, int precision, const QString &unit);
	typedef QPair<qreal,QString> UnitScale;
	static UnitScale unitPrefix(qreal x, const QString unit);
};

#endif // UTILS_H
