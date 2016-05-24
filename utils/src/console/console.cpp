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

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>
#include <cassert>
using namespace std;

#include "winhacks.h"
#include "thread.h"
#include "malassert.h"
#include "console.h"

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef OS_ANDROID
#include <android/log.h>
#define  LOGINFO(x...)  __android_log_print(ANDROID_LOG_INFO,"LilyEngine",x)
#endif

namespace ExPop {

    namespace Console {

        static bool mainConsoleDead = false;

        class ConsoleBuf;

        // Main Console class. Instance of this is a singleton.
        class Console {
		public:

			Console(void);
			~Console(void);

			// Adds a line of output that the output thread will
			// handle eventually.
            void addOutputLine(const std::string &line, bool doStdout, bool doGraphical);

			// Get a specific output stream by name.
			ostream &getOutputStream(const std::string &name);

			// Get a specific output buffer by name.
			ConsoleBuf *getOutputBuf(const std::string &name);

			// cout mutex, essentially. Public so that synced buffers
			// can dump to cout instead of adding it to the queue.
            Threads::Mutex combinedOutputMutex;
            Threads::Mutex coutMutex;

            // Get any new lines since the last time this function was
            // called that have gone to a console with graphical
            // output enabled.
            void getNewGraphicalLines(std::vector<std::string> &lines);

		private:

			// Read by the output thread so we know when to stop.
			bool shutdown;

			// std::vector of lines of text to output.
			vector<string> combinedOutputBuffers;
            vector<string> combinedGraphicalOutputBuffers;

			// All the individual buffers.
			map<string, ostream*> outputStreams;
			map<string, ConsoleBuf*> outputBufs;
            Threads::Mutex outputStreamsMutex;

			// Thread that just spews out the text to stdout.
            Threads::Thread outputThread;

			friend std::ostream &out(const string &outStreamName);
			friend void printConsoleBuffers(void *junk);
            friend void consoleShutdown(void);
        };

        // ConsoleBuf is for individual output streams.
        class ConsoleBuf : public streambuf {
		private:

			std::string attribs; // See setAttribs().
			std::string prefix;  // What to print before the text on
                                 // each line. (ie: "ERROR: ")

            // Dump to graphical console?
            bool graphical;

			// Dump to main console, or just drop lines?
			bool enabled;

			// Output in the main thread?
			bool synced;

			// Used for attribs and prefixes.
            Threads::Mutex attribMutex;
			Threads::Mutex outputMutex;

			// Separate buffers for each thread. They get deleted and
			// erased when the end of a line is reached. So don't
			// worry about managing them and leaking memory as long as
			// you finish all your lines with std::endl properly.
			std::map<Threads::ThreadId, ostringstream*> currentLines;

		public:

			ConsoleBuf(void);
            ~ConsoleBuf(void);

			// This is what the ostream is going to send stuff to.
			int overflow(int c);

			// Set VT100 attributes for this buffer. (The stuff that
			// goes between the "<ESC>[" and the "m".) Each line is
			// cleared with a "<ESC>[0m" if this is set.
			void setAttribs(const std::string &attribs);

			// Set prefix to appear before text from this buffer (ie:
			// "ERROR: ").
			void setPrefix(const std::string &prefix);

            // Set whether or not this shows up on a graphical console
            // (adds to a separate output list).
            void setGraphical(bool graphical);

			// Set whether this should actually dump its contents to
			// the main console, or just eat it silently.
			void setEnabled(bool enabled);

			// Set whether this should output in the calling thread or
			// not. Enable to disable queueing up messages.
			void setSynced(bool synced);

			// TODO: Add a destructor to clean up currentLines?
			//   Shouldn't be necessary...
        };

        // Get the console singleton.
        Console *getMainConsole(void) {

            static Console mainConsole;

            if(mainConsoleDead) {
                ExPop::fancyAssertFail("Getting main console after death! This is really bad!");
            }

            return &mainConsole;
        }

        // Main function for the thread that sends text to stdout.
        void printConsoleBuffers(void *junk) {

            // FIXME: Most of this should probably be a function inside of
            //   Console.

            Console *mainConsole = getMainConsole();

            if(mainConsole) {

                while(!mainConsole->shutdown) {

                    // Sleep for a while so we don't monopolize the
                    // CPU.  TODO: Add some Windows-friendly
                    // code. (Sleep())
                    usleep(10000);

                    // Local copy of the output data.
                    vector<string> outputChunk;

                    // Lock the output data just long enough to copy
                    // over the output data and clear it.
                    mainConsole->combinedOutputMutex.lock();
                    outputChunk = mainConsole->combinedOutputBuffers;
                    mainConsole->combinedOutputBuffers.clear();
                    mainConsole->combinedOutputMutex.unlock();

                    mainConsole->coutMutex.lock();

                    // Spit out all the text we've accumulated.
                    for(unsigned int i = 0; i < outputChunk.size(); i++) {

                        cout << outputChunk[i] << endl;

#ifdef OS_ANDROID
                        LOGINFO("%s", outputChunk[i].c_str());
#endif
                    }

                    mainConsole->coutMutex.unlock();

                }

                // Flush the last bit of whatever might still be left
                // after the shutdown flag was set.
                mainConsole->combinedOutputMutex.lock();
                for(unsigned int i = 0; i < mainConsole->combinedOutputBuffers.size(); i++) {

                    cout << mainConsole->combinedOutputBuffers[i] << endl;

#ifdef OS_ANDROID
                    LOGINFO("%s", mainConsole->combinedOutputBuffers[i].c_str());
#endif

                }
                mainConsole->combinedOutputMutex.unlock();
            }
        }

