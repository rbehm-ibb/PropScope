// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 9/27/2015 by behm
// ******************************************************

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class PropScope;
class ScopeScreen;
class ScopeSetup;
class TriggerSetup;
class Measurements;
class LedIcon;
class PointsModel;
class PointTableView;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
signals:
	void saveSettings(QSettings *conf);

public slots:
private slots:
	void quit();
	void start();
	void started();
	void stopped();
	void init();
	void about();
	void scopeError(QString msg);
	void scopeFound(QString msg);
	void scopeActive();
	void settingsChanged();
	void savePng();
	void saveData();
	void loadData();
	void noCursors(bool on);
private:
	QSettings *m_conf;
	PropScope *m_scope;
	ScopeScreen *m_screen;
	ScopeSetup *m_setScope;
	Measurements *m_measurements;
	TriggerSetup *m_trig;
	PointsModel *m_pointsModel;
	PointTableView *m_pointsView;
	QDockWidget *m_cursorDock;
	LedIcon *m_running;
	QLabel *m_devInfo;
	QLabel *m_logolabel;
	LedIcon *m_active;
};

#endif // MAINWINDOW_H
