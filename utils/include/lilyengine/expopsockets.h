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

// FIXME: This is incomplete. We still don't have support for UDP
// sockets, for example.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <lilyengine/config.h>

#if EXPOP_ENABLE_SOCKETS

#if !_WIN32
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#else
#include <windows.h>
#include <winsock.h>
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <cassert>

#endif

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_SOCKETS
namespace ExPop
{
    enum SocketType
    {
        SOCKETTYPE_TCP,
        SOCKETTYPE_UDP,
    };

    enum SocketState
    {
        SOCKETSTATE_DISCONNECTED,
        SOCKETSTATE_HOSTLOOKUP,
        SOCKETSTATE_CONNECTING,
        SOCKETSTATE_CONNECTED,

        SOCKETSTATE_LISTENING,
    };

    /// Socket type. Overrides streambuf so we can make an ostream out
    /// of it for extra laziness in our text-only communications.
    /// Sockets don't have background state or shared data aside from
    /// whatever the OS tracks, so it's safe to have multiple Socket
    /// instances across multiple threads.
    class Socket : public std::streambuf
    {
    public:

        Socket(SocketType type = SOCKETTYPE_TCP);

        ~Socket(void);

        /// Connect. FIXME: Does a hostname lookup that will block
        /// until it completes. Currently recommend multi-threaded
        /// approach if this is unacceptable.
        bool connectTo(const std::string &hostName, uint16_t port);

        /// Send data.
        ssize_t sendData(const void *data, size_t length);

        /// Get data.
        ssize_t recvData(void *data, size_t length);

        /// Returns true if data is available (or connections are
        /// ready to accept for a listening socket).
        bool hasData(void);

        /// Query the state (connected, disconnected, etc). This is
        /// meant to let other threads watch the state of the socket,
        /// so it is thread safe.
        SocketState getState(void);

        /// Returns true on success.
        bool startListening(uint16_t port);

        /// Returns a pointer to the new socket that should be deleted
        /// when done.
        Socket *acceptConnection(std::string *addrStr = NULL);

        /// Disconnect the socket.
        void disconnect(void);

        // streambuf overrides.
        int uflow() override;
        int underflow() override;
        int overflow(int c = EOF) override;

        std::string getPeerAddress() const;

    private:

        // These are both left intentionally undefined. Using the
        // defaults would be bad, however.
        Socket(const Socket &socket);
        Socket &operator=(const Socket &socket);

        // This is just for accept()ing new connections.
        Socket(int fd);

        // TCP or UDP.
        SocketType type;

        // State of the socket. Asserts for consistency and expected
        // to be readable by other threads for simple status updates.
        SocketState state;

        // System file descriptor.
        int fd;

        // Some of the streambuf stuff makes us keep a one-byte buffer
        // because they can read the stream without advancing. This is
        // only used by the streambuf overrides. So if you peek at a
        // byte with underflow() and then expect to read it back again
        // with recvData(), you're going to have a bad time.
        char inputBuffer;
        bool inputBufferFull;

        void constructSocket(void);
        void initCommon(void);
    };
}

#endif

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_SOCKETS

namespace ExPop
{
    // We need to handle some minor OS API differences between POSIX
    // sockets and Windows sockets.
  #if _WIN32
    inline void socketsCloseOSSpecfic(int x) { closesocket(x); }
    inline const char *socketsConstDataCast(const void *x) { return (const char*)(x); }
    inline char *socketsDataCast(void *x) { return (char*)(x); }
    typedef int SocketsAddrLen;
  #if !__GNUC__
    // Under Visual Studio, we can just have the lib linked here, I
    // guess.
  #pragma comment(lib, "ws2_32.lib")
  #endif
  #else
    inline void socketsCloseOSSpecfic(int x) { close(x); }
    inline const void *socketsConstDataCast(const void *x) { return (const void*)(x); }
    inline void *socketsDataCast(void *x) { return (void*)(x); }
  #if __CYGWIN__
    // Cygwin, why are you such a pain in my ass?
    typedef int SocketsAddrLen;
  #else
    typedef unsigned int SocketsAddrLen;
  #endif
  #endif

    inline void Socket::initCommon(void)
    {
        inputBufferFull = false;
      #if _WIN32
        WSADATA wsaData = {0};
        int startupResult = WSAStartup(MAKEWORD(2, 0), &wsaData);
        assert(!startupResult);
      #endif
    }

