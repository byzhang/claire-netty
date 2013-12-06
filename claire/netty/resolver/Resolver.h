// Copyright (c) 2013 The claire-netty Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef _CLAIRE_NETTY_RESOLVER_RESOLVER_H_
#define _CLAIRE_NETTY_RESOLVER_RESOLVER_H_

#include <string>
#include <vector>

#include <boost/function.hpp>

#include <claire/common/events/EventLoop.h>
#include <claire/netty/InetAddress.h>

namespace claire {

class Resolver
{
public:
    typedef boost::function<void(const std::vector<InetAddress>&)> ResolveCallback;

    virtual ~Resolver() {}

    virtual void Init(EventLoop* loop) {}
    virtual void Resolve(const std::string& address, const ResolveCallback& callback) = 0;
};

} // namespace claire

#endif // _CLAIRE_NETTY_RESOLVER_RESOLVER_H_
