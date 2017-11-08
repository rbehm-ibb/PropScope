// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 11/14/2015 by behm
// ******************************************************

#ifndef DATALABEL_H
#define DATALABEL_H

class DataLabel : public QLabel
{
	Q_OBJECT
public:
	DataLabel(QString unit, QWidget *parent = 0);
	void setData(qreal v);
	void clear() { setText(QString()); }
protected:
	const QString m_unit;
};

#endif // DATALABEL_H
