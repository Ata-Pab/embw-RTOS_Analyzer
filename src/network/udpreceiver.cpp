#include "udpreceiver.h"

#include <QUdpSocket>
#include <QDateTime>

UdpReceiver::UdpReceiver(QObject *parent) : QObject(parent) {}

UdpReceiver::~UdpReceiver()
{
    stop();
}

bool UdpReceiver::start(const QHostAddress &address, quint16 port)
{
    if (!m_socket)
    {
        m_socket = new QUdpSocket(this);
        connect(m_socket, &QUdpSocket::readyRead, this, &UdpReceiver::onReadyRead);
    }

    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->close();

    if (!m_socket->bind(address, port,
                        QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        emit errorOccurred(m_socket->errorString());
        return false;
    }
    return true;
}

void UdpReceiver::stop()
{
    if (m_socket && m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->close();
}

bool UdpReceiver::isListening() const
{
    return m_socket && m_socket->state() == QAbstractSocket::BoundState;
}

void UdpReceiver::onReadyRead()
{
    while (m_socket && m_socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(datagram.data(), datagram.size());

        const QString msg = QString::fromUtf8(datagram).trimmed();
        const QStringList parts = msg.split(u';');

        RtosEvent event;
        event.timestamp = QDateTime::currentDateTime();

        if (parts.size() == 3)
        {
            bool idOk = false, codeOk = false;
            const int taskId = parts[1].toInt(&idOk);
            const int code = parts[2].toInt(&codeOk);

            if (idOk && codeOk)
            {
                event.taskName = parts[0];
                event.taskId = taskId;
                event.eventType = rtosEventFromCode(code);
                const QString ts = event.timestamp.toString(QStringLiteral("HH:mm:ss.zzz"));
                event.rawLine = QStringLiteral("%1: [Remote Log] (ID:%2) %3 -> %4")
                                    .arg(ts)
                                    .arg(taskId)
                                    .arg(event.taskName, -25)
                                    .arg(rtosEventName(event.eventType));
            }
            else
            {
                // Malformed int fields — treat as raw
                event.taskName = QStringLiteral("(raw)");
                event.taskId = -1;
                event.eventType = RtosEventType::Unknown;
                event.rawLine = QStringLiteral("%1: [Remote Log] Raw: %2")
                                    .arg(event.timestamp.toString(QStringLiteral("HH:mm:ss.zzz")))
                                    .arg(msg);
            }
        }
        else
        {
            event.taskName = QStringLiteral("(raw)");
            event.taskId = -1;
            event.eventType = RtosEventType::Unknown;
            event.rawLine = QStringLiteral("%1: [Remote Log] Raw: %2")
                                .arg(event.timestamp.toString(QStringLiteral("HH:mm:ss.zzz")))
                                .arg(msg);
        }

        emit eventReceived(event);
    }
}
