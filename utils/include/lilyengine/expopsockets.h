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

#pragma once

#include <lilyengine/config.h>

#if EXPOP_ENABLE_SOCKETS

#if !_WIN32
#include <sys/types.h>
#endif

#include <string>

namespace ExPop {

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

    class Socket : public streambuf
    {
    public:

        Socket(SocketType type = SOCKETTYPE_TCP);

        ~Socket(void);

        bool connectTo(const std::string &hostName, uint16_t port);

        ssize_t sendData(const void *data, size_t length);
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

        int uflow() override;
        int underflow() override;
        int overflow(int c = EOF) override;

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

