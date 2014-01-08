// Copyright (c) 2013 The claire-netty Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <claire/netty/resolver/ResolverFactory.h>
#include <claire/netty/resolver/DnsResolver.h>
#include <claire/netty/resolver/StaticAddressResolver.h>
#include <claire/common/logging/Logging.h>

namespace claire {

ResolverFactory::ResolverFactory()
{
    creators_.insert(std::make_pair("static", &ResolverCreator<StaticAddressResolver>));
}

ResolverFactory::~ResolverFactory() {}

ResolverFactory* ResolverFactory::instance()
{
    return Singleton<ResolverFactory>::instance();
}

Resolver* ResolverFactory::Create(const std::string& name) const
{
    MutexLock lock(mutex_);
    auto it = creators_.find(name);
    if (it == creators_.end())
    {
        return NULL;
    }
    return (*(it->second))();
}

void ResolverFactory::Register(const std::string& name, Creator creator)
{
    MutexLock lock(mutex_);
    auto it = creators_.find(name);
    if (it != creators_.end())
    {
        LOG(FATAL) << "ResolverFactory: " << name << " already registered";
    }
    else
    {
        creators_.insert(std::make_pair(name, creator));
    }
}

} // namespace claire
