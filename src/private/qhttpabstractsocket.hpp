/** @file qhttpabstractsocket.hpp
 *
 * @copyright (C) 2018
 * @date 2018.06.18
 * @version 1.1.0
 * @author  S.Mehran M.Ziabary <ziabary@targoman.com>
 *          amir zamani <azadkuh@live.com>
 *
 */

#ifndef QHTTP_ABSTRACTSOCKET_HPP
#define QHTTP_ABSTRACTSOCKET_HPP

#include "qhttpfwd.hpp"

#include <QTcpSocket>
#include <QLocalSocket>
#include <QHostAddress>
#include <QUrl>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace details {
///////////////////////////////////////////////////////////////////////////////

/** an adapter for different socket types.
 * the main purpose of QHttp was to create a small HTTP server with ability to
 * support UNIX sockets (QLocalSocket)
 */
class QHttpAbstractSocket
{
public:
    QHttpAbstractSocket(QIODevice* socket) :
        isocket(socket)
    {}

    QHttpAbstractSocket& operator = (const QHttpAbstractSocket& o){
        isocket = o.isocket;
        return *this;
    }

    virtual ~QHttpAbstractSocket();

    void close() {
        if(Q_LIKELY(isocket))
            isocket->close();
    }

    void release() {
        close();
        if(Q_LIKELY(isocket))
            isocket->deleteLater();

        isocket   = nullptr;
    }

    qint64 readRaw(char* buffer, int maxlen) {
        return Q_LIKELY(isocket) ? isocket->read(buffer, maxlen) : 0;
    }

    void writeRaw(const QByteArray& data) {
        if (Q_LIKELY(isocket))
            isocket->write(data);
    }

    void disconnectAllQtConnections() {
        if ( isocket )
            QObject::disconnect(isocket, nullptr, nullptr, nullptr);
    }

    inline qint64 bytesAvailable() {
        return Q_LIKELY(isocket) ? isocket->bytesAvailable() : 0;
    }

    inline qint64 bytesToWrite() {
        return Q_LIKELY(isocket) ? isocket->bytesToWrite(): 0;
    }

public:
    /** used on server where incomming connection request is arrived*/
    virtual   void init(QObject* parent,
                        qintptr sockDesc,
                        const std::function< void() >& onReadyRead,
                        const std::function< void() >& onWriteReady,
                        const std::function< void() >& onDisconnected) = 0;

    /** used on clients to create new socket*/
    virtual   void init(const std::function< void() >& onConnected,
                        const std::function< void() >& onReadyRead,
                        const std::function< void() >& onWriteReady,
                        const std::function< void() >& onDisconnected) = 0;

    virtual void flush() = 0;
    virtual bool isOpen() const = 0;
    virtual void connectTo(const QString& url) = 0;
    virtual void connectTo(const QString& host, quint16 port) = 0;
    virtual QString remoteAddress() = 0;
    virtual quint16 remotePort() = 0;

public:
    QIODevice* isocket;
}; // class QHttpAbstractSocket


class QHttpTcpSocket : public QHttpAbstractSocket
{
public:
    QHttpTcpSocket(QObject *parent = Q_NULLPTR) :
        QHttpAbstractSocket(new QTcpSocket(parent))
    {}

    virtual ~QHttpTcpSocket() override;

    void init(QObject* parent,
              qintptr sockDesc,
              const std::function< void() >& onReadyRead,
              const std::function< void() >& onWriteReady,
              const std::function< void() >& onDisconnected) override{
        static_cast<QTcpSocket*>(isocket)->setSocketDescriptor(sockDesc);

        QObject::connect(
                    static_cast<QTcpSocket*>(isocket),  &QTcpSocket::readyRead,
                    onReadyRead
                    );
        QObject::connect(
                    static_cast<QTcpSocket*>(isocket),  &QTcpSocket::bytesWritten,
                    onWriteReady
                    );
        QObject::connect(
                    static_cast<QTcpSocket*>(isocket),      &QTcpSocket::disconnected,
                    parent, onDisconnected,
                    Qt::QueuedConnection
                    );
    }

