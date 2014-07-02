
///////////////////////////////////////////////////////////////////////////////
#ifndef Q_HTTP_SERVER_FWD_HPP
#define Q_HTTP_SERVER_FWD_HPP
///////////////////////////////////////////////////////////////////////////////
#include <QHash>
#include <QString>

///////////////////////////////////////////////////////////////////////////////

/** A map of request or response headers. */
typedef QHash<QByteArray, QByteArray>   THeaderHash;
///////////////////////////////////////////////////////////////////////////////

// QHttpServer
class QHttpServer;
class QHttpConnection;
class QHttpRequest;
class QHttpResponse;

// Qt
class QTcpServer;
class QTcpSocket;

// http_parser
struct http_parser_settings;
struct http_parser;

///////////////////////////////////////////////////////////////////////////////

/** Request method enumeration.
 * @note Taken from http_parser.h */
enum THttpMethod {
    EHTTP_DELETE         =  0,        ///< DELETE
    EHTTP_GET            =  1,        ///< GET
    EHTTP_HEAD           =  2,        ///< HEAD
    EHTTP_POST           =  3,        ///< POST
    EHTTP_PUT            =  4,        ///< PUT
    /* pathological */
    EHTTP_CONNECT        =  5,        ///< CONNECT
    EHTTP_OPTIONS        =  6,        ///< OPTIONS
    EHTTP_TRACE          =  7,        ///< TRACE
    /* webdav */
    EHTTP_COPY           =  8,        ///< COPY
    EHTTP_LOCK           =  9,        ///< LOCK
    EHTTP_MKCOL          = 10,        ///< MKCOL
    EHTTP_MOVE           = 11,        ///< MOVE
    EHTTP_PROPFIND       = 12,        ///< PROPFIND
    EHTTP_PROPPATCH      = 13,        ///< PROPPATCH
    EHTTP_SEARCH         = 14,        ///< SEARCH
    EHTTP_UNLOCK         = 15,        ///< UNLOCK
    /* subversion */
    EHTTP_REPORT         = 16,        ///< REPORT
    EHTTP_MKACTIVITY     = 17,        ///< MKACTIVITY
    EHTTP_CHECKOUT       = 18,        ///< CHECKOUT
    EHTTP_MERGE          = 19,        ///< MERGE
    /* upnp */
    EHTTP_MSEARCH        = 20,        ///< M-SEARCH
    EHTTP_NOTIFY         = 21,        ///< NOTIFY
    EHTTP_SUBSCRIBE      = 22,        ///< SUBSCRIBE
    EHTTP_UNSUBSCRIBE    = 23,        ///< UNSUBSCRIBE
    /* RFC-5789 */
    EHTTP_PATCH          = 24,        ///< PATCH
    EHTTP_PURGE          = 25,        ///< PURGE
};

///////////////////////////////////////////////////////////////////////////////

/** HTTP status codes. */
enum TStatusCode {
     ESTATUS_CONTINUE                           = 100,
     ESTATUS_SWITCH_PROTOCOLS                   = 101,
     ESTATUS_OK                                 = 200,
     ESTATUS_CREATED                            = 201,
     ESTATUS_ACCEPTED                           = 202,
     ESTATUS_NON_AUTHORITATIVE_INFORMATION      = 203,
     ESTATUS_NO_CONTENT                         = 204,
     ESTATUS_RESET_CONTENT                      = 205,
     ESTATUS_PARTIAL_CONTENT                    = 206,
     ESTATUS_MULTI_STATUS                       = 207,
     ESTATUS_MULTIPLE_CHOICES                   = 300,
     ESTATUS_MOVED_PERMANENTLY                  = 301,
     ESTATUS_FOUND                              = 302,
     ESTATUS_SEE_OTHER                          = 303,
     ESTATUS_NOT_MODIFIED                       = 304,
     ESTATUS_USE_PROXY                          = 305,
     ESTATUS_TEMPORARY_REDIRECT                 = 307,
     ESTATUS_BAD_REQUEST                        = 400,
     ESTATUS_UNAUTHORIZED                       = 401,
     ESTATUS_PAYMENT_REQUIRED                   = 402,
     ESTATUS_FORBIDDEN                          = 403,
     ESTATUS_NOT_FOUND                          = 404,
     ESTATUS_METHOD_NOT_ALLOWED                 = 405,
     ESTATUS_NOT_ACCEPTABLE                     = 406,
     ESTATUS_PROXY_AUTHENTICATION_REQUIRED      = 407,
     ESTATUS_REQUEST_TIMEOUT                    = 408,
     ESTATUS_CONFLICT                           = 409,
     ESTATUS_GONE                               = 410,
     ESTATUS_LENGTH_REQUIRED                    = 411,
     ESTATUS_PRECONDITION_FAILED                = 412,
     ESTATUS_REQUEST_ENTITY_TOO_LARGE           = 413,
     ESTATUS_REQUEST_URI_TOO_LONG               = 414,
     ESTATUS_REQUEST_UNSUPPORTED_MEDIA_TYPE     = 415,
     ESTATUS_REQUESTED_RANGE_NOT_SATISFIABLE    = 416,
     ESTATUS_EXPECTATION_FAILED                 = 417,
     ESTATUS_INTERNAL_SERVER_ERROR              = 500,
     ESTATUS_NOT_IMPLEMENTED                    = 501,
     ESTATUS_BAD_GATEWAY                        = 502,
     ESTATUS_SERVICE_UNAVAILABLE                = 503,
     ESTATUS_GATEWAY_TIMEOUT                    = 504,
     ESTATUS_HTTP_VERSION_NOT_SUPPORTED         = 505
};

///////////////////////////////////////////////////////////////////////////////
#endif // define Q_HTTP_SERVER_FWD_HPP
