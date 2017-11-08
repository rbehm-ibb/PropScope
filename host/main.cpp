// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 9/27/2015 by behm
// ******************************************************

#include "mainwindow.h"

bool verbose = false;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("propscope");
	app.setApplicationVersion("V1.05");
	app.setOrganizationDomain("rbehm@hushmail.com");
	app.setOrganizationName("R.Behm");
	app.setApplicationDisplayName("UI for the PropScope");
	app.setWindowIcon(QIcon(":/logo"));

//	QString device;
	{
		QCommandLineParser parser;
		parser.setApplicationDescription(app.applicationDisplayName());
		parser.addHelpOption();
		parser.addVersionOption();

//		QCommandLineOption devOption(QStringList() << "d" << "device", "Port for PropScope", "prop.scope", "prop.scope");
//		parser.addOption(devOption);
		QCommandLineOption verboseOption(QStringList() << "t" << "verbose", "be verbose");
		parser.addOption(verboseOption);
		parser.process(app);
		verbose = parser.isSet(verboseOption);
//		device = parser.value(devOption);

		QFile file(":/styles.css");
		if(file.open(QFile::ReadOnly))
		{
			qApp->setStyleSheet(QLatin1String(file.readAll()));
		}
		else
		{
			qWarning() << Q_FUNC_INFO << file.fileName() << file.errorString();
		}
	}
//	qDebug() << Q_FUNC_INFO << device;
	MainWindow mw;
	mw.show();
	return app.exec();
}
