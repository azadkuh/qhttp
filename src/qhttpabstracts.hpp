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
    virtual ~QHttpAbstractInput();

protected:
    explicit QHttpAbstractInput(QObject* parent);

    Q_DISABLE_COPY(QHttpAbstractInput)
};

///////////////////////////////////////////////////////////////////////////////
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPABSTRACTS_HPP
