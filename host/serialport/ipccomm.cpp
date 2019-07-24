// ******************************************************
// * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
// * All Rights reserved
// * created 10/23/2015 by behm
// ******************************************************

#include "ipccomm.h"
#include "ibserialport.h"

/// \brief The ControlChar enum defines the control chars used in the protocol.
enum ControlChar {
	STX = 'S',	///< The start of text
	ETX = 'E',	///< The end of text
	DLE = 'D',	///< data link escape
	ACK = 'A',	///< acknolegde
	NAK = 'N',	///< negative acknowledge
	WACK = 'W'	///< internal keep alive
};

const int RxCharTime = 100;	///< The inter message byte to byte timeouts.
const int TxRespTime = 100;	///< The timeout for rx of a response.

IpcCommThread::IpcCommThread(quint16 vid, quint16 pid, QObject *parent)
	: QThread(parent)
	, m_vid(vid)
	, m_pid(pid)
{
//	qDebug() << Q_FUNC_INFO << device;
}

IpcCommThread::~IpcCommThread()
{
	quit();
	wait(1000);
}

void IpcCommThread::run()
{
	IpcComm *comm = new IpcComm(m_vid, m_pid);
	m_realDevice = comm->device();
	if (comm->isValidPort())
	{
		connect(this, &IpcCommThread::txData, comm, &IpcComm::txData, Qt::QueuedConnection);
		connect(comm, &IpcComm::dataRxd, this, &IpcCommThread::dataRxd, Qt::QueuedConnection);
		connect(comm, &IpcComm::error, this, &IpcCommThread::error, Qt::QueuedConnection);
		exec();
	}
	delete comm;
}


/////////////////////////////////////////

IpcComm::IpcComm(quint16 vid, quint16 pid, QObject *parent)
	: QObject(parent)
	, m_waitResp(false)
	, m_hasTxBuffer(false)
{
	m_rxState = &IpcComm::rxIdle;
	m_rxStateName = "rxIdle";

	m_port = new IBSerialPort(vid, pid, 230400, this);
	connect(m_port, &QSerialPort::readyRead, this, &IpcComm::rxReady);
	connect(m_port, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &IpcComm::portError);

	m_rxTimer = new QTimer(this);
	m_rxTimer->setSingleShot(true);
	connect(m_rxTimer, &QTimer::timeout, this, &IpcComm::timeoutRx);
	m_txTimer = new QTimer(this);
	m_txTimer->setSingleShot(true);
	connect(m_txTimer, &QTimer::timeout, this, &IpcComm::timeoutTx);
}

bool IpcComm::isValidPort() const
{
	return m_port && m_port->isOpen();
}

QString IpcComm::device() const
{
	if (isValidPort())
	{
		return m_port->device();
	}
	else
	{
		return QString::null;
	}
}

void IpcComm::portError(QSerialPort::SerialPortError err)
{
	switch(err)
	{
	case QSerialPort::ResourceError:
		emit lostPortError();
		break;
	default:
		qWarning() << Q_FUNC_INFO << err << m_port->errorString();
	}
}

bool IpcComm::txData(const QByteArray d)
{
	if (d.size() > MaxMsgSize)
	{
		qWarning() << Q_FUNC_INFO << "too large" << d.size();
		return false;
	}
//	qDebug() << Q_FUNC_INFO << d.toHex();
	if (m_txQueue.size() >= 10)
	{
		qWarning() << Q_FUNC_INFO << "tx OV";
		emit error("TxQ OV");
		return false;
	}
	m_txQueue.enqueue(d);
	if (! m_waitResp)
	{
		txPoll();	// trigger it
	}
	return true;
}

void IpcComm::rxReady()
{
	QByteArray rx = m_port->readAll();
//	if (verbose) qDebug() << Q_FUNC_INFO << rx.size() << rx.toHex();
	for (int i = 0; i < rx.size(); ++i)
	{
		quint8 rxc = static_cast<quint8>(rx.at(i));
		(this->*m_rxState)(rxc);
	}
}

