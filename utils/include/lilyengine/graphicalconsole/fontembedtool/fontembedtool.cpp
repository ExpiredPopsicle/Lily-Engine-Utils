#include "../../utils.h"

#include <iostream>

extern "C" {
  #include "zlib.h"
}


inline std::string deflateString(const std::string &input, int level = 9)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    deflateInit(&strm, level);

    strm.avail_in = input.size();
    strm.next_in = (unsigned char*)&input[0];

    std::string outBuf;
    size_t outBufSize = input.size() * 2 + 512;
    outBuf.resize(outBufSize);

    strm.next_out = (unsigned char*)&outBuf[0];

    strm.avail_out = outBufSize;

    int r = Z_OK;
    while(r == Z_OK) {
        r = deflate(&strm, Z_FINISH);
        assert(r == Z_OK || r == Z_STREAM_END);
    }

    deflateEnd(&strm);

    outBuf.resize(outBuf.size() - strm.avail_out);

    return outBuf;
}

inline std::string inflateString(const std::string &input, int level = 9)
{
    z_stream strm = {0};
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    inflateInit(&strm);

    strm.avail_in = input.size();
    strm.next_in = (unsigned char*)&input[0];

    std::string outBuf;
    size_t outBufSize = input.size() * 16 + 65536;
    outBuf.resize(outBufSize);

    strm.next_out = (unsigned char*)&outBuf[0];

    strm.avail_out = outBufSize;

    int r = Z_OK;
    while(r == Z_OK) {
        r = inflate(&strm, Z_FINISH);
        assert(r == Z_OK || r == Z_STREAM_END);
        // TODO: Handle Z_BUF_ERROR if there's not enough space.
    }

    assert(r == Z_STREAM_END);

    inflateEnd(&strm);

    outBuf.resize(outBuf.size() - strm.avail_out);

    return outBuf;
}


std::string makeIndent(size_t i)
{
    std::string ret;
    ret.resize(i);
    for(size_t k = 0; k < i; k++) {
        ret[k] = ' ';
    }
    return ret;
}

int main(int argc, char *argv[])
{
    std::string imgStr = deflateString(ExPop::FileSystem::loadFileString("bitmapfont2.tga"));
    std::string templateStr = ExPop::FileSystem::loadFileString("graphicalconsole_fontimage_template.h");

    std::ofstream outFile("../graphicalconsole_fontimage.h");

    std::vector<std::string> templateLines;
    ExPop::stringTokenize(templateStr, "\n", templateLines, true);

    std::string declPlaceholder = "// DECLARATIONS";
    std::string implPlaceholder = "// IMPLEMENTATION";

    for(size_t i = 0; i < templateLines.size(); i++) {

        if(ExPop::stringEndsWith(declPlaceholder, templateLines[i])) {

            size_t indent = templateLines[i].size() - declPlaceholder.size();

            outFile << makeIndent(indent) << "uint8_t *graphicalConsoleGetFontFileBuffer(size_t *outLength);" << std::endl;

        } else if(ExPop::stringEndsWith(implPlaceholder, templateLines[i])) {

            size_t indent = templateLines[i].size() - implPlaceholder.size();

            outFile << makeIndent(indent) << "inline uint8_t *graphicalConsoleGetFontFileBuffer(size_t *outLength)" << std::endl;
            outFile << makeIndent(indent) << "{" << std::endl;

            outFile << makeIndent(indent + 4) << "static uint8_t data[] = {" << std::endl;

            for(size_t i = 0; i < imgStr.size(); i += 16) {
                outFile << makeIndent(indent + 7);
                for(size_t k = i; k < imgStr.size() && k < i + 16; k++) {
                    outFile << " " << "0x"
                            << std::hex
                            << std::setw(2)
                            << std::setfill('0')
                            << ((uint32_t)(uint8_t)imgStr[k])
                            << ",";
                }
                outFile << std::endl;
            }

            outFile << makeIndent(indent + 4) << "};" << std::endl;
            outFile << makeIndent(indent + 4) << "if(outLength) *outLength = " <<
                std::dec << std::setw(0) << imgStr.size() << ";" << std::endl;
            outFile << makeIndent(indent + 4) << "return data;" << std::endl;
            outFile << makeIndent(indent) << "}" << std::endl;

        } else {

            outFile << templateLines[i] << std::endl;

        }
    }

    return 0;
}


