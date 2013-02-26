#pragma once

#include <inttypes.h>
#include <string>

namespace ExPop {

    enum SocketType {
        SOCKETTYPE_TCP,
        SOCKETTYPE_UDP,
    };

    enum SocketState {

        SOCKETSTATE_DISCONNECTED,
        SOCKETSTATE_HOSTLOOKUP,
        SOCKETSTATE_CONNECTING,
        SOCKETSTATE_CONNECTED,

        SOCKETSTATE_LISTENING,
    };

    class Socket {
    public:

        Socket(SocketType type = SOCKETTYPE_TCP);

        ~Socket(void);

        bool connectTo(const std::string &hostName, uint16_t port);

        ssize_t sendData(const void *data, size_t length);
        ssize_t recvData(void *data, size_t length);

        /// Returns true if data is available (or connections are
        /// ready to accept for a listening socket).
        bool hasData(void);

        SocketState getState(void);

        /// Returns true on success.
        bool startListening(uint16_t port);

        /// Returns a pointer to the new socket that should be deleted
        /// when done.
        Socket *acceptConnection(std::string *addrStr = NULL);

        // TODO: streambuf stuff.

        void disconnect(void);

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

        int fd;

        void constructSocket(void);
    };

    /// Call this before doing anything with sockets.
    void socketsInit(void);

    /// Call this to clean up sockets.
    void socketsShutdown(void);
}