        // ----------------------------------------------------------------
        //   Console
        // ----------------------------------------------------------------

        // Get a specific output stream by name.
        ostream &Console::getOutputStream(const std::string &name) {
            ostream *ret = NULL;

            outputStreamsMutex.lock();

            if(outputStreams.count(name)) {

                // It exists. We'll return that.
                ret = outputStreams[name];

            } else {

                // It doesn't exist. Create it and the buffer to go with it.
                ConsoleBuf *newBuf = new ConsoleBuf();
                ret = new ostream(newBuf);
                outputBufs[name] = newBuf;
                outputStreams[name] = ret;

            }

            outputStreamsMutex.unlock();

            return *ret;
        }

        ConsoleBuf *Console::getOutputBuf(const std::string &name) {

            // Just to make sure the buffer and stream exist...
            getOutputStream(name);

            // Just fetch it from the map and return.
            outputStreamsMutex.lock();
            ConsoleBuf *buf = outputBufs[name];
            outputStreamsMutex.unlock();

            return buf;
        }

        Console::~Console(void) {

            shutdown = true;
            outputThread.join();
            mainConsoleDead = true;

            // Clean up ConsoleBufs.
            for(std::map<std::string, ConsoleBuf*>::iterator bufIterator = outputBufs.begin(); bufIterator != outputBufs.end(); bufIterator++) {
                delete (*bufIterator).second;
            }

            // Clean up streams.
            for(std::map<std::string, ostream*>::iterator streamIterator = outputStreams.begin(); streamIterator != outputStreams.end(); streamIterator++) {
                delete (*streamIterator).second;
            }

        }

        Console::Console(void) {
            // Create looping output thread.
            shutdown = false;
            outputThread = Threads::Thread(printConsoleBuffers, NULL);
        }

        void Console::addOutputLine(const std::string &line, bool doStdout, bool doGraphical) {

            // Add a line to the ouput buffer.
            combinedOutputMutex.lock();

            if(doStdout) {
                combinedOutputBuffers.push_back(line);
            }
            if(doGraphical) {
                combinedGraphicalOutputBuffers.push_back(line);
            }

            combinedOutputMutex.unlock();
        }

        void Console::getNewGraphicalLines(std::vector<std::string> &lines) {

            combinedOutputMutex.lock();

            lines.insert(
                lines.end(),
                combinedGraphicalOutputBuffers.begin(),
                combinedGraphicalOutputBuffers.end());

            combinedGraphicalOutputBuffers.clear();

            combinedOutputMutex.unlock();
        }

        // ----------------------------------------------------------------
        //   ConsoleBuf
        // ----------------------------------------------------------------

        ConsoleBuf::ConsoleBuf(void) {
            enabled = true;
            synced = false;
            graphical = false;
        }

        ConsoleBuf::~ConsoleBuf(void) {

            // Dump out any remaining data.
            for(std::map<Threads::ThreadId, ostringstream*>::iterator i = currentLines.begin();
                i != currentLines.end();
                i++) {

                cout << (*i).second->str() << endl;

                delete (*i).second;
            }
        }

