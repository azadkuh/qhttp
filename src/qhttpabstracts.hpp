#ifndef QHTTPABSTRACTS_HPP
#define QHTTPABSTRACTS_HPP

///////////////////////////////////////////////////////////////////////////////
#include "qhttpserverapi.hpp"
#include "qhttpserverfwd.hpp"

#include <QObject>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
///////////////////////////////////////////////////////////////////////////////

class QHttpAbstractInput : public QObject
{
    Q_OBJECT

public:
    /** Return all the headers in the incomming packet.
     * This returns a reference. If you want to store headers
     *  somewhere else, where the request may be deleted,
     *  make sure you store them as a copy.
     * @note All header names are <b>lowercase</b> . */
    virtual const THeaderHash&  headers() const=0;

    /** The HTTP version of the packet.
     * @return A string in the form of "x.x" */
    virtual const QString&      httpVersion() const=0;

    /** If this packet was successfully received.
     * Set before end() has been emitted, stating whether
     *  the message was properly received. This is false
     *  until the receiving the full request has completed. */
    virtual bool                isSuccessful() const=0;

signals:
    /** Emitted when new body data has been received.
     * @note This may be emitted zero or more times depending on the transfer type.
     * @param data Received data. */
    void                        data(const QByteArray &data);

    /** Emitted when the incomming packet has been fully received.
     * @note The no more data() signals will be emitted after this. */
    void                        end();

public:
    virtual                    ~QHttpAbstractInput();

protected:
    explicit                    QHttpAbstractInput(QObject* parent);

    Q_DISABLE_COPY(QHttpAbstractInput)
};

///////////////////////////////////////////////////////////////////////////////

class QHttpAbstractOutput : public QObject
{
    Q_OBJECT

public:
    /** changes the HTTP version string ex: "1.1" or "1.0".
     * version is "1.1" set by default. */
    virtual void            setVersion(const QString& versionString)=0;

    /** adds an HTTP header to the packet.
     * @note this method does not actually write anything to socket, just prepares the headers(). */
    virtual void            addHeader(const QByteArray& field, const QByteArray& value)=0;

    /** returns all the headers that already been set. */
    virtual THeaderHash&    headers()=0;

    /** writes a block of data into the HTTP packet.
     * @note headers are written (flushed) before any data.
     * @warning after calling this method add a new header, set staus code, set Url have no effect! */
    virtual void            write(const QByteArray &data)=0;

    /** ends (finishes) the outgoing packet by calling write().
     * headers and data will be flushed to the underlying socket.
     *
     * @sa write() */
    virtual void            end(const QByteArray &data = QByteArray())=0;

signals:
    /** Emitted when all the data has been sent.
     * this signal indicates that the underlaying socket has transmitted all
     *  of it's buffered data. */
    void                    allBytesWritten();

    /** Emitted when the packet is finished and reports if it was the last packet.
     * if it was the last packet (google for "Connection: keep-alive / close")
     *  the http connection (socket) will be closed automatically. */
    void                    done(bool wasTheLastPacket);

public:
    virtual                ~QHttpAbstractOutput();

protected:
    explicit                QHttpAbstractOutput(QObject* parent);

    Q_DISABLE_COPY(QHttpAbstractOutput)
};

///////////////////////////////////////////////////////////////////////////////
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPABSTRACTS_HPP
