#ifndef HTTPCLIENT_HPP
#define HTTPCLIENT_HPP
///////////////////////////////////////////////////////////////////////////////
#include <QObject>
///////////////////////////////////////////////////////////////////////////////
class HttpClientPrivate;

class HttpClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 clientId     READ clientId)
    Q_PROPERTY(QString address      READ address        WRITE setAddress)
    Q_PROPERTY(quint16 port         READ port           WRITE setPort)
    Q_PROPERTY(quint32 requestCount READ requestCount   WRITE setRequestCount)
    Q_PROPERTY(quint32 sleepTime    READ sleepTime      WRITE setSleepTime)
    Q_PROPERTY(quint32 commandCount READ commandCount)
    Q_PROPERTY(bool    keepAlive    READ keepAlive      WRITE setKeepAlive)

public:
    explicit        HttpClient(quint32 clientId, QObject *parent = 0);
    virtual        ~HttpClient();

signals:
    void            finished(int clientId, size_t sentCommands);

public slots:
    void            start();

public:     // properties
    quint32         clientId() const;

    const QString&  address() const;
    void            setAddress(const QString&);

    quint16         port() const;
    void            setPort(quint16);

    quint32         requestCount() const;
    void            setRequestCount(quint32);

    quint32         sleepTime() const;
    void            setSleepTime(quint32);

    quint32         commandCount() const;

    bool            keepAlive() const;
    void            setKeepAlive(bool);

protected:
    void            timerEvent(QTimerEvent*);

private:
    Q_DECLARE_PRIVATE(HttpClient)
    Q_DISABLE_COPY(HttpClient)

    QScopedPointer<HttpClientPrivate> d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
#endif // HTTPCLIENT_HPP
