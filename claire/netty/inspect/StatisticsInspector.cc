// Copyright (c) 2013 The claire-netty Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <claire/netty/inspect/StatisticsInspector.h>

#include <string>

#include <boost/bind.hpp>

#include <claire/netty/http/HttpServer.h>
#include <claire/netty/http/HttpRequest.h>
#include <claire/netty/http/HttpResponse.h>
#include <claire/netty/http/HttpConnection.h>

#include <claire/common/metrics/HistogramRecorder.h>

namespace claire {

StatisticsInspector::StatisticsInspector(HttpServer* server)
{
    server->Register("/histograms",
                     boost::bind(&StatisticsInspector::OnHistograms, _1),
                     false);
}

void StatisticsInspector::OnHistograms(const HttpConnectionPtr& connection)
{
    HttpResponse response;
    response.set_status(HttpResponse::k200OK);
    HistogramRecorder::instance()->WriteHTMLGraph("", response.mutable_body());
    response.AddHeader("Content-Type", "text/html");
    connection->Send(&response);
}

} // namespace claire
