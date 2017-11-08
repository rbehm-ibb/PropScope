// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 9/27/2015 by behm
// ******************************************************

#include "mainwindow.h"
#include "toolbarspacer.h"
#include "ledicon.h"
#include "propscope.h"
#include "scopesetup.h"
#include "scopescreen.h"
#include "triggersetup.h"
#include "pointsmodel.h"
#include "pointtableview.h"
#include "measurements.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_conf(new QSettings(qApp->applicationDirPath() + "/propscope.rc", QSettings::IniFormat))
{
	setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion());

	m_pointsModel = new PointsModel(this);
	m_scope = new PropScope(m_conf, this);
	m_screen = new ScopeScreen;
	connect(m_pointsModel, &PointsModel::cursorDeleted, m_screen, &ScopeScreen::redraw);
	setCentralWidget(m_screen);
	m_pointsModel->setGraphs(m_screen->channels());

	QToolBar *tbScope = addToolBar("TB-scope");
	QAction *actExit = tbScope->addAction(QIcon(":/exit"), tr("&Quit"), this, SLOT(quit()));
	actExit->setShortcut(QKeySequence::Quit);
	tbScope->addSeparator();
	QAction *act;
	act = tbScope->addAction(QIcon(":/fileopen"), "Load Data");
	connect(act, &QAction::triggered, this, &MainWindow::loadData);
	act->setShortcut(QKeySequence("Ctrl+L"));
	act = tbScope->addAction(QIcon(":/doc-save"), "Save Data");
	connect(act, &QAction::triggered, this, &MainWindow::saveData);
	act->setShortcut(QKeySequence("Ctrl+S"));
	act = tbScope->addAction(QIcon(":/pics/image.svgz"), "Save as PNG");
	connect(act, &QAction::triggered, this, &MainWindow::savePng);
	act->setShortcut(QKeySequence("Ctrl+P"));
	ToolBarSpacer::addSpacer(tbScope, 30);
	act = tbScope->addAction(QIcon(":/right"), "Start");
	connect(act, &QAction::triggered, this, &MainWindow::start);
	act->setShortcut(QKeySequence("Ctrl+G"));
	act = tbScope->addAction(QIcon(":/stop"), "Stop");
	connect(act, &QAction::triggered, m_scope, &PropScope::stop);
	act->setShortcut(QKeySequence("Ctrl+H"));
	tbScope->addWidget(m_running = new LedIcon(this));

	act = tbScope->addAction(QIcon(":/remove"), "ClrCur");
	connect(act, &QAction::triggered, m_pointsModel, &PointsModel::clear);

	ToolBarSpacer::addSpacer(tbScope);

	tbScope->addAction(QIcon(":/info"), "About", this, SLOT(about()));
	m_logolabel = new QLabel;
	tbScope->addWidget(m_logolabel);
	QLabel *logolabel = new QLabel;
	QPixmap logo2(":/ibb");
	logolabel->setPixmap(logo2);
	tbScope->addWidget(logolabel);
	m_running->setOnOff(false);

	QDockWidget *dock;
	dock = new QDockWidget(tr("Sampling/Scaling"), this);
	dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	dock->setWidget(m_setScope = new ScopeSetup(m_scope, m_screen));

	dock = new QDockWidget(tr("Trigger"), this);
	dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	m_trig = new TriggerSetup(m_screen, m_setScope);
	dock->setWidget(m_trig);

	dock = new QDockWidget(tr("Measurements"), this);
	dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	m_measurements = new Measurements;
	dock->setWidget(m_measurements);

	m_cursorDock = new QDockWidget(tr("Cursors"), this);
	m_cursorDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	m_cursorDock->setAllowedAreas(Qt::LeftDockWidgetArea /*| Qt::RightDockWidgetArea*/);
	addDockWidget(Qt::RightDockWidgetArea, m_cursorDock);
	m_pointsView = new PointTableView(m_pointsModel);
//	m_pointsView->setMinimumWidth(200);
	connect(m_pointsModel, &PointsModel::selected, m_pointsView, &PointTableView::setCurrentIndex);
	m_cursorDock->setWidget(m_pointsView);

	m_active = new LedIcon(this);
	m_active->setSize(20);
	m_devInfo = new QLabel;
	statusBar()->addPermanentWidget(m_devInfo);
	statusBar()->addPermanentWidget(m_active);

	connect(m_scope, &PropScope::scopeActive, this, &MainWindow::scopeActive);
	connect(m_scope, &PropScope::setTrigger, m_screen, &ScopeScreen::setTrigger);
	connect(m_scope, &PropScope::analogRxd, m_screen, &ScopeScreen::analogRxd);
	connect(m_scope, &PropScope::analogRxd, m_measurements, &Measurements::analogRxd);
	connect(m_scope, &PropScope::overRange, m_measurements, &Measurements::overRange);
	connect(m_scope, &PropScope::started, this, &MainWindow::started);
	connect(m_scope, &PropScope::stopped, this, &MainWindow::stopped);
	connect(m_scope, &PropScope::error, this, &MainWindow::scopeError);
	connect(m_scope, &PropScope::scopeFound, this, &MainWindow::scopeFound);
	connect(this, &MainWindow::saveSettings, m_scope, &PropScope::saveSettings);
	connect(this, &MainWindow::saveSettings, m_pointsModel, &PointsModel::saveData);

	QTimer::singleShot(1000, this, &MainWindow::init);

	m_screen->init(m_scope->config());
	m_trig->init(m_scope->config().trigger());

	connect(m_setScope, &ScopeSetup::changed, this, &MainWindow::settingsChanged);
	connect(m_setScope, &ScopeSetup::changed, m_trig, &TriggerSetup::changedScaling);
	connect(m_trig, &TriggerSetup::changed, this, &MainWindow::settingsChanged);
	connect(m_measurements, &Measurements::minMax, m_trig, &TriggerSetup::minMax);

	connect(m_scope, &PropScope::analogRxd, m_trig, &TriggerSetup::triggerred);
	QPixmap logo1(":/logo");
	m_logolabel->setPixmap(logo1);
	m_logolabel->setEnabled(false);
	connect(m_pointsModel, &PointsModel::empty, this, &MainWindow::noCursors);
}

