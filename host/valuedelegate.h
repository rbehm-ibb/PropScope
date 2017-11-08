// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 1/30/2016 by behm
// ******************************************************

#ifndef VALUEDELEGATE_H
#define VALUEDELEGATE_H

#include "utils.h"

class ValueDelegate : public QStyledItemDelegate
{
public:
	Q_OBJECT
public:
	ValueDelegate(QString unit, int decimals, QObject *parent);
	~ValueDelegate();
	// QAbstractItemDelegate interface
public:
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual QString displayText(const QVariant &value, const QLocale &locale) const;
	virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const;
	virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const;
protected:
	const QString m_unit;
	const int m_decimals;
};

#endif // VALUEDELEGATE_H