// polling function
void IpcComm::txPoll()
{
	sendAck();
	if (m_waitResp)
	{
		if (m_rxdResponse)
		{
			switch (m_rxdResponse)
			{
			case ACK:
				m_hasTxBuffer = false;
				m_txTimer->stop();
				m_waitResp = false;
				break;
			case WACK:
				m_txTimer->start(TxRespTime);
				break;
			case NAK:
				m_txTimer->stop();
				m_waitResp = false;	// will repeat
				qDebug() << Q_FUNC_INFO << "NAK";
				if (++m_txRepeat > 5)
				{
					m_hasTxBuffer = false;	// drop it
					qDebug() << Q_FUNC_INFO << "abort on tx";
					emit error("Tx abort");
				}
				else
				{
					emit error("NAK rxd");
				}
				txPoll();	// try to resend
				break;
			}
			m_rxdResponse = 0;
		}
	}
	if (! m_waitResp && (m_hasTxBuffer || ! m_txQueue.isEmpty()))
	{
		if (! m_hasTxBuffer)
		{
			m_txBuffer = m_txQueue.dequeue();
			m_hasTxBuffer = true;
			m_txRepeat = 0;
		}
		m_rxdResponse = 0;
		txByte(STX);
		quint8 txSum = STX;
		for (int i = 0; i < m_txBuffer.size(); ++i)
		{
			const quint8 txc = static_cast<quint8>(m_txBuffer.at(i));
			txByte(txc);
			txSum += txc;
			if (txc == DLE)
			{
				txByte(DLE);
			}
		}
		txByte(DLE);
		txByte(ETX);
		txSum += ETX;
		txByte(txSum);
		m_txTimer->start(TxRespTime);
		m_waitResp = true;

		sendAck();
	}
}

void IpcComm::timeoutRx()
{
	qDebug() << Q_FUNC_INFO << m_rxStateName << m_rxBuffer.size();
	m_rxBuffer.clear();
	m_rxState = &IpcComm::rxIdle;
	m_rxStateName = "rxIdle";
	emit error("Rx Timeout");
}

void IpcComm::timeoutTx()
{
	m_waitResp = false;
	txPoll();
	emit error("Tx Timeout");
	qDebug() << Q_FUNC_INFO << m_rxStateName << m_rxdResponse << m_waitResp;
}

void IpcComm::rxIdle(quint8 rxd)
{
//	qDebug() << Q_FUNC_INFO << hex << rxd << dec;
	switch (rxd)
	{
	case ACK:
	case NAK:
		m_rxdResponse = rxd;
		txPoll();
		break;
	case STX:
		m_rxSum = STX;
		m_rxBuffer.clear();
		m_rxState = &IpcComm::rxData;
		m_rxStateName = "rxData";
		m_rxTimer->start(RxCharTime);
		keepAlive();
		break;
	default:	// ignore
		break;
	}
}

void IpcComm::rxData(quint8 rxd)
{
//	qDebug() << Q_FUNC_INFO << hex << rxd << dec;
	if (rxd == DLE)
	{
		m_rxState = &IpcComm::rxDle;
		m_rxStateName = "rxDle";
	}
	else
	{
		m_rxBuffer.append(static_cast<char>(rxd));
		m_rxSum += rxd;
	}
	m_rxTimer->start(RxCharTime);
	keepAlive();
}

void IpcComm::rxDle(quint8 rxd)
{
//	qDebug() << Q_FUNC_INFO << hex << rxd << dec;
	switch (rxd)
	{
	case DLE:
		m_rxSum += rxd;
		m_rxBuffer.append(static_cast<char>(rxd));
		m_rxState = &IpcComm::rxData;
		m_rxStateName = "rxData";
		break;
	case ETX:
		m_rxSum += rxd;
		m_rxState = &IpcComm::rxCrc;
		m_rxStateName = "rxCrc";
		break;
	}
	m_rxTimer->start(RxCharTime);
	keepAlive();
}

void IpcComm::rxCrc(quint8 rxd)
{
//	qDebug() << Q_FUNC_INFO << hex << rxd << dec;
	m_rxState = &IpcComm::rxIdle;
	m_rxStateName = "rxIdle";
	m_rxTimer->stop();
	if (rxd == m_rxSum)
	{
//		qDebug() << Q_FUNC_INFO << m_rxBuffer;
		emit dataRxd(m_rxBuffer);
		m_txResponse = ACK;
	}
	else
	{
//		qDebug() << Q_FUNC_INFO << "NAK";
		m_txResponse = NAK;
		emit error("Rx NAK response");
	}
	txPoll();	// try to send response
}

void IpcComm::keepAlive()
{
	if (m_waitResp && ((m_rxdResponse == 0) || (m_rxdResponse == WACK)))
	{
		m_rxdResponse = WACK;
		m_txTimer->start(TxRespTime);
	}
}

void IpcComm::txByte(quint8 txd)
{
//	if (verbose) qDebug() << Q_FUNC_INFO << hex << txd << dec;
	m_port->putChar(char(txd));
}

void IpcComm::sendAck()
{
//	qDebug() << Q_FUNC_INFO << hex << m_txResponse << dec;
	if (m_txResponse != 0)
	{
		txByte(m_txResponse);
		m_txResponse = 0;
	}
}