    void init(const std::function< void() >& onConnected,
              const std::function< void() >& onReadyRead,
              const std::function< void() >& onWriteReady,
              const std::function< void() >& onDisconnected) override {
        QObject::connect(
                    static_cast<QTcpSocket*>(isocket),  &QTcpSocket::connected,
                    onConnected
                    );
        QObject::connect(
                    static_cast<QTcpSocket*>(isocket),  &QTcpSocket::readyRead,
                    onReadyRead
                    );
        QObject::connect(
                    static_cast<QTcpSocket*>(isocket),  &QTcpSocket::bytesWritten,
                    onWriteReady
                    );
        QObject::connect(
                    static_cast<QTcpSocket*>(isocket),  &QTcpSocket::disconnected,
                    onDisconnected
                    );
    }

    inline void flush() override {
        if(Q_LIKELY(isocket))
            static_cast<QTcpSocket*>(isocket)->flush();
    }

    inline bool isOpen() const override {
        return Q_LIKELY(isocket) ?
                    (static_cast<QTcpSocket*>(isocket)->isOpen() && static_cast<QTcpSocket*>(isocket)->state() == QTcpSocket::ConnectedState) :
                    false;

    }

    inline void connectTo(const QString&) override {
        throw exQHttpNotImplemented("conncet to URL can not be implemented for TCP sockets");
    }

    inline void connectTo(const QString& host, quint16 port) override {
        static_cast<QTcpSocket*>(isocket)->connectToHost(host, port);
    }

    inline QString remoteAddress() override { return static_cast<QTcpSocket*>(isocket)->peerAddress().toString(); }
    inline quint16 remotePort() override { return static_cast<QTcpSocket*>(isocket)->peerPort(); }
}; // class QHttpTcpSocket

class QHttpLocalSocket : public QHttpAbstractSocket
{
public:
    QHttpLocalSocket(QObject *parent = Q_NULLPTR) :
        QHttpAbstractSocket(new QLocalSocket(parent))
    {}
    virtual ~QHttpLocalSocket() override;

    void init(QObject* parent,
              qintptr sockDesc,
              const std::function< void() >& onReadyRead,
              const std::function< void() >& onWriteReady,
              const std::function< void() >& onDisconnected) override{
        static_cast<QLocalSocket*>(isocket)->setSocketDescriptor(sockDesc);

        QObject::connect(
                    static_cast<QLocalSocket*>(isocket), &QLocalSocket::readyRead,
                    onReadyRead
                    );
        QObject::connect(
                    static_cast<QLocalSocket*>(isocket), &QLocalSocket::bytesWritten,
                    onWriteReady
                    );
        QObject::connect(
                    static_cast<QLocalSocket*>(isocket),  &QLocalSocket::disconnected,
                    parent, onDisconnected,
                    Qt::QueuedConnection
                    );
    }

    void init(const std::function< void() >& onConnected,
              const std::function< void() >& onReadyRead,
              const std::function< void() >& onWriteReady,
              const std::function< void() >& onDisconnected) override {
        QObject::connect(
                    static_cast<QLocalSocket*>(isocket),  &QLocalSocket::connected,
                    onConnected
                    );
        QObject::connect(
                    static_cast<QLocalSocket*>(isocket),  &QLocalSocket::readyRead,
                    onReadyRead
                    );
        QObject::connect(
                    static_cast<QLocalSocket*>(isocket),  &QLocalSocket::bytesWritten,
                    onWriteReady
                    );
        QObject::connect(
                    static_cast<QLocalSocket*>(isocket),  &QLocalSocket::disconnected,
                    onDisconnected
                    );
    }

    inline void flush() override {
        if(Q_LIKELY(isocket))
            static_cast<QLocalSocket*>(isocket)->flush();
    }

    inline bool isOpen() const override {
        return Q_LIKELY(isocket) ?
                    (static_cast<QLocalSocket*>(isocket)->isOpen() && static_cast<QLocalSocket*>(isocket)->state() == QLocalSocket::ConnectedState) :
                    false;
    }

    inline void connectTo(const QString& name) override {
        static_cast<QLocalSocket*>(isocket)->connectToServer(name);
    }

    inline void connectTo(const QString&, quint16) override {
        throw exQHttpNotImplemented("conncet to Host:Port can not be implemented for Local sockets");
    }

    inline QString remoteAddress() override { return static_cast<QLocalSocket*>(isocket)->fullServerName(); }
    inline quint16 remotePort() override { return 0; /* not used in local sockets */; }

}; // class QHttpLocalSocket

///////////////////////////////////////////////////////////////////////////////
} // namespace details
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTP_SOCKET_HPP
