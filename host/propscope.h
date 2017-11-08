// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/6/2015 by behm
// ******************************************************

#ifndef PROPSCOPE_H
#define PROPSCOPE_H

#include "utils.h"
#include "configdata.h"
#include "analogdata.h"
#include "triggerdata.h"

class IpcCommThread;

class PropScope : public QObject
{
	Q_OBJECT
public:
	PropScope(QSettings *conf, QObject *parent);
	~PropScope();
	const ConfigData &config() const { return m_config; }

	QString getScopeInfo() const { return m_scopeInfo; }
	QString device() const;

	bool running() const { return m_running; }

signals:
	void started();
	void stopped();
	void analogRxd(const ConfigData &config, const AnalogDataSet &data);
	void setTrigger(const TriggerData &td);
	void sendIpc(const QByteArray d);
	void error(QString msg);
	void scopeFound(QString msg);
	void scopeActive();
	void overRange(bool l0, bool h0, bool l1, bool h1);

public slots:
	void saveSettings(QSettings *conf);
	void startWithConfig(const ConfigData &config);
	void stop();
	void getVersion();
	void settingsChanged(const ConfigData &config);
private slots:
	void start();
	void rxdIpc(const QByteArray rx);
	void startComm();
	void sendConfig();
private:
	IpcCommThread *m_scopeComm;
	ConfigData m_config;	/// current config sent to hw
	ConfigData m_newConfig;	/// current config sent to hw
	bool m_running;
	QString m_scopeInfo;
	QTimer *m_confTimer;
	void sendConfData(char cmd);
};

#endif // PROPSCOPE_H
