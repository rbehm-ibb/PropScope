// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 12/26/2015 by behm
// ******************************************************

#include "ibserialport.h"

IBSerialPort::IBSerialPort(QString device, QObject *parent)
	: QSerialPort(parent)
{
	init(device, 0);
}

IBSerialPort::IBSerialPort(QString device, int defaultBaud, QObject *parent)
	: QSerialPort(parent)
{
	init(device, defaultBaud);
}

IBSerialPort::IBSerialPort(quint16 vid, quint16 pid, int baud, QObject *parent)
	: QSerialPort(parent)
{
	QSerialPortInfo device;
	foreach (QSerialPortInfo spi, QSerialPortInfo::availablePorts())
	{
		if (spi.vendorIdentifier() == vid && spi.productIdentifier() == pid)
		{
			device = spi;
			setObjectName(device.description());
			setPort(device);
			if (! open(QIODevice::ReadWrite))
			{
				qWarning() << Q_FUNC_INFO << portName() << errorString();
				return;
			}
			setBaudRate(baud);
			setParity(QSerialPort::NoParity);
			setDataBits(QSerialPort::Data8);
			setFlowControl(QSerialPort::NoFlowControl);
			return;
		}
	}
	if (! device.isValid())
	{
		qWarning() << Q_FUNC_INFO << hex << vid << pid << hex << "not found";

	}
}

QString IBSerialPort::device() const
{
	return QString("%1:%2").arg(portName()).arg(baudRate());
}

void IBSerialPort::init(QString device, int defaultBaud)
{
	int baud = defaultBaud == 0 ? 9600 :defaultBaud;
	if (device.contains(':'))
	{
		bool ok;
		QString sbaud = device.section(':', 1);
		baud = sbaud.toUInt(&ok);
		if (! ok)
		{
			qWarning() << Q_FUNC_INFO << "Bad baudrate given" << device << "using default";
		}
		device = device.section(':', 0, 0);
	}
	setObjectName(device);
	setPortName(device);
	if (! open(QIODevice::ReadWrite))
	{
		qWarning() << Q_FUNC_INFO << portName() << errorString();
		return;
	}
	setBaudRate(baud);
	setParity(QSerialPort::NoParity);
	setDataBits(QSerialPort::Data8);
	setFlowControl(QSerialPort::NoFlowControl);
}
