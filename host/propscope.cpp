// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/6/2015 by behm
// ******************************************************

#include "propscope.h"
#include "ipccomm.h"
#include "utils.h"

const QDataStream::ByteOrder P8X32endian = QDataStream::LittleEndian;
const quint32 xtal = 100L * 1000L * 1000L;	// 100MHz

PropScope::PropScope(QSettings *conf, QObject *parent)
	: QObject(parent)
	, m_running(false)
	, m_confTimer(new QTimer(this))
{
	m_scopeComm = new IpcCommThread(0x13e2, 0x0080, this);
	connect(m_scopeComm, &IpcCommThread::dataRxd, this, &PropScope::rxdIpc);
	connect(m_scopeComm, &IpcCommThread::error, this, &PropScope::error);
	connect(this, &PropScope::sendIpc, m_scopeComm, &IpcCommThread::txData);
	m_confTimer->setSingleShot(true);
	m_confTimer->setInterval(100);
	connect(m_confTimer, &QTimer::timeout, this, &PropScope::sendConfig);
	m_config.load(conf);
	QTimer::singleShot(10, this, &PropScope::startComm);
}

PropScope::~PropScope()
{
	if (m_running)
	{
		stop();
	}
}

QString PropScope::device() const
{
	return m_scopeComm->device();
}

void PropScope::startComm()
{
	m_scopeComm->start();
	QThread::usleep(10000);
	if (! m_scopeComm->isRunning())
	{
		QMessageBox::critical(0, qApp->applicationName(), "No serial port\nor already running.");
		qApp->quit();
	}
	QTimer::singleShot(1000, this, &PropScope::getVersion);
	//	QTimer::singleShot(2000, this, &PropScope::start);
}

void PropScope::saveSettings(QSettings *conf)
{
//	qDebug() << Q_FUNC_INFO << endl << m_config;
	m_config.save(conf);
}

void PropScope::settingsChanged(const ConfigData &config)
{
	m_newConfig = config;
//	qDebug() << Q_FUNC_INFO;
	if (m_running)
	{
		m_confTimer->start();
	}
}

void PropScope::sendConfig()
{
	m_config = m_newConfig;
	sendConfData('G');
}

void PropScope::sendConfData(char cmd)
{
	QByteArray txData;
	QDataStream ds(&txData, QIODevice::WriteOnly);
	ds.setByteOrder(P8X32endian);
	quint32 frqa = (quint64(1) << 32) * m_config.time().sampleRate() / xtal;
	quint16 depth = m_config.time().depth();
//	quint8 pca = m_config.pca();
	ds << quint8(cmd)
	   << quint8(m_config.ch(0).res())
	   << quint8(m_config.ch(0).dc())
	   << quint8(m_config.ch(1).res())
	   << quint8(m_config.ch(1).dc())
	   << quint32(frqa)
	   << depth
	      ;
	quint16 level = qBound(0, int(m_config.trigger().level() / m_config.ch(m_config.trigger().channel()).scale()) + 512, 1023);
	quint16 tpp = m_config.trigger().post();
	uint tp = quint16(qreal(tpp) / 100. * qreal(depth));
	quint16 tpost = qBound(0U, tp, uint(depth));
	ds << quint8(m_config.trigger().channel())
	   << quint8(m_config.trigger().mode())
	   << quint8(m_config.trigger().edge())
	   << level
	   << quint16(tpost)
	      ;
//	qDebug() << Q_FUNC_INFO << m_config.trigger() << level;
	emit sendIpc(txData);
	emit setTrigger(m_config.trigger());
}

void PropScope::startWithConfig(const ConfigData &config)
{
//	qDebug() << Q_FUNC_INFO << config;
	m_config = config;
	start();
}

void PropScope::start()
{
//	stop();
	sendConfData('G');
}

void PropScope::stop()
{
//	qDebug() << Q_FUNC_INFO;
	emit sendIpc("S");
	m_running = false;
	emit stopped();
}

void PropScope::getVersion()
{
	emit sendIpc("V");
}

