// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2016 Clifford Jolly
//     http://expiredpopsicle.com
//     expiredpopsicle@gmail.com
//
// ---------------------------------------------------------------------------
//
//   This software is provided 'as-is', without any express or implied
//   warranty. In no event will the authors be held liable for any
//   damages arising from the use of this software.
//
//   Permission is granted to anyone to use this software for any
//   purpose, including commercial applications, and to alter it and
//   redistribute it freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must
//      not claim that you wrote the original software. If you use this
//      software in a product, an acknowledgment in the product
//      documentation would be appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must
//      not be misrepresented as being the original software.
//
//   3. This notice may not be removed or altered from any source
//      distribution.
//
// -------------------------- END HEADER -------------------------------------

#include <string>
#include <sstream>
#include <iostream>
#include <inttypes.h>
#include <cassert>
using namespace std;

#if !_WIN32
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#else
#include <windows.h>
#include <winsock.h>
#endif

#include "expopsockets.h"

#if _WIN32
#define CLOSE_SYSTEM closesocket
#define CONST_DATA_CAST(x) ((const char*)(x))
#define DATA_CAST(x) ((char*)(x))
typedef int ADDR_LEN_TYPE;
#else
#define CLOSE_SYSTEM close
#define CONST_DATA_CAST(x) ((const void*)(x))
#define DATA_CAST(x) ((void*)(x))
typedef unsigned int ADDR_LEN_TYPE;
#endif

namespace ExPop {

    // Windows keeps some stupid socket state in the background, so I
    // suppose we have to too. Derp.
    static unsigned int socketCounter = 0;
    static bool socketsDoneInit = false;

    void socketsInit(void) {
      #if _WIN32
        WSADATA wsaData = {0};
        int startupResult = WSAStartup(MAKEWORD(2, 0), &wsaData);
        assert(!startupResult);
      #endif
        socketsDoneInit = true;
    }

    void socketsShutdown(void) {
      #if _WIN32
        WSACleanup();
      #endif
        socketsDoneInit = false;
    }

    void Socket::constructSocket(void) {
        int internalSocketType = (type == SOCKETTYPE_TCP) ? SOCK_STREAM : SOCK_DGRAM;
        fd = socket(AF_INET, internalSocketType, 0);
    }

    Socket::Socket(SocketType type) {

        assert(socketsDoneInit);

        socketCounter++;

        // FIXME: Implement UDP.
        assert(type == SOCKETTYPE_TCP);

        this->type = type;
        state = SOCKETSTATE_DISCONNECTED;
        constructSocket();
    }

    Socket::Socket(int fd) {
        type = SOCKETTYPE_TCP;
        state = SOCKETSTATE_CONNECTED;
        this->fd = fd;
    }

    Socket::~Socket(void) {
        CLOSE_SYSTEM(fd);

        socketCounter--;
    }

    void Socket::disconnect(void) {

        assert(
            state == SOCKETSTATE_CONNECTED ||
            state == SOCKETSTATE_LISTENING);

        CLOSE_SYSTEM(fd);
        constructSocket();
        state = SOCKETSTATE_DISCONNECTED;
    }

    bool Socket::connectTo(const std::string &hostName, uint16_t port) {

        assert(type == SOCKETTYPE_TCP);
        assert(state == SOCKETSTATE_DISCONNECTED);

        state = SOCKETSTATE_HOSTLOOKUP;

        // This blocks.
        hostent *host = gethostbyname(hostName.c_str());

        if(!host) {
            return false;
        }

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        // FIXME: This probably only works correctly for IPv4 stuff.
        addr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

        // The call to connect() blocks.
        state = SOCKETSTATE_CONNECTING;
        int connectStatus = connect(fd, (sockaddr*)&addr, sizeof(addr));
        state = connectStatus ? SOCKETSTATE_DISCONNECTED : SOCKETSTATE_CONNECTED;

        if(state == SOCKETSTATE_DISCONNECTED) {
            return false;
        }

        return true;
    }

    ssize_t Socket::sendData(const void *data, size_t length) {

        assert(state == SOCKETSTATE_CONNECTED);

        ssize_t ret = send(fd, CONST_DATA_CAST(data), length, 0);
        if(ret == 0 || ret == -1) {
            disconnect();
        }

        return ret;
    }

    ssize_t Socket::recvData(void *data, size_t length) {

        assert(state == SOCKETSTATE_CONNECTED);

        ssize_t ret = recv(fd, DATA_CAST(data), length, 0);
        if(ret == 0 || ret == -1) {
            disconnect();
        }

        return ret;
    }

    bool Socket::hasData(void) {

        assert(
            state == SOCKETSTATE_CONNECTED ||
            type == SOCKETTYPE_UDP);

        fd_set readFs;
        timeval tv;

        FD_ZERO(&readFs);
        FD_SET(fd, &readFs);

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        int ret = select(fd + 1, &readFs, NULL, NULL, &tv);

        if(ret == -1) {
            disconnect();
            return false;
        }

        if(FD_ISSET(fd, &readFs)) {
            return true;
        }

        return false;
    }

    SocketState Socket::getState(void) {
        return state;
    }

    bool Socket::startListening(uint16_t port) {

        assert(state == SOCKETSTATE_DISCONNECTED);
        assert(type == SOCKETTYPE_TCP);

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        if(bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
            // Something already on this port?
            state = SOCKETSTATE_DISCONNECTED;
            return false;
        }

        if(listen(fd, 4) < 0) {
            // The bind() succeeded but the listen() failed, leaving it in
            // a weird state, so just reset.
            disconnect();
            return false;
        }

        state = SOCKETSTATE_LISTENING;
        return true;
    }

    Socket *Socket::acceptConnection(std::string *addrStr) {

        assert(type == SOCKETTYPE_TCP);
        assert(state == SOCKETSTATE_LISTENING);

        sockaddr_in addr;
        ADDR_LEN_TYPE addrLen = sizeof(addr);
        Socket *ret = new Socket(accept(fd, (sockaddr*)&addr, &addrLen));

        if(addrStr) {

            // FIXME? This might have endian issues.
            ostringstream addrStrOut;
            addrStrOut <<
                (unsigned int)((unsigned char*)&addr.sin_addr.s_addr)[0] << "." <<
                (unsigned int)((unsigned char*)&addr.sin_addr.s_addr)[1] << "." <<
                (unsigned int)((unsigned char*)&addr.sin_addr.s_addr)[2] << "." <<
                (unsigned int)((unsigned char*)&addr.sin_addr.s_addr)[3];
            *addrStr = addrStrOut.str();
        }

        ret->type = SOCKETTYPE_TCP;
        ret->state = SOCKETSTATE_CONNECTED;

        return ret;
    }

}

