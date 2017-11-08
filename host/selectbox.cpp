// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/18/2015 by behm
// ******************************************************

#include "selectbox.h"

SelectBox::SelectBox(QWidget *parent)
	: QComboBox(parent)
{
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChangedSlot(int)));
}

SelectBox::SelectBox(const SelectBox::ItemsInit items[], QWidget *parent)
	: QComboBox(parent)
{
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChangedSlot(int)));
	setItems(items);
}

void SelectBox::setItems(const SelectBox::ItemsInit items[])
{
	int cix = currentIndex();
	clear();
	for (int i = 0; items[i].name; ++i)
	{
		addItem(items[i].name, items[i].value);
	}
	setCurrentIndex(cix);
}

int SelectBox::value() const
{
	return currentData().toInt();
}

void SelectBox::setValue(int n)
{
	int i = findData(n);
	if (i < 0)
	{
//		qWarning() << Q_FUNC_INFO << objectName() << n;
		setCurrentText(QString::number(n));
	}
	else
	{
		setCurrentIndex(i);
	}
}

void SelectBox::currentIndexChangedSlot(int index)
{
	Q_UNUSED(index)
	emit valueChanged((value()));
}
