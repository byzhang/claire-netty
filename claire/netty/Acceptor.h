// Copyright (c) 2013 The claire-netty Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef _CLAIRE_NETTY_ACCEPTOR_H_
#define _CLAIRE_NETTY_ACCEPTOR_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace claire {

class Socket;
class Channel;
class EventLoop;
class InetAddress;

class Acceptor : boost::noncopyable
{
public:
    typedef boost::function<void(Socket&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listen_address);
    ~Acceptor();

    void set_new_connection_callback(const NewConnectionCallback& callback)
    {
        new_connection_callback_ = callback;
    }

    bool listenning() const { return listenning_; }
    void Listen();

private:
    void OnRead();

    EventLoop* loop_;
    std::unique_ptr<Socket> accept_socket_;
    std::unique_ptr<Channel> accept_channel_;
    NewConnectionCallback new_connection_callback_;
    bool listenning_;
};

} // namespace claire

#endif // _CLAIRE_NETTY_ACCEPTOR_H_
