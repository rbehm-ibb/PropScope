// ******************************************************
// * (C) 2007 by R. Behm (rbehm@eae.de), EAE electronics
// * All Rights reserved
// ******************************************************

#include "project.h"
#include "serial.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

//#define WITH_LOCK 0

SerialPort::SerialPort(const QString _devDescr, QObject * parent, const QString name)
	: QObject(parent)
	, devDescr(_devDescr)
	, baud(9600)
	, mFd(-1)
	, rxNoti(0)
	, socket(-1)
{
	mDevice = devDescr;
	setObjectName(name);
	if (devDescr.contains(':'))
	{
		baud = devDescr.section(':', 1, 1).toUInt();
		mDevice = devDescr.section(':', 0, 0);
	}
//	qDebug() << Q_FUNC_INFO << _devDescr << baud << objectName();
	reOpen();
}

SerialPort::SerialPort(int _socket, QObject * parent, const QString name)
	: QObject(parent)
	, devDescr(QString("socket#%1").arg(socket))
	, baud(0)
	, mFd(-1)
	, rxNoti(0)
	, socket(_socket)
{
	setObjectName(name);
	mDevice = devDescr;
	setObjectName(mDevice);
	reOpen();
}

bool SerialPort::reOpen()
{
	close();
	if (socket < 0)
	{
		delete rxNoti;
		rxNoti = 0;
		if (mDevice.isEmpty())
			return false;
		mFd = open(mDevice.toAscii(), O_RDWR | O_NOCTTY);
		if (mFd < 0)
		{
			int errnum = errno;
			qCritical() << Q_FUNC_INFO << objectName() << mDevice << "e:" << errnum << strerror(errnum);
			return false;
		}
		tcgetattr(mFd, &oldtio);	// save old settings.
		bzero(&newtio, sizeof(newtio));

		newtio.c_cflag = CS8 | CLOCAL | CREAD;
		switch(baud)
		{
		case 75:
			newtio.c_cflag |= B75;
			break;
		case 150:
			newtio.c_cflag |= B150;
			break;
		case 300:
			newtio.c_cflag |= B300;
			break;
		case 600:
			newtio.c_cflag |= B600;
			break;
		case 1200:
			newtio.c_cflag |= B1200;
			break;
		case 2400:
			newtio.c_cflag |= B2400;
			break;
		case 4800:
			newtio.c_cflag |= B4800;
			break;
		case 9600:
			newtio.c_cflag |= B9600;
			break;
		case 19200:
			newtio.c_cflag |= B19200;
			break;
		case 38400:
			newtio.c_cflag |= B38400;
			break;
		case 57600:
			newtio.c_cflag |= B57600;
			break;
		case 115200:
			newtio.c_cflag |= B115200;
			break;
		default:
			newtio.c_cflag |= B9600;
			break;
		}
		newtio.c_iflag = IGNPAR;
		newtio.c_oflag = 0;
		newtio.c_lflag = 0;
		newtio.c_cc[VINTR]	= 0;
		newtio.c_cc[VQUIT]	= 0;
		newtio.c_cc[VERASE]	= 0;
		newtio.c_cc[VKILL]	= 0;
		newtio.c_cc[VEOF]	= 0;
		newtio.c_cc[VTIME]	= 0;
		newtio.c_cc[VMIN]	= 0;
		newtio.c_cc[VSWTC]	= 0;
		newtio.c_cc[VSTART]	= 0;
		newtio.c_cc[VSTOP]	= 0;
		newtio.c_cc[VSUSP]	= 0;
		newtio.c_cc[VEOL]	= 0;
		newtio.c_cc[VREPRINT]	= 0;
		newtio.c_cc[VDISCARD]	= 0;
		newtio.c_cc[VLNEXT]	= 0;
		newtio.c_cc[VKILL]	= 0;
		newtio.c_cc[VEOL]	= 0;

		tcflush(mFd, TCIFLUSH);	// FlushInput
		tcsetattr(mFd, TCSANOW, &newtio);
	}
	else	// socket
	{
		mFd = socket;
	}
//	cout << qDebug() << "@" << name() << " at " << mDevice << " done" << endl;
	rxNoti = new QSocketNotifier(mFd, QSocketNotifier::Read, this);
	return true;
}

SerialPort::~SerialPort()
{
	close();
}

void SerialPort::close()
{
// 	cout << qDebug()<< "@"  << name() << " at " << mDevice << " done" << endl;
	if (socket >= 0)
		return;
	if (mFd >= 0)
	{
		tcsetattr(mFd, TCSANOW, &oldtio);
		::close(mFd);
	}
	mFd = -1;
}

int SerialPort::read(char * data, size_t size)
{
	if (valid())
		return ::read(mFd, data, size);
	else
		return -1;
}

QByteArray SerialPort::read(size_t maxsize)
{
	if (! valid())
		return QByteArray();
	QByteArray res;
	res.resize(maxsize);
	int rc = ::read(mFd, res.data(), maxsize);
	if (rc <= 0)
		return QByteArray();
	res.truncate(rc);
	return res;

}

int SerialPort::write(const char * data, size_t size)
{
	if (valid())
		return ::write(mFd, data, size);
	else
		return -1;
}

int SerialPort::write(const QByteArray data)
{
	if (valid())
		return ::write(mFd, data.constData(), data.size());
	else
		return -1;
}

/////////////////////////////////

SerialPortLine::SerialPortLine(QString devDescr, QObject * parent, const QString name)
	: SerialPort(devDescr, parent, name)
{
// 	qDebug() << qDebug() << " dev=" << devDescr << endl;
	if (rxNoti) connect(rxNoti, SIGNAL(activated(int)), this, SLOT(readData(int)));
}

SerialPortLine::SerialPortLine(int _socket, QObject * parent, const QString name)
	: SerialPort(_socket, parent, name)
{
// 	qDebug() << qDebug() << " socket=" << socket << endl;
	if (rxNoti) connect(rxNoti, SIGNAL(activated(int)), this, SLOT(readData(int)));
}

SerialPortLine::~SerialPortLine()
{
}

QByteArray SerialPortLine::readLine()
{
	int i = rxd.indexOf('\n');
	if (i >= 0)
	{
		QByteArray line = rxd.left(i);
		rxd = rxd.mid(i+1);
		line = line.replace('\r', "");
		return line;
	}
	return QByteArray();
}

void SerialPortLine::readData(int)
{
	QByteArray rx = read(1000);
	// cout << qDebug() << " rc=" << rc << endl;
	if (! rx.isEmpty())
	{
		rxd += rx;
		if (rxd.contains('\n'))
			emit hasLine();
		else if (rxd.length() > 5000)
		{
			qWarning() << Q_FUNC_INFO << " length OV";
			rxd = QByteArray();
		}

	}
// 	cout << qDebug() << " rxd# " << rxd.length() << endl;
}
