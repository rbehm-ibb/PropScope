// ******************************************************
// * copyright (C) 2016 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 5/27/2016 by behm
// ******************************************************

#include "pointtableview.h"
#include "pointsmodel.h"
#include "valuedelegate.h"

PointTableView::PointTableView(PointsModel *model, QWidget *parent)
	: QTableView(parent)
	, m_model(model)
{
	setModel(model);
	model->setAllowEditY(true);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);

	m_actClear = new QAction(QIcon(":/pics/table_delete.png"), tr("Clear Table"), this);
	m_actRemove = new QAction(QIcon(":/pics/table_row_delete.png"), tr("Remove Row"), this);

	addAction(m_actRemove);
	addAction(m_actClear);
	connect(m_actRemove, &QAction::triggered, this, &PointTableView::remove);
	connect(m_actClear, &QAction::triggered, this, &PointTableView::clear);
	setContextMenuPolicy(Qt::ActionsContextMenu);

	QHeaderView *header = horizontalHeader();
	for (int i = 0; i < header->count(); ++i)
	{
		header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
	}
	header->setStretchLastSection(true);
	for (int i = 1; i < model->columnCount(); ++i)
	{
		setItemDelegateForColumn(i, new ValueDelegate(model->units()[i], 3, this));
	}
}

void PointTableView::insert()
{
	if (currentIndex().isValid())
	{
		m_model->insertRow(currentIndex().row());
	}
}

void PointTableView::append()
{
	m_model->insertRow(m_model->rowCount());

}

void PointTableView::remove()
{
	if (currentIndex().isValid())
	{
		m_model->removeRow(currentIndex().row());
	}
}

void PointTableView::clear()
{
	m_model->clear();
}

void PointTableView::setModel(PointsModel *model)
{
	QTableView::setModel(model);
}

void PointTableView::selectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
//	qDebug() << Q_FUNC_INFO << selected;
}

void PointTableView::currentChanged(const QModelIndex &/*current*/, const QModelIndex &/*previous*/)
{
//	qDebug() << Q_FUNC_INFO << current;
}
