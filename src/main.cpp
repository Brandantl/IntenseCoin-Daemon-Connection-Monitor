#include "Poco/Process.h"
#include "Poco/Pipe.h"
#include "Poco/StreamCopier.h"
#include "Poco/PipeStream.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "RPC.h"
#include "Poco/Thread.h"
#include "RPCException.h"
#include "cereal/cereal.hpp"
#include "cereal/types/map.hpp"
#include "cereal/archives/json.hpp"

#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/SocketAddress.h"
#include <thread>

int main()
{
    RPC test;
    test.open(48782);
    std::map<std::string, Connection> connectMap;
    Poco::Mutex mu;

    // Save map to disk.
    std::ifstream in("connect_map.json");
    if (in.is_open())
    {
        std::cout << "Loading data from disk...\n";
        {
            cereal::JSONInputArchive ar(in);
            ar(CEREAL_NVP(connectMap));
        }
        in.close();
    }

    while (true)
    {
        std::vector<Connection> currConnections;
        try
        {
            currConnections = test.getConnections();
        }
        catch (AppGeneralException & exp)
        {
            std::cout << "Error: " << exp.what() << '\n';
        }

        bool changed = false;
        for (auto conn : currConnections)
        {
            if (!connectMap.count(conn.address))
            {
                mu.lock();
                connectMap[conn.address] = conn;
                mu.unlock();
                changed = true;
            }
        }

        if (changed)
        {
            // Save map to disk.
            std::ofstream out("connect_map.json", std::ios::trunc);
            if (out.is_open())
            {
                std::cout << "Saving data to disk...\n";
                {
                    cereal::JSONOutputArchive ar(out);
                    ar(CEREAL_NVP(connectMap));
                }
                out.close();
            }
        }

        auto serverThread = [&mu, &connectMap]()
        {
            Poco::Net::ServerSocket srv(9098); // does bind + listen
            for (;;)
            {
                Poco::Net::StreamSocket ss = srv.acceptConnection();
                Poco::Net::SocketStream str(ss);

                std::cout << "New Connection: " << ss.address() << '\n';

                mu.lock();
                std::stringstream strstrm;
                {
                    cereal::JSONOutputArchive ar(strstrm);
                    ar(CEREAL_NVP(connectMap));
                }
                mu.unlock();
                std::string data = strstrm.str();

                // Remove white space
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
                data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
                data.erase(std::remove(data.begin(), data.end(), '\t'), data.end());
                data.erase(std::remove(data.begin(), data.end(), ' '), data.end());

                str <<
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: application/json\r\n";
                str << "Content-Length: " << data.length() << "\r\n" << "\r\n"
                    << data
                    << std::flush;
            }
        };
        std::thread td(serverThread);
        td.detach();
        Poco::Thread::sleep(1000);
    }

    return 0;
}
