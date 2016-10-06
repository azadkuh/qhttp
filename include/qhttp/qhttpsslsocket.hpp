/** @file qhttpsslsocket.hpp
 *
 * @copyright (C) 2016
 * @date 2016.10.06
 * @version 1.0.0
 * @author amir zamani <azadkuh@live.com>
 *
 */

#ifndef __QHTTPSSLSOCKET_HPP__
#define __QHTTPSSLSOCKET_HPP__

#include "qhttpsslconfig.hpp"
#include <QSslSocket>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace ssl {
///////////////////////////////////////////////////////////////////////////////

/// a lightweight helper for QSslSocket
struct Socket : public QSslSocket
{
    explicit Socket(QObject* parent);
    virtual ~Socket() = default;

    // by default logs the ssl/tls errors, override to customize
    virtual void onSslErrors(const QList<QSslError>&);
    // by default logs the error, override to customize
    virtual void onPeerVerifyError(const QSslError&);

    /// setup by a list of trusted certificates
    void setup(const CertificateList& trustedCas);

    /// setup for mutual authentication
    void setup(const QSslKey& mykey, const QSslCertificate& mycert,
               const CertificateList& trustedCas);

    /// ignores the insecure self-signed certificates
    void acceptSelfSignedCertificates(const CertificateList&);

    /// helper function
    void setup(const Config& cfg) {
        if (cfg.hasLocal())
            setup(cfg.myKey, cfg.myCertificate, cfg.caCertificates);
        else
            setup(cfg.caCertificates);

        acceptSelfSignedCertificates(cfg.ignorableSelfSigned);
    }

}; // Socket

///////////////////////////////////////////////////////////////////////////////
} // namespace ssl
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // __QHTTPSSLSOCKET_HPP__
