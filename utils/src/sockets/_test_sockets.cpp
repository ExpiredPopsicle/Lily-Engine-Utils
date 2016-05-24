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

#include "expopsockets.h"
using namespace ExPop;

int main(int argc, char *argv[]) {

    socketsInit();

    string hostName = "expiredpopsicle.com";
    string path = "/";

    Socket sock;
    assert(sock.connectTo(hostName, 80));

    ostringstream outBufferStr;
    outBufferStr << "GET " << path << " HTTP/1.1\r\n";
    outBufferStr << "Host: " << hostName << "\r\n";
    outBufferStr << "\r\n";
    string outBuffer = outBufferStr.str();

    sock.sendData(outBuffer.c_str(), outBuffer.size());

    while(!sock.hasData()) {
        cout << "Waiting..." << endl;
    }

    char recvBuf;
    int gotBytes = 0;
    while(sock.getState() == SOCKETSTATE_CONNECTED) {

        gotBytes = sock.recvData(&recvBuf, 1);

        if(gotBytes > 0) {
            cout << recvBuf;
        }

        // cout << "Derp1" << endl;
        // cout << "Derp3 " << (sock.getState() == SOCKETSTATE_CONNECTED) << ", " << sock.hasData() << endl;
        while(sock.getState() == SOCKETSTATE_CONNECTED && !sock.hasData()) {
            // cout << "Waiting for more..." << endl;
        }
        // cout << "Derp2 " << sock.getState() << ", " << sock.hasData() << endl;
    }

    cout << endl;

  // #if 0

  //   Socket *listener = new Socket(SOCKETTYPE_TCP);
  //   while(!listener->startListening(7018)) {
  //       cout << "Waiting to start listening..." << endl;
  //       sleep(1);
  //   }

  //   cout << "Listening..." << endl;
  //   Socket *clientSock = NULL;
  //   string remoteHost;
  //   bool quit = false;
  //   bool quitOuter = false;

  //   while(!quitOuter && (clientSock = listener->acceptConnection(&remoteHost))) {

  //       quit = false;
  //       cout << "Got a connection from " << remoteHost << endl;

  //       while(!quit && !quitOuter) {

  //           char inBuf = 0;

  //           if(clientSock->getState() != SOCKETSTATE_CONNECTED) {
  //               cout << "Disconnected at 1" << endl;
  //               break;
  //           }

  //           clientSock->sendData("HERPDERP: ", 10);

  //           if(clientSock->getState() != SOCKETSTATE_CONNECTED) {
  //               cout << "Disconnected at 2" << endl;
  //               break;
  //           }

  //           do {
  //               clientSock->recvData(&inBuf, 1);

  //               if(clientSock->getState() != SOCKETSTATE_CONNECTED) {
  //                   cout << "Disconnected at 3" << endl;
  //                   quit = true;
  //                   break;
  //               }

  //           } while(inBuf == '\n');

  //           // if(clientSock->getState() != SOCKETSTATE_CONNECTED) {
  //           //     cout << "Disconnected at 4" << endl;
  //           //     break;
  //           // }

  //           if(!quit) {
  //               if(inBuf == 'q') {
  //                   quitOuter = true;
  //               }
  //               if(inBuf == 'd') {
  //                   cout << "AUGHBLARGH" << endl;
  //               }
  //           }
  //       }

  //       delete clientSock;
  //   }

  //   delete listener;

  // #endif

    socketsShutdown();

    return 0;
}
