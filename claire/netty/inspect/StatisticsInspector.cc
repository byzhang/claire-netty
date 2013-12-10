// Copyright (c) 2013 The claire-netty Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <claire/netty/inspect/StatisticsInspector.h>

#include <string>

#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <claire/netty/http/HttpServer.h>
#include <claire/netty/http/HttpRequest.h>
#include <claire/netty/http/HttpResponse.h>
#include <claire/netty/http/HttpConnection.h>

#include <claire/netty/inspect/InspectAssets.h>
#include <claire/common/metrics/HistogramRecorder.h>
#include <claire/common/metrics/CounterSampler.h>

namespace claire {

StatisticsInspector::StatisticsInspector(HttpServer* server)
    : sampler_(new CounterSampler(server->loop()))
{
    server->Register("/histograms",
                     boost::bind(&StatisticsInspector::OnHistograms, _1),
                     false);
    server->Register("/counters",
                     boost::bind(&StatisticsInspector::OnCounters, _1),
                     false);
    server->Register("/counterdata",
                     boost::bind(&StatisticsInspector::OnCounterData, this, _1),
                     true);

    server->RegisterAsset("/static/grapher.js",
                          ASSET_grapher_js,
                          ASSET_grapher_js_length);
    server->RegisterAsset("/static/parser.js",
                          ASSET_parser_js,
                          ASSET_parser_js_length);
    server->RegisterAsset("/static/dygraph-extra.js",
                          ASSET_dygraph_extra_js,
                          ASSET_dygraph_extra_js_length);
    server->RegisterAsset("/static/dygraph-combined.js",
                          ASSET_dygraph_combined_js,
                          ASSET_dygraph_combined_js_length);
}

StatisticsInspector::~StatisticsInspector() {}

void StatisticsInspector::OnHistograms(const HttpConnectionPtr& connection)
{
    HttpResponse response;
    response.set_status(HttpResponse::k200OK);
    HistogramRecorder::instance()->WriteHTMLGraph("", response.mutable_body());
    response.AddHeader("Content-Type", "text/html");
    connection->Send(&response);
}

void StatisticsInspector::OnCounters(const HttpConnectionPtr& connection)
{
    HttpResponse response;
    response.set_status(HttpResponse::k200OK);
    response.mutable_body()->assign(ASSET_graphview_html, ASSET_graphview_html_length);
    connection->Send(&response);
}

void StatisticsInspector::OnCounterData(const HttpConnectionPtr& connection)
{
    std::string output;
    if (connection->mutable_request()->uri().query().empty())
    {
        sampler_->WriteJson(&output);
    }
    else
    {
        auto metrics_query = connection->mutable_request()->uri().get_parameter("metrics");
        auto since_query = connection->mutable_request()->uri().get_parameter("since");

        std::vector<std::string> metrics;
        boost::split(metrics, metrics_query, boost::is_any_of(","));

        int64_t since;
        try
        {
           since = boost::lexical_cast<int64_t>(since_query);
        }
        catch (...)
        {
            since = 0;
        }
        sampler_->WriteJson(&output, metrics, since);
    }

    HttpResponse response;
    response.set_status(HttpResponse::k200OK);
    response.AddHeader("Content-Type", "application/json");
    response.mutable_body()->swap(output);
    connection->Send(&response);
}

} // namespace claire
