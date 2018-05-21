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
#include "RPC.h"
#include "Util.h"
#include "RPCException.h"
#include <cassert>

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include <Poco/Net/HTTPResponse.h>
#include <cassert>

void RPC::handleNetworkError(const std::string & msg) const
{
    throw RPCConnectionError(msg);
}

void RPC::handleRPCError(Poco::DynamicStruct error) const
{
    throw RPCGeneralError(error["code"].toString(), error["message"].toString());
}

Poco::DynamicStruct RPC::getDataFromRPC(const std::string & method, const Poco::DynamicStruct & args, int id) const
{
    assert(port > 0);

    // Building JSON string
    Poco::DynamicStruct data;

    data["jsonrpc"] = "2.0";
    data["id"] = Poco::format("%d", id);
    data["method"] = method;
    data["params"] = args;

    try
    {
        // Networking
        std::string body = data.toString();
        Poco::Net::HTTPClientSession s(RPC_HOSTNAME, port);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, RPC_JSON);
        Poco::Net::HTTPResponse res;
        Poco::JSON::Parser parser;
        std::stringstream ss;

        // Setup Request
        request.setContentType("application/json");
        request.setContentLength(body.length());
        s.sendRequest(request) << body; // Send POST body

        // Recieve Return Data
        std::istream &is = s.receiveResponse(res);
        Poco::StreamCopier::copyStream(is, ss);

        // Parse JSON and return
        auto JSONResult = parser.parse(ss.str());
        Poco::JSON::Object::Ptr object = JSONResult.extract<Poco::JSON::Object::Ptr>();
        return Poco::DynamicStruct(*object);
    }
    catch (const Poco::Exception & exp)
    {
        handleNetworkError(exp.what());
    }
    return {};
}

RPC::RPC() : port(0)
{

}

RPC::RPC(const RPC& obj)
{
    this->port = obj.port;
}

void RPC::open(unsigned short _port)
{
    port = _port;
}

std::vector<Connection> RPC::getConnections(int id) const
{
    auto json = getDataFromRPC(DAEMON_GET_CONNECTIONS, {}, id);

    // Ensure RPC is happy.
    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
    if (!json["error"].isEmpty())
        handleRPCError(json["error"].extract<Poco::DynamicStruct>());

    auto result = json["result"].extract<Poco::DynamicStruct>();
    auto connections = result["connections"].extract<Poco::Dynamic::Array>();

    std::vector<Connection> connVect;
    struct Connection conn;
    for (auto client : connections)
    {
        conn.incoming = client["incoming"].convert<bool>();
        conn.localhost = client["localhost"].convert<bool>();
        conn.local_ip = client["local_ip"].convert<bool>();
        conn.address = client["address"].convert<std::string>();
        conn.host = client["host"].convert<std::string>();
        conn.ip = client["ip"].convert<std::string>();
        conn.port = client["port"].convert<std::string>();
        conn.peer_id = client["peer_id"].convert<std::string>();
        conn.connection_id = client["connection_id"].convert<std::string>();
        conn.state = client["state"].convert<std::string>();
        conn.recv_count = client["recv_count"].convert<uint64_t>();
        conn.recv_idle_time = client["recv_idle_time"].convert<uint64_t>();
        conn.send_count = client["send_count"].convert<uint64_t>();
        conn.send_idle_time = client["send_idle_time"].convert<uint64_t>();
        conn.live_time = client["live_time"].convert<uint64_t>();
        conn.avg_download = client["avg_download"].convert<uint64_t>();
        conn.current_download = client["current_download"].convert<uint64_t>();
        conn.avg_upload = client["avg_upload"].convert<uint64_t>();
        conn.current_upload = client["current_upload"].convert<uint64_t>();
        conn.support_flags = client["support_flags"].convert<uint64_t>();
        conn.height = client["height"].convert<uint64_t>();
        connVect.emplace_back(conn);
    }

    return connVect;
}

unsigned short RPC::getPort() const
{
    return port;
}

RPC& RPC::operator=(const RPC &rhs)
{
    if (this != &rhs) {
        this->port = rhs.port;
    }
    return *this;
}
