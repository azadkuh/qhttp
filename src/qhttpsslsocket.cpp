#include "qhttp/qhttpsslsocket.hpp"
#include "qhttp/qhttpabstracts.hpp"

#include <QSslConfiguration>
#include <QHostAddress>
#include <QFile>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace ssl {
namespace {
///////////////////////////////////////////////////////////////////////////////
bool
appendTrusted(QSslConfiguration& sslconfig, const CertificateList& cas) {
    if (cas.isEmpty())
        return false;

    auto trusted = cas;
    trusted << sslconfig.caCertificates()
#if QT_VERSION >= 0x050500
            << QSslConfiguration::systemCaCertificates()
#endif
    ;
    sslconfig.setCaCertificates(trusted);

    return true;
}
///////////////////////////////////////////////////////////////////////////////
} // namespace anon
///////////////////////////////////////////////////////////////////////////////

QSslCertificate
loadCertificate(const QString& filePath, QSsl::EncodingFormat fmt) {
    QFile f(filePath);
    if (f.open(QFile::ReadOnly)) {
        return QSslCertificate{f.readAll(), fmt};
    }

    return QSslCertificate{};
}

QSslKey
loadKey(
    const QString& filePath, QSsl::KeyAlgorithm alg, QSsl::EncodingFormat fmt) {
    QFile f(filePath);
    if (f.open(QFile::ReadOnly)) {
        return QSslKey{f.readAll(), alg, fmt};
    }

    return QSslKey{};
}
///////////////////////////////////////////////////////////////////////////////

ssl::Socket::Socket(QObject* parent) : QSslSocket(parent) {
    // log ssl errors
    void (QSslSocket::*serrFunc)(const QList<QSslError>&) =
        &QSslSocket::sslErrors;
    QObject::connect(this, serrFunc, [this](const auto& errors) {
        this->onSslErrors(errors);
    });

    QObject::connect(
        this, &QSslSocket::peerVerifyError, [this](const QSslError& err) {
        this->onPeerVerifyError(err);
    });
}

void
Socket::onSslErrors(const QList<QSslError>& errors) {
    qDebug("ssl errors from %s:%d",
        qPrintable(Stringify::toStringV4(peerAddress())),
        peerPort());

    for (const auto& err : errors) {
        qDebug("  (%d): %s",
            static_cast<int>(err.error()),
            qPrintable(err.errorString()));
    }
}

void
Socket::onPeerVerifyError(const QSslError& err) {
    qDebug("peer certificate failed (%d): %s",
        static_cast<int>(err.error()),
        qPrintable(err.errorString()));
}

void
ssl::Socket::setup(const CertificateList& cas) {
    auto sslconfig = sslConfiguration();
    if (appendTrusted(sslconfig, cas))
        setSslConfiguration(sslconfig);
}

void ssl::Socket::setup(const QSslKey& mykey, const QSslCertificate& mycert,
                        const CertificateList& cas) {
    auto sslconfig = sslConfiguration();
    sslconfig.setPrivateKey(mykey);
    sslconfig.setLocalCertificate(mycert);
    appendTrusted(sslconfig, cas);
    setSslConfiguration(sslconfig);
}

void
ssl::Socket::acceptSelfSignedCertificates(const CertificateList& certs) {
    if (certs.isEmpty())
        return;

    QList<QSslError> errors;
    for ( const auto& cert : certs ) {
        errors.append({QSslError::SelfSignedCertificate, cert});
    }

    ignoreSslErrors(errors);
}
///////////////////////////////////////////////////////////////////////////////
} // namespace ssl
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