void PropScope::rxdIpc(const QByteArray rx)
{
	QDataStream ds(rx);
	ds.setByteOrder(P8X32endian);
	quint8 uiid;
	ds >> uiid;
	char id = char(uiid);
//	qDebug() << Q_FUNC_INFO << "rx" << id << rx.size()/* << rx.toHex()*/;
	emit scopeActive();
	switch (id)
	{
	default:
	case '?':
		qWarning() << Q_FUNC_INFO << "?" << id;
		break;
	case '#':
	{
		quint32 l;
		ds >> l;
		qWarning() << Q_FUNC_INFO << "size" << l;
	}
		break;
	case 'p':
	{
//		quint8 pca;
//		quint32 d;
//		ds >> pca >> d;
//		qDebug() << Q_FUNC_INFO << "pca" << bin << pca << dec << d;
	}
		break;
	case 'v':
	{
//		qDebug() << Q_FUNC_INFO << "rx" << id << rx.size() << rx.toHex();
		struct Version
		{
			quint32 calib[23];
			quint16 hwversion, testversion;
			quint16 cardhwversion, cardtestversion;
			quint16 dummy;
			char version[17];
			char cardversion[15];
		} v;
		memcpy(&v, rx.constData()+1, sizeof(v));
		const char *pgmVersion = rx.constData() + sizeof(v)-1;
//		qDebug() << pgmVersion << v.version << v.hwversion << v.testversion;
		m_scopeInfo = QString("<img src=\":/logo\"><br><b>Firmware</b><br>%1<br><b>Hardware</b><br>%2<br>Version %3:%4")
				.arg(pgmVersion)
				.arg(v.version)
				.arg(v.hwversion, 4, 16, QChar('0'))
				.arg(v.testversion, 4, 16, QChar('0'))
				;
		if (v.cardversion[0])
		{
			m_scopeInfo += QString("<br><b>Card</b><br>%1<br>Version %2:%3")
				.arg(v.cardversion)
				.arg(v.cardhwversion, 4, 16, QChar('0'))
				.arg(v.cardtestversion, 4, 16, QChar('0'))
				;
		}
		else
		{
			m_scopeInfo += "<br><b>No Ext-Card</b>";
		}
		emit scopeFound(QString("%1 FW:%2").arg(v.version).arg(pgmVersion));
	}
		break;
	case 'g':
//		qDebug() << Q_FUNC_INFO << "rx" << id << rx.size() << rx.toHex();
		m_running = true;
		emit started();
		break;
	case 's':
//		qDebug() << Q_FUNC_INFO << "rx" << id << rx.size() << rx.toHex();
		m_running = false;
		emit stopped();
		break;
	case 'd':
		if (m_running)
		{
			bool h0 = false, l0 = false, h1 = false, l1 = false;
			quint16 depth;
			quint8 pca;
			quint16 tpost;
			ds >> pca >> tpost;
			ds >> depth;
			qreal dt = 1. / m_config.time().sampleRate();
			Utils::UnitScale scale = Utils::unitPrefix(dt*depth, "s");
			dt /= scale.first;
			int toff = depth - tpost;
			const int offset = 510;
			AnalogData time(depth);
			AnalogData ch0(depth);
			AnalogData ch1(depth);
//			quint32 lastt = 0;
			for (int i = 0; i < int(depth); ++i)
			{
				quint32 v;
				if (ds.atEnd())
				{
					qWarning() << Q_FUNC_INFO << "not enough data, depth=" << depth << "at" << i;
					return;
				}
				ds >> v;
				qint32 d0 = (v & 0x3ff);
				qint32 d1 = ((v >> 10) & 0x3ff);
				if (d0 >= 1020) h0 = true;
				if (d1 >= 1020) h1 = true;
				if (d0 <= 20) l0 = true;
				if (d1 <= 20) l1 = true;
//				quint32 t = (v >> 20);
//				quint32 ddt = (t - lastt) & 0xfff;
//				lastt = t;
//				qDebug() << delta << ddt << d0 << d1 << dec;
				time[i] = dt * (i - toff);
				ch0[i] = (d0 - offset) * m_config.ch(0).scale();
				ch1[i] = (d1 - offset) * m_config.ch(1).scale();
			}
			if (! ds.atEnd())
			{
				qWarning() << Q_FUNC_INFO << "too much data << depth=" << depth << "size=" << rx.size();
				return;
			}
			m_config.time().setDepth(depth);
			m_config.time().setUnitScale(scale);
			m_config.time().setRange(time.first(), time.last());
			emit analogRxd(m_config, AnalogDataSet { time, ch0, ch1} );
			emit overRange(l0, h0, l1, h1);
		}
		break;
	}
}


