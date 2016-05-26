/** @file httpreader.hxx
 *
 * @copyright (C) 2016
 * @date 2016.05.26
 * @version 1.0.0
 * @author amir zamani <azadkuh@live.com>
 *
 */

#ifndef __QHTTP_HTTPREADER_HXX__
#define __QHTTP_HTTPREADER_HXX__

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
        icollectCapacity = atMost;
        icollectedData.clear();
        if ( atMost > 0 )
            icollectedData.reserve(atMost);
    }

    bool shouldCollect() const {
        return icollectCapacity > 0;
    }

    bool append(const char* data, size_t length) {
        int newLength = icollectedData.length() + (int) length;

        if ( shouldCollect()    &&    newLength >= icollectCapacity )
            return false; // the capacity is full

        icollectedData.append(data, length);
        return true;
    }

public:
    bool       isuccessful      = false;
    TReadState ireadState       = EEmpty;

    int        icollectCapacity = 0;
    QByteArray icollectedData;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace details
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // __QHTTP_HTTPREADER_HXX__
