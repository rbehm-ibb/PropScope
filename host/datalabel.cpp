// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 11/14/2015 by behm
// ******************************************************

#include "datalabel.h"
#include "utils.h"

DataLabel::DataLabel(QString unit, QWidget *parent)
	: QLabel(parent)
	, m_unit(unit)
{
	setAlignment(Qt::AlignRight);
}

void DataLabel::setData(qreal v)
{
	if (qIsNaN(v))
	{
		setText("-");
	}
	else
	{
		setText(Utils::formatValue(v, m_unit));
	}
}
