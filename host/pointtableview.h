// ******************************************************
// * copyright (C) 2016 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 5/27/2016 by behm
// ******************************************************

#ifndef POINTTABLEVIEW_H
#define POINTTABLEVIEW_H

class PointsModel;

class PointTableView : public QTableView
{
	Q_OBJECT
public:
	PointTableView(PointsModel *model, QWidget *parent = 0);
public slots:
	void insert();
	void append();
	void remove();
	void clear();
protected:
private:
	PointsModel *m_model;
	QAction *m_actClear;
	QAction *m_actRemove;

	// QAbstractItemView interface
public:
	void setModel(PointsModel *model);

protected slots:
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	void currentChanged(const QModelIndex &current, const QModelIndex &previous);
};

#endif // POINTTABLEVIEW_H