        int ConsoleBuf::overflow(int c) {

            // This will probably lead to lots of locking/unlocking very fast.
            // FIXME: Come up with a better solution.
            outputMutex.lock();

            // Get the specific buffer for this thread.
            ostringstream *curLinePtr = currentLines[Threads::getMyId()];
            if(!curLinePtr) {
                // It doesn't exist, so make it.
                curLinePtr = new ostringstream;
                currentLines[Threads::getMyId()] = curLinePtr;
            }

            ostringstream &currentLine = *(curLinePtr);

            // If we've reached an end of the line, we need to flush it out to
            // mainConsole's output buffer.
            if(c == '\n' || c == '\r') {

                // Get the VT100 attributes.
                string attribs;
                string prefix;
                bool enabled;

                attribMutex.lock();
                attribs = this->attribs;
                prefix  = this->prefix;
                enabled = this->enabled;
                attribMutex.unlock();

                string startAttribs;
                string endAttribs;

                // Don't bother adding anything (even the "<ESC>[0m")
                // if there's nothing in attribs.
                if(attribs.size()) {

                    // This seems like a really silly way of getting
                    // the 0x1B character into a std::string, but
                    // whatever.
                    char escChar[2];
                    escChar[0] = 0x1B;
                    escChar[1] = 0;

                    startAttribs = string(escChar) + "[" + attribs + "m";
                    endAttribs = string(escChar) + "[0m";
                }

                std::string outputLine;

#ifdef WIN32

				// FIXME: VT100 attribs don't work in Windows. :[

				// TODO: We can probably parse the attribs out here
				//   and figure out the right console API functions to
				//   get the same effects.

				// outputLine = currentLine.str();

                outputLine = startAttribs + currentLine.str() + endAttribs;

#else

				outputLine = startAttribs + currentLine.str() + endAttribs;

#endif

                if(prefix.size()) {

                    // Add ConsoleBuf-specific prefix.
                    outputLine = prefix + outputLine;
                }

                if(enabled) {

                    if(!synced) {

                        // Queue up the output for the output thread.
                        getMainConsole()->addOutputLine(outputLine, true, graphical);

                    } else {

                        // Output directly in this thread (slower, blocks).
                        getMainConsole()->coutMutex.lock();

                        cout << outputLine << endl;

#ifdef OS_ANDROID
                        LOGINFO("%s", outputLine.c_str());
#endif

                        getMainConsole()->coutMutex.unlock();

                        getMainConsole()->addOutputLine(outputLine, false, graphical);
                    }
                }

                // Get rid of the buffer for this thread and clean up the map
                // reference to it.
                delete curLinePtr;
                currentLines.erase(Threads::getMyId());

            } else {

                // Just add the character to the line buffer.
                currentLine << (char)c;
            }

            outputMutex.unlock();

            return 0;
        }

        void ConsoleBuf::setAttribs(const std::string &attribs) {
            attribMutex.lock();
            this->attribs = attribs;
            attribMutex.unlock();
        }

        void ConsoleBuf::setPrefix(const std::string &prefix) {
            attribMutex.lock();
            this->prefix = prefix;
            attribMutex.unlock();
        }

        void ConsoleBuf::setEnabled(bool enabled) {
            attribMutex.lock();
            this->enabled = enabled;
            attribMutex.unlock();
        }

        void ConsoleBuf::setSynced(bool synced) {
            attribMutex.lock();
            this->synced = synced;
            attribMutex.unlock();
        }

        void ConsoleBuf::setGraphical(bool graphical) {
            attribMutex.lock();
            this->graphical = graphical;
            attribMutex.unlock();
        }

        // ----------------------------------------------------------------
        //   Public interface
        // ----------------------------------------------------------------

        // Note on accessing mainConsole: Things could get really
        // weird here because the order that static variables get
        // cleaned up is hard to determine. As long as the main thread
        // is the last thread to finish, this should be fine,
        // though. (I think.)

        std::ostream &out(const string &outStreamName) {

            // So we can still squeeze out messages after destructors have
            // fired in whatever order.
            if(mainConsoleDead) return cout;

            // This is just a wrapper for mainConsole's getOutputStream that we
            // can use almost as conveniently as std::cout.
            return getMainConsole()->getOutputStream(outStreamName);
        }

        std::ostream &out(int streamNum) {

            // As above, for after destructor stuff.
            if(mainConsoleDead) return cout;

            // This is just a wrapper for the string version.
            ostringstream outStreamName;
            outStreamName << streamNum;
            return getMainConsole()->getOutputStream(outStreamName.str());
        }

        void outSetAttribs(const std::string &outStreamName, const std::string &attribs) {

            if(mainConsoleDead) return;

            ConsoleBuf *buf = getMainConsole()->getOutputBuf(outStreamName);
            buf->setAttribs(attribs);
        }

        void outSetPrefix(const std::string &outStreamName, const std::string &prefix) {

            if(mainConsoleDead) return;

            ConsoleBuf *buf = getMainConsole()->getOutputBuf(outStreamName);
            buf->setPrefix(prefix);
        }

        void outSetEnabled(const std::string &outStreamName, bool enabled) {

            if(mainConsoleDead) return;

            ConsoleBuf *buf = getMainConsole()->getOutputBuf(outStreamName);
            buf->setEnabled(enabled);
        }

        void outSetSynced(const std::string &outStreamName, bool synced) {

            if(mainConsoleDead) return;

            ConsoleBuf *buf = getMainConsole()->getOutputBuf(outStreamName);
            buf->setSynced(synced);
        }

        void outSetGraphical(const std::string &outStreamName, bool graphical) {

            if(mainConsoleDead) return;

            ConsoleBuf *buf = getMainConsole()->getOutputBuf(outStreamName);
            buf->setGraphical(graphical);
        }

        void outGetGraphicalLines(std::vector<std::string> &lines) {

            if(mainConsoleDead) return;

            getMainConsole()->getNewGraphicalLines(lines);
        }

    }

}

