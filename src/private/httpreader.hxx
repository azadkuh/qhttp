/** @file httpreader.hxx
 *
 * @copyright (C) 2016
 * @date 2016.05.26
 * @version 1.0.0
 * @author amir zamani <azadkuh@live.com>
 *
 */

#ifndef QHTTP_HTTPREADER_HXX
#define QHTTP_HTTPREADER_HXX

#include "qhttpbase.hpp"
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace details {
///////////////////////////////////////////////////////////////////////////////

// usage in client::QHttpResponse, server::QHttpRequest
template<class TBase>
class HttpReader : public TBase
{
public:
    enum TReadState {
        EEmpty,
        EPartial,
        EComplete,
        ESent
    };

public:
    void collectData(int atMost) {
        icollectRequired = true;
        icollectCapacity = atMost;
        icollectedData.clear();
        if ( atMost > 0 )
            icollectedData.reserve(atMost);
    }

    bool append(const char* data, int length) {
        if ( !icollectRequired ) // not allowed to collect data
            return false;

        int newLength = icollectedData.length() + length;

        if ( icollectCapacity > 0    &&    newLength > icollectCapacity )
            return false; // the capacity is full

        icollectedData.append(data, length);
        return true;
    }

    // call cb if the message is not finalized yet
    template<class Func>
    void finalizeSending(Func cb) {
        if ( ireadState != EComplete ) {
            ireadState  = EComplete;
            isuccessful = true;

            cb();
        }
    }

public:
    bool       isuccessful      = false;
    TReadState ireadState       = EEmpty;

    /// shall I collect incoming body data by myself?
    bool       icollectRequired = false;
    int        icollectCapacity = 0;
    QByteArray icollectedData;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace details
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTP_HTTPREADER_HXX
