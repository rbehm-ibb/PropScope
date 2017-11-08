// ******************************************************
// * (C) 2007 by R. Behm (rbehm@eae.de), EAE electronics
// * All Rights reserved
// ******************************************************

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include "project.h"
#include <termios.h>

/**
  * @author R.Behm, EAE electronics
  * \brief Serial port interface
  */

class SerialPort : public QObject
{
//	Q_OBJECT
public:
	/// creates SerialPort and opens it
	/// devDescr may contain device:baud, if baud invalid or missing 9600 is used
	SerialPort(const QString devDescr, QObject * parent = 0, const QString name = QString());
	SerialPort(int socket, QObject * parent = 0, const QString name = QString());
	virtual ~SerialPort();
	bool valid() const { return mFd >= 0; }
	int fd() const { return mFd; }
	const QString device() const { return mDevice; }
	QSocketNotifier * rxNotifier() const { return rxNoti; }
	QByteArray read(size_t maxsize);
	int read(char *data, size_t size);
	int write(const QByteArray);
	int write(const char *data, size_t size);
	bool reOpen();
	void close();
protected:
	const QString devDescr;
	QString mDevice;
	int baud;
	int mFd;		///< the input port
	QSocketNotifier * rxNoti;
	struct termios oldtio, newtio;
	int socket;
};

class SerialPortLine : public SerialPort
{
	Q_OBJECT
public:

	SerialPortLine(QString devDescr, QObject * parent = 0, const QString name = QString());
	SerialPortLine(int socket, QObject * parent = 0, const QString name = QString());
	virtual ~SerialPortLine();
	bool reOpen() { rxd.truncate(0); return SerialPort::reOpen(); }
	QByteArray readLine();
	bool canReadLine() const { return rxd.contains('\n'); }
protected:
	QByteArray rxd;
private slots:
	void readData(int);
signals:
	void hasLine();
};

#endif
