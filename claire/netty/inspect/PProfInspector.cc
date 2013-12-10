// Copyright (c) 2013 The claire-netty Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <claire/netty/inspect/PProfInspector.h>

#include <gperftools/profiler.h>
#include <gperftools/malloc_extension.h>

#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <claire/netty/http/HttpRequest.h>
#include <claire/netty/http/HttpResponse.h>
#include <claire/netty/http/HttpServer.h>

#include <claire/common/events/EventLoop.h>
#include <claire/common/symbolizer/Symbolizer.h>
#include <claire/common/files/FileUtil.h>
#include <claire/common/logging/Logging.h>

static const char kProfileFile[] = "profile.dat";
static const int kDefaultProfileSeconds = 30;

namespace claire {

PProfInspector::PProfInspector(HttpServer* server)
    : server_(server)
{
    server_->Register("/pprof/profile",
                      boost::bind(&PProfInspector::OnProfile, this, _1),
                      false);
    server_->Register("/pprof/heap",
                      boost::bind(&PProfInspector::OnHeap, _1),
                      false);
    server_->Register("/pprof/growth",
                      boost::bind(&PProfInspector::OnGrowth, _1),
                      false);
    server_->Register("/pprof/cmdline",
                      boost::bind(&PProfInspector::OnCmdline, _1),
                      false);
    server_->Register("/pprof/symbol",
                      boost::bind(&PProfInspector::OnSymbol, _1),
                      false);
}

void PProfInspector::OnProfile(const HttpConnectionPtr& connection)
{
    auto request = connection->mutable_request();
    if (request->method() != HttpRequest::kGet)
    {
        connection->OnError(HttpResponse::k400BadRequest,
                            "Only accept Get method");
        return ;
    }

    int seconds;
    auto seconds_parameter = request->get_parameter("seconds");
    if (seconds_parameter.length() > 0)
    {
        seconds = atoi(seconds_parameter.c_str());
    }
    else
    {
        seconds = kDefaultProfileSeconds;
    }

    {
        MutexLock lock(mutex_);
        if (connections_.empty())
        {
            ProfilerStart(kProfileFile);
            EventLoop::CurrentLoopInThisThread()->RunAfter(seconds*1000,
                                                           boost::bind(&PProfInspector::OnProfileComplete, this));
        }
        connections_.insert(connection->id());
    }
}

void PProfInspector::OnProfileComplete()
{
    ProfilerFlush();
    ProfilerStop();

    std::set<HttpConnection::Id> connections;
    {
        MutexLock lock(mutex_);
        connections.swap(connections_);
    }

    std::string output;
    FileUtil::ReadFileToString(kProfileFile, &output);
    for (auto it = connections.cbegin(); it != connections.cend(); ++it)
    {
        server_->SendByHttpConnectionId(*it, output);
        server_->Shutdown(*it);
    }
}

void PProfInspector::OnHeap(const HttpConnectionPtr& connection)
{
    std::string output;
    MallocExtension::instance()->GetHeapSample(&output);

    connection->Send(output);
    connection->Shutdown();
}

void PProfInspector::OnGrowth(const HttpConnectionPtr& connection)
{
    std::string output;
    MallocExtension::instance()->GetHeapGrowthStacks(&output);

    connection->Send(output);
    connection->Shutdown();
}

void PProfInspector::OnCmdline(const HttpConnectionPtr& connection)
{
    std::string output;
    FileUtil::ReadFileToString("/proc/self/cmdline", &output);
    for (size_t i = 0; i < output.length(); i++)
    {
        if (output[i] == '\0')
        {
            output[i] = '\n';
        }
    }

    connection->Send(output);
    connection->Shutdown();
}

void PProfInspector::OnSymbol(const HttpConnectionPtr& connection)
{
    auto request = connection->mutable_request();
    if (request->method() == HttpRequest::kGet)
    {
        Buffer buffer;
        buffer.Append("num_symbols: 1\n");
        connection->Send(&buffer);
        connection->Shutdown();
        return ;
    }

    if (request->method() != HttpRequest::kPost)
    {
        connection->OnError(HttpResponse::k400BadRequest,
                            "Only accept Post method");
        return ;
    }

    std::vector<std::string> addresses;
    boost::algorithm::split(addresses, request->body(), boost::is_any_of("+"));

    HttpResponse response;
    for (auto it = addresses.begin(); it != addresses.end(); ++it)
    {
        StringPiece name;
        Symbolizer symbolizer;
        Dwarf::LocationInfo location;

        response.mutable_body()->append(*it);
        response.mutable_body()->append("\t");
        auto address = static_cast<uintptr_t>(strtoull((*it).c_str(), NULL, 0));
        if (symbolizer.Symbolize(address, &name, &location))
        {
            response.mutable_body()->append(name.ToString());
            response.mutable_body()->append("\n");
        }
        else
        {
            response.mutable_body()->append("unknown\n");
        }
    }

    connection->Send(&response);
}

} // namespace claire