MainWindow::~MainWindow()
{
}

void MainWindow::quit()
{
	m_conf->setValue("scope/running", m_scope->running());
	m_scope->stop();
	emit saveSettings(m_conf);
	m_conf->sync();
	qApp->closeAllWindows();
}

void MainWindow::start()
{
	ConfigData conf = m_setScope->getSettings();
	conf.setTrigger(m_trig->get());
	m_scope->startWithConfig(conf);
}

void MainWindow::started()
{
	m_running->setOnOff(true);
}

void MainWindow::stopped()
{
	m_running->setOnOff(false);
}

void MainWindow::init()
{
	m_devInfo->setText(m_scope->device());
}

void MainWindow::about()
{
	QIcon saveIcon = windowIcon();
	setWindowIcon(QIcon(":/ibb"));
	char year[] = "2016";
	QString text("<h2>%1</h2>"
		     "<h3>%2</h3>"
		     "Version %3"
		     "<br>&copy; %4, %5,<br>Mail: <a href=\"mailto:%6\">%6</a>"
		     "<p>Using  <img src=\":/stdicons/qt-logo-about.png\"> %8"
		     "<p>%7"
		     );
	QString info = m_scope->getScopeInfo();
	text = text
			.arg(qApp->applicationName())
			.arg(qApp->applicationDisplayName())
			.arg(qApp->applicationVersion())
			.arg(year)
			.arg(qApp->organizationName())
			.arg(qApp->organizationDomain())
			.arg(info)
			.arg(qVersion())
			;
	QMessageBox::about(this, qApp->applicationName(), text);
	setWindowIcon(saveIcon);
	m_scope->getVersion();
}

void MainWindow::scopeError(QString msg)
{
	qWarning() << msg;
	statusBar()->showMessage(msg, 5000);
}

void MainWindow::scopeFound(QString msg)
{
	m_logolabel->setEnabled(true);
	m_devInfo->setText(msg + " @ " + m_scope->device());
	if (m_conf->value("scope/running").toBool())
	{
		start();
	}
}

void MainWindow::scopeActive()
{
	m_active->setLed((m_active->color() == LedIconSource::Off) ? LedIconSource::Blue : LedIconSource::Off);
}

void MainWindow::settingsChanged()
{
	ConfigData conf = m_setScope->getSettings();
	conf.setTrigger(m_trig->get());
	m_scope->settingsChanged(conf);
}

void MainWindow::savePng()
{
//	qDebug() << Q_FUNC_INFO;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Screen Shot"),
							m_conf->value("file").toString(),
							tr("PNG (*.png);;All Files (*)"));
	if (! fileName.isEmpty())
	{
		if (! fileName.endsWith(".png"))
			fileName += ".png";
		QPixmap pm(size());
		render(&pm);
		QImage img = pm.toImage();
		img.setText("Created by", qApp->applicationName() + " " + qApp->applicationVersion());
		img.setText("Device", m_devInfo->text());
		img.setText("Date/Time", QDateTime::currentDateTime().toString(Qt::ISODate));
		/*bool rc =*/ img.save(fileName);
//		qDebug() << Q_FUNC_INFO << fileName << rc;
		statusBar()->showMessage("SavedTo " + fileName, 10*1000);
	}
}

void MainWindow::saveData()
{
//	qDebug() << Q_FUNC_INFO;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Data"),
							m_conf->value("file").toString(),
							tr("DAT (*.dat);;All Files (*)"));
	if (! fileName.isEmpty())
	{
		if (! fileName.endsWith(".dat"))
			fileName += ".dat";
		QSettings save(fileName, QSettings::IniFormat);
		save.setValue("@/time", QDateTime::currentDateTime().toString(Qt::ISODate));
		save.setValue("@/device", m_devInfo->text());
		save.setValue("@/program", qApp->applicationName());
		save.setValue("@/version", qApp->applicationVersion());
		m_screen->saveData(save);
		statusBar()->showMessage("SavedTo " + save.fileName(), 10*1000);
	}
}

void MainWindow::loadData()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Data"),
							m_conf->value("file").toString(),
							tr("DAT (*.dat);;All Files (*)"));
	if (! fileName.isEmpty())
	{
		if (! fileName.endsWith(".dat"))
			fileName += ".dat";
		QSettings save(fileName, QSettings::IniFormat);
		QString devInfo = save.value("@/device").toString();
		if (devInfo.isEmpty())
		{
			QMessageBox::critical(this, "Error", QString("%1 is Invalid").arg(fileName));
			return;
		}
		m_devInfo->setText(devInfo.section('@', 0, 0));
		m_scope->stop();
		ConfigData conf;
		const AnalogDataSet d = m_screen->loadData(conf, save);
//		conf.load(&save);
		m_setScope->init(conf);
		m_trig->init(conf.trigger());
		m_measurements->analogRxd(conf, d);
	}
}

void MainWindow::noCursors(bool on)
{
	m_cursorDock->setVisible(! on);
}
