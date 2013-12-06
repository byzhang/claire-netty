// Copyright (c) 2013 The claire-netty Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <claire/netty/inspect/FlagsInspector.h>

#include <ctemplate/template.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <vector>

#include <boost/bind.hpp>

#include <claire/netty/http/HttpServer.h>
#include <claire/netty/http/HttpRequest.h>
#include <claire/netty/http/HttpResponse.h>
#include <claire/netty/http/HttpConnection.h>
#include <claire/netty/inspect/InspectAssets.h>

namespace claire {

namespace  {

void OnFlagsPage(const HttpConnectionPtr& connection, HttpResponse* response)
{
    std::vector< ::google::CommandLineFlagInfo> flags;
    GetAllFlags(&flags);

    ctemplate::TemplateDictionary dictionary("data");
    dictionary.SetValue("HOST_NAME", connection->mutable_request()->get_header("HOST"));

    auto it = flags.begin();
    while (it != flags.end())
    {
        if (it->filename[0] != '/')
        {
            auto discard_name = it->filename;
            while ((it != flags.end()) && (discard_name == it->filename))
            {
                ++it;
            }
            continue;
        }

        const auto filename = it->filename;
        auto table = dictionary.AddSectionDictionary("FLAG_TABLE");
        table->SetValue("FILE_NAME", filename);

        while ((it != flags.end()) && (filename == it->filename))
        {
            auto row = table->AddSectionDictionary("FLAG_ROW");
            if (!it->is_default)
            {
                row->SetValue("FLAG_STYLE", "color:green");
            }

            row->SetValue("FLAG_NAME", it->name);
            row->SetValue("FLAG_TYPE", it->type);
            row->SetValue("FLAG_DESCRIPTION", it->description);
            row->SetValue("FLAG_DEFAULT_VALUE", it->default_value);
            row->SetValue("FLAG_CURRENT_VALUE", it->current_value);
            ++it;
        }
    }

    ctemplate::StringToTemplateCache("flags.html",
                                     ASSET_flags_html,
                                     ASSET_flags_html_length,
                                     ctemplate::STRIP_WHITESPACE);
    ctemplate::ExpandTemplate("flags.html",
                              ctemplate::STRIP_WHITESPACE,
                              &dictionary,
                              response->mutable_body());
    response->AddHeader("Content-Type", "text/html");
}

void ModifyFlags(const HttpConnectionPtr& connection, HttpResponse* response)
{
    auto body = connection->mutable_request()->mutable_body();

    rapidjson::Document doc;
    doc.Parse<0>(body->c_str());

    rapidjson::Document result;
    result.SetObject();
    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it)
    {
        if (::google::SetCommandLineOption(it->name.GetString(),
                                           it->value.GetString()).empty())
        {
            result.AddMember(it->name, it->value, doc.GetAllocator());
        }
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    result.Accept(writer);

    response->set_status(HttpResponse::k200OK);
    response->AddHeader("Content-Type", "application/json");
    response->mutable_body()->assign(buffer.GetString());
}

} // namespace

FlagsInspector::FlagsInspector(HttpServer* server)
{
    server->Register("/flags",
                     boost::bind(&FlagsInspector::OnFlags, _1),
                     false);

    server->RegisterAsset("/static/flags.html",
                          ASSET_flags_html,
                          ASSET_flags_html_length);
    server->RegisterAsset("/static/flags.js",
                          ASSET_flags_js,
                          ASSET_flags_js_length);
}

void FlagsInspector::OnFlags(const HttpConnectionPtr& connection)
{
    HttpResponse response;
    if (connection->mutable_request()->method() == HttpRequest::kGet)
    {
        OnFlagsPage(connection, &response);
    }
    else if (connection->mutable_request()->method() == HttpRequest::kPost)
    {
        ModifyFlags(connection, &response);
    }
    else
    {
        connection->OnError(HttpResponse::k400BadRequest,
                            "Only accept Get/Post method");
        return ;
    }
    connection->Send(&response);
}

} // namespace claire