    inline void Socket::constructSocket(void)
    {
        int internalSocketType = (type == SOCKETTYPE_TCP) ? SOCK_STREAM : SOCK_DGRAM;
        fd = socket(AF_INET, internalSocketType, 0);
    }

    inline Socket::Socket(SocketType type)
    {
        initCommon();

        // FIXME: Implement UDP.
        assert(type == SOCKETTYPE_TCP);

        this->type = type;
        state = SOCKETSTATE_DISCONNECTED;
        constructSocket();
    }

    inline Socket::Socket(int fd)
    {
        initCommon();

        type = SOCKETTYPE_TCP;
        state = SOCKETSTATE_CONNECTED;
        this->fd = fd;
    }

    inline Socket::~Socket(void)
    {
        socketsCloseOSSpecfic(fd);

      #if _WIN32
        WSACleanup();
      #endif
    }

    inline void Socket::disconnect(void)
    {
        // assert(
        //     state == SOCKETSTATE_CONNECTED ||
        //     state == SOCKETSTATE_LISTENING);

        socketsCloseOSSpecfic(fd);
        constructSocket();
        state = SOCKETSTATE_DISCONNECTED;
    }

    inline bool Socket::connectTo(const std::string &hostName, uint16_t port)
    {
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

    inline ssize_t Socket::sendData(const void *data, size_t length)
    {
        // assert(state == SOCKETSTATE_CONNECTED);

        ssize_t ret = send(fd, socketsConstDataCast(data), length, 0);
        if(ret == 0 || ret == -1) {
            disconnect();
        }

        return ret;
    }

    inline ssize_t Socket::recvData(void *data, size_t length)
    {
        // assert(state == SOCKETSTATE_CONNECTED);

        ssize_t ret = recv(fd, socketsDataCast(data), length, 0);
        if(ret == 0 || ret == -1) {
            disconnect();
        }

        return ret;
    }

    inline bool Socket::hasData(void)
    {
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

    inline SocketState Socket::getState(void)
    {
        return state;
    }

    inline bool Socket::startListening(uint16_t port)
    {
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

    inline Socket *Socket::acceptConnection(std::string *addrStr)
    {
        assert(type == SOCKETTYPE_TCP);
        assert(state == SOCKETSTATE_LISTENING);

        sockaddr_in addr;
        SocketsAddrLen addrLen = sizeof(addr);
        Socket *ret = new Socket(accept(fd, (sockaddr*)&addr, &addrLen));

        if(addrStr) {

            // FIXME? This might have endian issues.
            std::ostringstream addrStrOut;
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

    inline std::string Socket::getPeerAddress() const
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        socklen_t addrLen = sizeof(addr);
        getpeername(fd, (sockaddr*)&addr, &addrLen);

        std::ostringstream addrStr;

        if(addr.sin_family == AF_INET) {
            addrStr << ((addr.sin_addr.s_addr & 0x000000ff) >> 0 ) << ".";
            addrStr << ((addr.sin_addr.s_addr & 0x0000ff00) >> 8 ) << ".";
            addrStr << ((addr.sin_addr.s_addr & 0x00ff0000) >> 16) << ".";
            addrStr << ((addr.sin_addr.s_addr & 0xff000000) >> 24) << ":";
            addrStr << htons(addr.sin_port) << std::endl;
        } else {
            // TODO: Implement ipv6.
        }

        return addrStr.str();
    }

    inline int Socket::uflow()
    {
        if(inputBufferFull) {
            inputBufferFull = false;
            return inputBuffer;
        }

        if(getState() != SOCKETSTATE_CONNECTED) {
            return EOF;
        }

        char buf[2] = { 0, 0 };
        size_t dataSize = recvData(&buf, 1);

        if(!dataSize) {
            return EOF;
        }

        return buf[0];
    }

    inline int Socket::underflow()
    {
        if(inputBufferFull) {
            return inputBuffer;
        }

        size_t dataSize = recvData(&inputBuffer, 1);

        if(!dataSize) {
            return EOF;
        }

        inputBufferFull = true;
        return inputBuffer;
    }

    inline int Socket::overflow(int c)
    {
        if(getState() != SOCKETSTATE_CONNECTED) {
            return EOF;
        }

        char buf = (char)c;
        size_t dataSize = sendData(&c, 1);

        if(!dataSize) {
            return EOF;
        }

        return buf;
    }

}

#endif

