/*
Copyright(C) 2018 Brandan Tyler Lasley

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
*/
#pragma once
#include <string>
#include <vector>
#include <string>

#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include "cereal/cereal.hpp"

#define RPC_HOSTNAME                        "127.0.0.1"
#define RPC_JSON                            "/json_rpc"

// Methods
#define DAEMON_GET_CONNECTIONS              "get_connections"

struct Connection
{
    bool incoming;
    bool localhost;
    bool local_ip;
    std::string address;
    std::string host;
    std::string ip;
    std::string port;
    std::string peer_id;
    uint64_t recv_count;
    uint64_t recv_idle_time;
    uint64_t send_count;
    uint64_t send_idle_time;
    std::string state;
    uint64_t live_time;
    uint64_t avg_download;
    uint64_t current_download;
    uint64_t avg_upload;
    uint64_t current_upload;
    uint32_t support_flags;
    std::string connection_id;
    uint64_t height;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(
            CEREAL_NVP(incoming),
            CEREAL_NVP(localhost),
            CEREAL_NVP(local_ip),
            CEREAL_NVP(address),
            CEREAL_NVP(host),
            CEREAL_NVP(ip),
            CEREAL_NVP(port),
            CEREAL_NVP(peer_id),
            CEREAL_NVP(recv_count),
            CEREAL_NVP(recv_idle_time),
            CEREAL_NVP(send_count),
            CEREAL_NVP(send_idle_time),
            CEREAL_NVP(state),
            CEREAL_NVP(live_time),
            CEREAL_NVP(avg_download),
            CEREAL_NVP(current_download),
            CEREAL_NVP(avg_upload),
            CEREAL_NVP(current_upload),
            CEREAL_NVP(support_flags),
            CEREAL_NVP(connection_id),
            CEREAL_NVP(height)
        );
    }
};

class RPC
{
public:
    RPC();
    RPC(const RPC & obj);
    void                            open(unsigned short _port);
    std::vector<Connection>         getConnections(int id = 0) const;
    unsigned short                  getPort() const;
    RPC&                            operator=(const RPC &rhs);

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(CEREAL_NVP(port));
    }
private:
    unsigned short                  port;

    void                            handleNetworkError(const std::string & msg) const;
    void                            handleRPCError(Poco::DynamicStruct error) const;
    Poco::DynamicStruct             getDataFromRPC(const std::string & method, const Poco::DynamicStruct & args, int id = 0) const;
};