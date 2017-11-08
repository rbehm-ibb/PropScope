// ******************************************************
// * copyright (C) 2016 by Becker Electronics Taiwan Ltd.
// * All Rights reserved
// * created 1/30/2016 by behm
// ******************************************************

#include "valuedelegate.h"
#include "utils.h"

ValueDelegate::ValueDelegate(QString unit, int decimals, QObject *parent)
	: QStyledItemDelegate(parent)
	, m_unit(unit)
	, m_decimals(decimals)
{
}

ValueDelegate::~ValueDelegate()
{
//	qDebug() << Q_FUNC_INFO;
}

QSize ValueDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QString text = displayText(index.data(), option.locale);
	QRect tr = option.fontMetrics.boundingRect(text);
//	qDebug() << Q_FUNC_INFO << text << tr;
	return tr.size() + option.decorationSize;
}

QString ValueDelegate::displayText(const QVariant &value, const QLocale &/*locale*/) const
{
	bool ok;
	qreal v = value.toDouble(&ok);
	if (! ok)
	{
		return "-?-";
	}
	if (qIsNaN(v))
	{
		return QString();
	}
	return Utils::formatValue(v, m_decimals, m_unit);
}

QWidget *ValueDelegate::createEditor(QWidget *parent,
				       const QStyleOptionViewItem &/* option */,
				       const QModelIndex &/*index*/) const
{
	QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
	editor->setFrame(false);
	editor->setRange(-1e20, 1e20);
	editor->setAlignment(Qt::AlignRight);
	return editor;
}

//Utils::Prefix m_prefix(Utils::nullPrefix);

void ValueDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	qreal value = index.model()->data(index, Qt::EditRole).toDouble();
	Utils::UnitScale unitF = Utils::unitPrefix(value, m_unit);
	value /= unitF.first;
	int decimals = m_decimals;
	if (value >= 100)
		decimals -= 3;
	else if (value >= 10)
		decimals -= 2;
	QDoubleSpinBox *spinbox = static_cast<QDoubleSpinBox*>(editor);
	spinbox->setSingleStep(exp10(-decimals));
	spinbox->setDecimals(decimals);
	spinbox->setSuffix(" " + unitF.second);
	spinbox->setValue(value);
}

void ValueDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
				    const QModelIndex &index) const
{
	QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
	spinBox->interpretText();
	qreal oldValue = index.data().toDouble();
	Utils::UnitScale unitF = Utils::unitPrefix(oldValue, m_unit);
	qreal value = spinBox->value() * unitF.first;
	model->setData(index, value, Qt::EditRole);
}

void ValueDelegate::updateEditorGeometry(QWidget *editor,
					 const QStyleOptionViewItem &option,
					 const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}
