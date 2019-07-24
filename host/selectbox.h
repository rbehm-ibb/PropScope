// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/18/2015 by behm
// ******************************************************

#ifndef SELECTBOX_H
#define SELECTBOX_H

class SelectBox : public QComboBox
{
	Q_OBJECT
public:
	struct ItemsInit
	{
		const char *name;
		int value;
	};
	SelectBox(QWidget *parent = nullptr);
	SelectBox(const SelectBox::ItemsInit items[], QWidget *parent = nullptr);
	void setItems(const ItemsInit items[]);
	int value() const;
signals:
	void valueChanged(int value);
public slots:
	void setValue(int n);
private slots:
	void currentIndexChangedSlot(int index);
};

#endif // SELECTBOX_H
