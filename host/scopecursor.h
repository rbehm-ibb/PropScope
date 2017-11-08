// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/21/2015 by behm
// ******************************************************

#ifndef SCOPECURSOR_H
#define SCOPECURSOR_H

class ScopeCursor : public QCPItemStraightLine
{
	Q_OBJECT
public:
	enum Side { Left, Right, Top, Bottom };
	ScopeCursor(QCPGraph *graph, ScopeCursor::Side side, QPen pen);
	~ScopeCursor();
	virtual void addText(const QString baseText);
	void setText(const QString text);
	void setVisible(bool on);
	virtual void mouseMoveEvent(QMouseEvent *event) = 0;
	int id() const { return m_id; }
	void setId(int id) { m_id = id; }
signals:
	void deleted(ScopeCursor *me);
	void cursorSelected(ScopeCursor *me);
public slots:

protected:
	const Side m_side;
	int m_id;
	QCPItemText *m_text;
	QString m_baseText;

protected:
	virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged);
};

class HCursor : public ScopeCursor
{
	Q_OBJECT
public:
	HCursor(QCPGraph *graph, int gid, ScopeCursor::Side side, QPen pen);
	void setY(qreal y);
	qreal posY() const { return point1->value(); }
	void mouseMoveEvent(QMouseEvent *event);
	int graphId() const { return m_graphId; }

signals:
	void posYchanged(HCursor *cursor, qreal y);
	void posYchangedId(int id, qreal y);
private slots:
protected:
	const int m_graphId;
};

class VCursor : public ScopeCursor
{
	Q_OBJECT
public:
	VCursor(QCPGraph *graph, QPen pen);
	void setX(qreal x);
	qreal posX() const { return point1->key(); }
	void mouseMoveEvent(QMouseEvent *event);
signals:
	void posXchanged(VCursor *vc, qreal x);
public slots:
	void timeUnitChanged(qreal unitScale);
protected:
	qreal m_unitScale;
};

#endif // SCOPECURSOR_H
