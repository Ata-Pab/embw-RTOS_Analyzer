#pragma once

#include <QObject>
#include <QHostAddress>
#include "rtos/rtosevent.h"

class QUdpSocket;

/**
 * @class UdpReceiver
 * @brief Listens on a UDP socket and emits parsed RtosEvent signals.
 *
 * Wire format expected: "taskName;taskId;eventCode"
 * (matches the Python log_listener.py format)
 */
class UdpReceiver : public QObject
{
    Q_OBJECT

public:
    explicit UdpReceiver(QObject *parent = nullptr);
    ~UdpReceiver() override;

    /// Bind to @a address : @a port and start receiving. Returns false on error.
    bool start(const QHostAddress &address, quint16 port);
    void stop();
    bool isListening() const;

signals:
    void eventReceived(const RtosEvent &event);
    void errorOccurred(const QString &message);

private slots:
    void onReadyRead();

private:
    QUdpSocket *m_socket{nullptr};
};
