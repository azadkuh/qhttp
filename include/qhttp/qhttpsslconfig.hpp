/** @file qhttpsslconfig.hpp
 *
 * @copyright (C) 2016
 * @date 2016.10.06
 * @version 1.0.0
 * @author amir zamani <azadkuh@live.com>
 *
 */

#ifndef __QHTTPSSLCONFIG_HPP__
#define __QHTTPSSLCONFIG_HPP__

#include <QSslKey>
#include <QSslCertificate>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace ssl {
///////////////////////////////////////////////////////////////////////////////
// helper functions

/// loads an SSL certificate from filePath.
QSslCertificate
loadCertificate(const QString& filePath, QSsl::EncodingFormat fmt = QSsl::Pem);

/// loads an SSL key (private) from filePath.
QSslKey
loadKey(
    const QString&       filePath,
    QSsl::KeyAlgorithm   alg = QSsl::Rsa,
    QSsl::EncodingFormat fmt = QSsl::Pem);
///////////////////////////////////////////////////////////////////////////////

using CertificateList = QList<QSslCertificate>;

/// ssl configuration for server or client
struct Config {

    /// local private key
    QSslKey         myKey;
    /// local certificate
    QSslCertificate myCertificate;

    /// trusted Ca list if any
    CertificateList caCertificates;
    /// if the list is not empty, ignores these self-signed certificates
    CertificateList ignorableSelfSigned;

    bool hasLocal() const {
        return !myKey.isNull() && !myCertificate.isNull();
    }

    explicit Config(const QString& keyFilePath, const QString& certFilePath)
        : myKey(loadKey(keyFilePath)),
          myCertificate(loadCertificate(certFilePath)) {}

    Config() = default;
}; // struct Config

///////////////////////////////////////////////////////////////////////////////
} // namespace ssl
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // __QHTTPSSLCONFIG_HPP__
