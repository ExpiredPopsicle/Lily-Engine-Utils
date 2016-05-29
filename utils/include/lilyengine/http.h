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

// Simple HTTP library.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#include <string>
#include <iostream>
#include <sstream>

#include <lilyengine/config.h>
#include <lilyengine/expopsockets.h>
#include <lilyengine/malstring.h>

#if EXPOP_ENABLE_TESTING
#include <lilyengine/testing.h>
#endif

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_SOCKETS

namespace ExPop
{
    inline void httpGet(const std::string &url);

  #if EXPOP_ENABLE_TESTING
    inline void doHttpTests(size_t &passCounter, size_t &failCounter);
  #endif
}

#endif

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_SOCKETS

namespace ExPop
{
    inline void httpGet(const std::string &url)
    {
        // Maliciously encoded URLs can do bad things, and we're
        // expecting this to be able to at least kind of recognize a
        // badly formatted URL. Let's at least reject anything that
        // has whitespace in it.
        for(size_t i = 0; i < url.size(); i++) {
            if(isWhiteSpace(url[i])) {
                return;
            }
        }

        // Parse the URL.
        std::string scheme;
        std::string authority;
        std::string path;
        std::string query;
        std::string fragment;
        stringParseUri(url,
            scheme,
            authority,
            path,
            query,
            fragment);

        // Split up the user/host/port even further.
        string user;
        string host;
        string hostName;
        string hostPort;
        stringSplit(authority, "@", user, host, true);
        stringSplit(host, ":", hostName, hostPort);

        // Port 80 if nothing is specified.
        if(hostPort == "") hostPort = "80";
        uint16_t port = atoi(hostPort.c_str());

        // REMOVEME
        Socket foo;
        Socket bar;

        // Actually initiate the connection and create streams.
        Socket sock;
        sock.connectTo(hostName, port);
        std::ostream out(&sock);
        std::istream in(&sock);

        // Send the request.
        out << "GET " << path << " HTTP/1.1" << std::endl;
        out << "Host: " << host << std::endl;
        out << std::endl;

        // Get the HTTP response.
        std::string httpResponse;
        std::getline(in, httpResponse);
        std::cout << "http response: " << httpResponse << std::endl;
        std::string httpResponseCodeStr;
        std::string httpResponseCodeText;
        std::string httpResponseCodeNumber;
        std::string httpVersionNumber;
        stringSplit(httpResponse, " ", httpVersionNumber, httpResponseCodeStr);
        stringSplit(httpResponseCodeStr, " ", httpResponseCodeNumber, httpResponseCodeText);
        int httpStatusCode = atoi(httpResponseCodeNumber.c_str());

        // Read headers.
        size_t contentLength = 0;
        bool useChunked = false;
        while(sock.getState() == SOCKETSTATE_CONNECTED) {

            // Read a line and tidy it up.
            std::string line;
            std::getline(in, line);
            line = stringReplace<char>("\r", "", line);

            // If we get a blank line, we're done here.
            if(line.size() == 0) break;

            // Determine header type.
            std::string headerType;
            std::string headerValue;
            stringSplit(line, ":", headerType, headerValue);

            // Find the actual start of the header value.
            size_t headerValueStart = 0;
            while(headerValueStart < headerValue.size() && headerValue[headerValueStart] == ' ') {
                headerValueStart++;
            }
            if(headerValueStart && headerValueStart < headerValue.size()) {
                headerValue = headerValue.substr(headerValueStart);
            }

            // Find a content-length header or transfer-encoding for
            // chunked encoding.
            std::string lowerType = stringToLower(headerType);
            if(lowerType == "content-length") {
                contentLength = atoi(headerValue.c_str());
            } else if(lowerType == "transfer-encoding") {
                if(stringToLower(headerValue) == "chunked") {
                    useChunked = true;
                }
            }
        }

        std::cout << "content length is: " << contentLength << std::endl;
        std::cout << "chunked encoding:  " << useChunked << std::endl;

        std::string output;

        while(sock.getState() == SOCKETSTATE_CONNECTED) {

            if(useChunked) {

                // Skip past newlines that might be trailing after
                // last chunk.
                while(sock.underflow() == '\r' || sock.underflow() == '\n') {
                    sock.uflow();
                }

                // Read chunk size.
                std::string hexLengthString;
                while(sock.getState() == SOCKETSTATE_CONNECTED) {
                    char buf[2] = { 0, 0 };
                    buf[0] = sock.uflow();

                    if( (buf[0] >= '0' && buf[0] <= '9') ||
                        (buf[0] >= 'a' && buf[0] <= 'z') ||
                        (buf[0] >= 'A' && buf[0] <= 'Z'))
                    {
                        // Valid hex character. Add this character to the
                        // hex number.
                        hexLengthString = hexLengthString + buf;
                    } else {
                        // Found a non-valid hex character, indicating
                        // that we're done. Skip through to the end of the
                        // line and leave the loop.
                        while(buf[0] != '\n') {
                            buf[0] = sock.uflow();
                        }
                        break;
                    }
                }
                contentLength = strtol(hexLengthString.c_str(), nullptr, 16);
            }

            if(contentLength == 0) {
                // No more chunks!
                break;
            }

            // Actually read the page data.
            while(sock.getState() == SOCKETSTATE_CONNECTED && contentLength) {
                char c = sock.uflow();
                contentLength--;
                output.append(1, c);
            }

            if(!useChunked) {
                break;
            }
        }

        sock.disconnect();

        std::cout << output << std::endl;
        std::cout << endl;
    }

  #if EXPOP_ENABLE_TESTING
    inline void doHttpTests(size_t &passCounter, size_t &failCounter)
    {
        httpGet("http://google.com");
        httpGet("http://butts@expiredpopsicle.com:80/index.html");
        httpGet("http://expiredpopsicle.com");
    }
  #endif
}

#endif






