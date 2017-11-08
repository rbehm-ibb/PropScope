// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/23/2015 by behm
// ******************************************************

#ifndef IPCCOMM_H
#define IPCCOMM_H

#include "project.h"

class IBSerialPort;

class IpcComm;

class IpcCommThread : public QThread
{
	Q_OBJECT
public:
	IpcCommThread(quint16 vid, quint16 pid, QObject *parent);
	~IpcCommThread();
	QString device() const { return m_realDevice; }

signals:
	void dataRxd(const QByteArray d);
	void txData(const QByteArray d);
	void error(QString msg);
protected:
	void run();
private:
	const quint16 m_vid;
	const quint16 m_pid;
	QString m_realDevice;
};

/// \brief The IpcComm class implements a serial communication protocol.
///
/// The IpcComm protocol has the following syntax:
/// - __STX__ \<transparent binary data\> __DLE__ __ETX__ \<chksum\>
/// - Response is __ACK__ or __NAK__
/// - \<transparent binary data\> uses DLE doubling
/// - \<chksum\> (8 bit) is the 8 bit sum of all sent bytes (incl STX and ETX, excl additional DLE bytes)
/// - We use printable ctrl chars 'S', 'D', 'E', 'A', 'N' for STX, DLE, ETX, ACK and NAK
/// - The first byte of the data is used a message identifier. Usually this is also a printable ASCII char.
///
/// The receiver is implemented as a state machine. The transmitter just sends all data of a block.
/// There is no risk of tx buffer overflow, since the max block size is 128 bytes and a new block is only sent
/// when the previous has been received by the other end (or it has been aborted).
///
/// The protocol is full duplex. At any time a block can be transmitted in either direction.
/// Responses are always sent after a transmitted block.
///
/// Timeouts are watched for inside a block and while waiting for a response.
/// If a block could not be successfully transmitted (ACK'd by other side) after 5 repeats it is discarded.
///

class IpcComm : public QObject
{
	Q_OBJECT
public:

	/// Create an object
	/// \param device The device (e.g. ttyUSB0) to use, with optional baudrate (ttyUSB1:57600, the default is 57600).
	/// \param parent for QObject
	explicit IpcComm(quint16 vid, quint16 pid, QObject *parent = 0);

	bool isValidPort() const;
	QString device() const;

	///maximal size of a message block
	static const int MaxMsgSize = 128;

	/// Queue a block for transmission.
	/// \param d the Block to send.
	/// \return true if successfull.
	bool txData(const QByteArray d);

signals:

	/// We have received a data block.
	void dataRxd(const QByteArray d);

	/// There was some error during communication (e.g. Timeout).
	/// \param msg Descriptive error message
	void error(QString msg);

	/// Something bad happened.
	/// The hardware port seems to have vanished. Could be removed USB.
	/// If something like this happens, try to recreate the IpcComm object. If this fails, complain to the user.
	void lostPortError();
public slots:
private slots:

	/// Internnaly connected to the port and called when data has been received.
	void rxReady();

	/// Called internally and tries to send a block or reply.
	void txPoll();

	/// Called internally from timer
	void timeoutRx();

	/// Called internally from timer
	void timeoutTx();

	/// Called internnaly from QSerialPort. Used to detect broken hardware connection.
	/// \param err The error code from QSerialPort.
	void portError(QSerialPort::SerialPortError err);
private:
	/// The rx state machine
	void (IpcComm::*m_rxState)(quint8 rxd);
	QString m_rxStateName;

	/// state of rx state machine. Idle state
	/// \param rxd next rxd byte
	void rxIdle(quint8 rxd);

	/// state of rx state machine. Data state when inside a block
	/// \param rxd next rxd byte
	void rxData(quint8 rxd);

	/// state of rx state machine. Had a DLE inside a block
	/// \param rxd next rxd byte
	void rxDle(quint8 rxd);

	/// state of rx state machine. Waiting for crc byte.
	/// \param rxd next rxd byte
	void rxCrc(quint8 rxd);

	/// Used internnaly to simulate a reply when rx is currently in use for rx of a block. Prevents timer from firing.
	void keepAlive();

	/// Simple helper to send a byte.
	/// \param txd Byte to send.
	void txByte(quint8 txd);

	/// Send a pending response.
	void sendAck();

	/// Used to detect time outs during rx (byte o byte)
	QTimer *m_rxTimer;

	/// Used to detect timeouts for response.
	QTimer *m_txTimer;

	/// Handles the real hardware port.
	IBSerialPort *m_port;

	/// To collect rxd data.
	QByteArray m_rxBuffer;

	/// To queue up block to be sent. Limited to max 10.
	QQueue<QByteArray> m_txQueue;

	/// Collects the checksum for rx
	quint8 m_rxSum;

	/// response to be sent or 0
	quint8 m_txResponse;

	/// Response received or 0
	quint8 m_rxdResponse;

	/// We are waitung for a response.
	bool m_waitResp;

	/// Counter for block repeats after NAK or timeout. after 5 repeats a block is discarded.
	int m_txRepeat;

	/// m_txBuffer contains valid data.
	bool m_hasTxBuffer;

	/// Contains the current block.
	QByteArray m_txBuffer;
};

#endif // IPCCOMM_H
