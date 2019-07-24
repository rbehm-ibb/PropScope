// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 12/26/2015 by behm
// ******************************************************

#ifndef IBSERIALPORT_H
#define IBSERIALPORT_H

#include "project.h"

///
/// \brief The IBSerialPort class is a simple extension to QSerialPort.
///
/// It consists only a constructor.
/// The constructor creates a QSerialPort with standard settings and allows a baudrate to be given
/// with the device parameter. The optional baudrate is appended to the device name with a ":".
///
/// Sets the objectname() to the device name.
///
class IBSerialPort : public QSerialPort
{
public:
	/// \brief Create a IBSerialPort with standard settings.
	///
	/// Settings are 8bit/char, no parity, no flow control.
	/// \param device the device to use with optional baudrate appended like "ttyS0:9600".
	/// \param parent for QObject
	IBSerialPort(QString device, QObject *parent = nullptr);
	IBSerialPort(QString device, int defaultBaud, QObject *parent = nullptr);
	/// \brief Create IBSerialPort by selecting via vendorID and productID
	/// \param vid vendor ID
	/// \param pid product ID
	/// \param baud baudrate
	/// \param parent for QObject
	IBSerialPort(quint16 vid, quint16 pid, int baud, QObject *parent = nullptr);
	QString device() const;
private:
	void init(QString device, int defaultBaud);
};

#endif // IBSERIALPORT_H
