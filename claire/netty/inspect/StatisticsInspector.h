// Copyright (c) 2013 The claire-netty Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef _CLAIRE_NETTY_STATISTICSINSPECTOR_H_
#define _CLAIRE_NETTY_STATISTICSINSPECTOR_H_

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace claire {

class HttpServer;
class HttpConnection;
typedef boost::shared_ptr<HttpConnection> HttpConnectionPtr;

class StatisticsInspector : boost::noncopyable
{
public:
    explicit StatisticsInspector(HttpServer* server);

private:
    static void OnHistograms(const HttpConnectionPtr& connection);
    static void OnCounters(const HttpConnectionPtr& connection);
};

} // namespace claire

#endif // _CLAIRE_NETTY_STATISTICSINSPECTOR_H_
