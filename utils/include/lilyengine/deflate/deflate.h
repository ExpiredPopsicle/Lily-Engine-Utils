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

// DEFLATE decompression-only implementation. Can decompress raw
// DEFLATE streams, and with zlib headers and checksums.

// There are a couple of incomplete compression-related functions in
// here too. Compression might be implemented some day.

// See deflate_streambuf.h for a stream-based version of this.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "deflate_huffmannode.h"
#include "deflate_bitstream.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <sstream>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Deflate
    {
        // This is the public interface. Most other things in here
        // shouldn't be used directly unless you know what you're
        // doing.

        /// Decompress a DEFLATEd chunk of data (buffer), and append
        /// it to outputBuf. This could end up reading from outputBuf
        /// depending on how far back the L777 distance markers
        /// indicate. Just be aware that your new decompressed data
        /// could depend on data already in outputBuf.
        ///
        /// Use start to indicate a byte offset inside the buffer. Use
        /// endPtr to save the offset of the byte after the end of the
        /// DEFLATE stream.
        bool decompress(
            const std::string &buffer,
            std::string &outputBuf,
            size_t start = 0,
            size_t *endPtr = nullptr);
    }
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Deflate
    {
        // ----------------------------------------------------------------------
        // Internal types.

        // This represents something decoded from the Huffman tables,
        // but not yet passed through the L777 decompression.
        // Alternatively, it's something that's been compressed with
        // L777 compression and needs to have Huffman codes built for
        // it. Depends on if you're compressing or decompressing.
        struct IntermediateCode
        {
            enum {
                LENGTHDISTANCE,
                LITERAL,
                ENDOFDATA
            } type;

            union
            {
                struct
                {
                    uint32_t literalValue;
                };
                struct
                {
                    uint32_t length;
                    uint32_t distance;
                };
            };
        };

        // Current state of the decompressor. These were all local
        // variables until the thing was reworked to read a byte at a
        // time.
        struct DecompressState
        {
            // Bits in the address of a previous byte. 16 bits = 65536
            // bytes.
            static const size_t previousByteBits = 16;

            // 65536 bytes.
            static const size_t previousByteBufferSize = 1 << previousByteBits;

            // Bitmask for the 65536 bytes.
            static const size_t previousByteBitMask = previousByteBufferSize - 1;

            // Previous bytes in a ring buffer, which can be referred
            // to by the L777 codes during decompression.
            uint8_t previousBytes[previousByteBufferSize];
            size_t previousBytesIndex;

            // The length/literal Huffman table and distance table for
            // the current block of Huffman-compressed data.
            // (blockType = 1 or 2)
            HuffmanNode *lengthAndLiteralHuffmanTable;
            HuffmanNode *distanceHuffmanTable;

            // Remanining bytes to be copied in an L777 copy
            // operation. (blockType = 1 or 2, inside an L777 copy)
            size_t L777BytesRemaining;
            size_t L777Distance;

            // Remaining bytes in the current block of uncompressed
            // data. (blockType = 0)
            size_t remainingRawData;

            // The input stream.
            BitStream bitStream;

            // True if this is the final block, and it should just
            // start spitting out EOFs once this is done.
            bool isFinalBlock;

            // The current block type. 0 = uncompressed, 1 Huffman
            // table compressed with fixed tables, 2 = Huffman table
            // compressed with dynamic tables, ~(uint32_t)0 = between
            // blocks.
            uint32_t blockType;

            DecompressState(std::istream &sourceStream);
            ~DecompressState();

            // Add a byte to the previous bytes ring buffer, so that
            // the L777 system can read it for duplication later.
            void addByte(uint8_t byte);

            // Get a previously stored byte at some (negative) offset.
            uint8_t getPreviousByte(size_t reverseIndex);

            // Get a previously stored byte and add it back to the
            // previous bytes buffer at the end.
            uint8_t getL777Byte();
        };

        // ----------------------------------------------------------------------
        // Internal function prototypes for anything we'll use before
        // the definition.

        template<typename T>
        T reverseByteOrder(const T &val);

        // ----------------------------------------------------------------------
        // Deflate internal functions.

        // Convert a uint32_t to a string '1' and '0' representing it
        // as binary.
        inline std::string numberToBinary(
            uint32_t number, size_t bits,
            bool okayOverflow = false)
        {
            std::string ret;

            while(number) {
                ret = ((number & 1) ? "1" : "0") + ret;
                number >>= 1;
            }

            // Fill in the rest with zeros.
            while(ret.size() < bits) {
                ret = "0" + ret;
            }

            if(!okayOverflow) {
                if(ret.size() != bits) {
                    // Error. Resulting number has too many digits.
                    return "";
                }
            }

            return ret;
        }

        // Convert astring '1' and '0' representing a binary number to
        // a uint32_t of that number.
        inline uint32_t binaryToNumber(const std::string &binary)
        {
            uint32_t ret = 0;
            std::string tmp = binary;
            while(tmp.size()) {
                ret <<= 1;
                ret += (tmp[0] == '1') ? 1 : 0;
                tmp = tmp.substr(1, tmp.size() - 1);
            }
            return ret;
        }

        // Increment a binary number string, keeping the same number
        // of digits (and leading zeros).
        inline std::string getNextCode(
            const std::string &binaryNumber, bool okayOverflow = false)
        {
            std::string tmp;
            tmp = numberToBinary(binaryToNumber(binaryNumber) + 1, binaryNumber.size(), okayOverflow);
            return tmp;
        }

        // Build a Huffman table from code lengths, as described in
        // RFC1951, with all of DEFLATE's extra requirements for
        // deterministic table building. Returns nullptr on failure.
        inline HuffmanNode *buildHuffmanNodeTreeFromCodelengths(
            const std::vector<uint32_t> &codeLengths)
        {
            // Most of the code here is adapted from the code shown in
            // RFC1951. Some variable names match the variable names
            // used there.

            // Fill in bl_count (RFC1951 variable).
            std::vector<uint32_t> bl_count;
            for(size_t i = 0; i < codeLengths.size(); i++) {
                if(bl_count.size() <= codeLengths[i]) {
                    bl_count.resize(codeLengths[i] + 1);
                }
                if(codeLengths[i]) {
                    bl_count[codeLengths[i]]++;
                }
            }

            // Fill in next_code (RFC1951 variable).
            uint32_t code = 0;
            std::vector<std::string> next_code;
            next_code.resize(bl_count.size());
            for(uint32_t bits = 1; bits < bl_count.size(); bits++) {

                code = (code + bl_count[bits - 1]) << 1;
                next_code[bits] = numberToBinary(code, bits);

                if(next_code[bits].size() != bits) {
                    // Error: Too many codes for this code length.
                    return nullptr;
                }
            }

            // TODO: Possbily remove this testing stuff.
            // completedRanges just keeps track of which code lengths
            // we've expressed every possible value for, in case we
            // encounter them again.
            std::vector<bool> completedRanges;
            for(size_t i = 0; i < codeLengths.size(); i++) {
                if(codeLengths[i] >= completedRanges.size()) {
                    completedRanges.resize(codeLengths[i] + 1);
                }
            }

            // FIXME: Using std::string to store the codes at this
            // point seems odd.
            std::vector<std::string> codesByChar;
            for(uint32_t n = 0; n < codeLengths.size(); n++) {

                uint32_t len = codeLengths[n];

                if(len != 0) {

                    if(completedRanges[len]) {
                        // Too many codes for this length.
                        return nullptr;
                    }

                    if(codesByChar.size() <= n) {
                        codesByChar.resize(n + 1);
                    }

                    codesByChar[n] = next_code[len];

                    if(codesByChar[n].size() != codeLengths[n]) {
                        // FIXME: Can we even hit this condition
                        // anymore?
                        return nullptr;
                    }

                    std::string nextCode = getNextCode(next_code[len], true);

                    if(nextCode.size() > next_code[len].size()) {
                        completedRanges[len] = true;
                    }

                    next_code[len] = nextCode;
                }
            }

            // Build the final tree.
            HuffmanNode *finalTree = new HuffmanNode;
            finalTree->isLeaf = false;
            for(uint32_t n = 0; n < codesByChar.size(); n++) {
                if(codesByChar[n].size()) {
                    finalTree->setValueWithCode(n, codesByChar[n]);
                }
            }

            // Handle the case of a single distance code. Note that
            // the only known test case for this is pak0.pk3 from
            // Quake 3, specifically the razor/lower.md3 file.
            if(finalTree->getNonNullLeafCount() == 1) {

                HuffmanNode *newChild1 = new HuffmanNode;
                *newChild1 = *finalTree->findFirstLeaf();

                HuffmanNode *newChild2 = new HuffmanNode;
                *newChild2 = *finalTree->findFirstLeaf();

                HuffmanNode *newRoot = new HuffmanNode;
                newRoot->isLeaf = false;
                newRoot->children[0] = newChild1;
                newRoot->children[1] = newChild2;

                delete finalTree;
                finalTree = newRoot;

                // Final check of the modified tree.
                if(!finalTree->checkCompleteTree()) {
                    delete finalTree;
                    return nullptr;
                }
            }

            // We need to check for a complete tree before we can
            // check for a single, isolated node.
            if(!finalTree->checkCompleteTree()) {
                // Error: Bad final tree in buildHuffmanNodeTreeFromCodelengths.
                delete finalTree;
                return nullptr;
            }

            return finalTree;
        }

        // Build the fixed Huffman table for literals and length
        // values specified in RFC1951.
        inline HuffmanNode *createFixedHuffmanTree()
        {
            // This should now be all we need to generate the fixed Huffman
            // code trees.
            std::vector<uint32_t> codeLengths2;
            codeLengths2.resize(288);
            for(size_t i = 0; i <= 287; i++) {
                if(i <= 143) {
                    codeLengths2[i] = 8;
                } else if(i <= 255) {
                    codeLengths2[i] = 9;
                } else if(i <= 279) {
                    codeLengths2[i] = 7;
                } else if(i <= 287) {
                    codeLengths2[i] = 8;
                }
            }

            HuffmanNode *finalTree = buildHuffmanNodeTreeFromCodelengths(codeLengths2);

            // If this fails, we messed up something horribly.
            EXPOP_DEFLATE_ASSERT(finalTree);

            return finalTree;
        }

        // Build the fixed Huffman table for distance values specified
        // in RFC1951.
        inline HuffmanNode *createFixedHuffmanTreeForDistances()
        {
            // Distance table for fixed Huffman trees version.
            std::vector<uint32_t> distanceTableCodeLengths;
            for(size_t i = 0; i < 32; i++) {
                distanceTableCodeLengths.push_back(5);
            }

            HuffmanNode *distanceHuffmanTable =
                buildHuffmanNodeTreeFromCodelengths(distanceTableCodeLengths);

            EXPOP_DEFLATE_ASSERT(distanceHuffmanTable);

            return distanceHuffmanTable;
        }

        // Read Huffman tables from a stream for the dynamic table mode.
        inline bool readDynamicHuffmanTrees(
            BitStream &deflateSpecificBuf,
            HuffmanNode *&finalTree,
            HuffmanNode *&distanceHuffmanTable)
        {
            // Most of this function is covered by RFC 1951, Page 12. Section
            // 3.2.7. See check that for details.

            // Huffman tables for the compressed data we want are compressed,
            // themselves, with Huffman tables. So first we must read in and
            // decode the Huffman tables to decompress the Huffman tables.

            // HLIT: Number of literal/length codes minus 257.
            uint32_t hlit  = deflateSpecificBuf.eatIntegral_bigEndian<uint32_t>(5);

            // HDIST: Number of distance codes minus 1.
            uint32_t hdist = deflateSpecificBuf.eatIntegral_bigEndian<uint32_t>(5);

            // HCLEN: Number of code length codes minus 4. We'll use this for
            // decompressing the literal/length and distance codes.
            uint32_t hclen = deflateSpecificBuf.eatIntegral_bigEndian<uint32_t>(4);

            // Spec says no more than 19 codes here!
            if((hclen + 4) >= 20) {
                // Error: Too many codes in readDynamicHuffmanTrees.
                return false;
            }

            // The code lengths for the first table (which we'll use to
            // compress the other tables) are stored out of order, presumably
            // with the most commonly used values first. This table maps the
            // order in the stream to the order we'll actually use them in.
            // This way it can have just a subset of them, and leave the rest
            // as zeros. See RFC 1951, Page 13.
            uint32_t indexMapping[] = {
                16, 17, 18, 0,
                8,  7,  9,  6,
                10, 5,  11, 4,
                12, 3,  13, 2,
                14, 1,  15
            };

            // Initialize a table of zeros.
            std::vector<uint32_t> codeLengthsForAlphabetDecoded;
            codeLengthsForAlphabetDecoded.resize(19);

            // Read in all the lengths we have.
            uint32_t counter = 0;
            for(size_t i = 0; i < (hclen + 4); i++) {
                codeLengthsForAlphabetDecoded[indexMapping[counter]] =
                    deflateSpecificBuf.eatIntegral_bigEndian<uint32_t>(3);
                counter++;
            }

            HuffmanNode *finalAlphabetCode =
                buildHuffmanNodeTreeFromCodelengths(
                    codeLengthsForAlphabetDecoded);

            // Handle alphabet code decoding failure.
            if(!finalAlphabetCode) {
                // Error: Bad alphabet code in readDynamicHuffmanTrees.
                return false;
            }

            // This is where we'll store the code lengths for the final
            // Huffman table that we'll use for decompressing the actual data.
            std::vector<uint32_t> codeLengthsForLiteralsAndLengths;

            // Literal length codes and distances are both in the same block
            // of data here. It's one after another, but one can overflow into
            // the other. For example, bytes could be repeated from the
            // literal/length block into the distance block. So we'll read it
            // all as one big block and then split it up afterwards.
            uint32_t i = 0;
            while(i < hlit + 257 + hdist + 1) {

                uint32_t value = finalAlphabetCode->readCodeFromBitStream(deflateSpecificBuf);
                uint32_t repeatLength = 0;

                // readCodeFromBitStream's error value is ~0. Bail out if we
                // get it. It'll mean we ran past the end of the stream
                // prematurely.
                if(value == ~(uint32_t)0) {
                    // Error: Bad code.
                    delete finalAlphabetCode;
                    return false;
                }

                switch(value) {

                    case 16: { // Copy previous.

                        // 16 is the "copy previous" special marker, but if we
                        // haven't read anything to copy yet, then it's an
                        // error. There's no way that should exist like that
                        // in normal data and there's nothing reasonable we
                        // can do with it.
                        if(!i) {
                            delete finalAlphabetCode;
                            return false;
                        }

                        // Read the previous value.
                        uint32_t valueToRepeat = codeLengthsForLiteralsAndLengths[i - 1];

                        // Read the number of times we're going to repeat it.
                        repeatLength = deflateSpecificBuf.eatIntegral_bigEndian<uint32_t>(2) + 3;

                        // Copy it and move the write pointer that many times.
                        for(size_t k = 0; k < repeatLength; k++) {
                            codeLengthsForLiteralsAndLengths.push_back(valueToRepeat);
                            i++;
                        }

                    } break;

                    case 17: // Copy 0.

                        // 17 means to copy zero some number of times. It's
                        // the low-range mode, which takes up less space but
                        // only can represent 3 bits worth of range.

                        // Determine actual length to write (3 bits + constant
                        // 3). (For magic numbers, see RFC 1951, section
                        // 3.2.7)
                        repeatLength = deflateSpecificBuf.eatIntegral_bigEndian<uint32_t>(3) + 3;

                        // Write it.
                        while(repeatLength) {
                            codeLengthsForLiteralsAndLengths.push_back(0);
                            i++;
                            repeatLength--;
                        }

                        break;

                    case 18:

                        // 18 also means to copy zero some number of times.
                        // It's the high-range mode, which takes up more space
                        // but can represent 7 bits worth of range.

                        // Determine actual length to write (7 bits + constant
                        // 11). (For magic numbers, see RFC 1951, section
                        // 3.2.7)
                        repeatLength = deflateSpecificBuf.eatIntegral_bigEndian<uint32_t>(7) + 11;

                        // Write it.
                        while(repeatLength) {
                            codeLengthsForLiteralsAndLengths.push_back(0);
                            i++;
                            repeatLength--;
                        }

                        break;

                    default: {

                        // Default behavior. Just write the value directly.
                        // Remember that this is just a code length, not an
                        // actual code.
                        codeLengthsForLiteralsAndLengths.push_back(value);
                        i++;

                    } break;
                }

            }

            delete finalAlphabetCode;
            finalAlphabetCode = nullptr;

            // If we didn't end up with exactly as many entries in the
            // combined literal/length and distance block as we think we
            // should have, then that's an error.
            if(codeLengthsForLiteralsAndLengths.size() != hlit + 257 + hdist + 1) {
                // Error: Incomplete code lengths.
                return false;
            }

            // Move the distance code lengths into their own buffer, and
            // remove those values from the literal/length buffer.
            std::vector<uint32_t> codeLengthsForDistanceAlphabet;
            codeLengthsForDistanceAlphabet.insert(
                codeLengthsForDistanceAlphabet.begin(),
                codeLengthsForLiteralsAndLengths.begin() + hlit + 257,
                codeLengthsForLiteralsAndLengths.end());
            codeLengthsForLiteralsAndLengths.erase(
                codeLengthsForLiteralsAndLengths.begin() + hlit + 257,
                codeLengthsForLiteralsAndLengths.end());

            // Another sanity check.
            if(codeLengthsForLiteralsAndLengths.size() != hlit + 257) {
                // Error: Incorrect code length count for distance
                // alphabet.
                return false;
            }

            // End marker needs to show up in there somewhere. If the code
            // length for the code that represents 256 (end of data) isn't
            // present, then we can never decode a 256, and therefor never end
            // the stream.
            if(!codeLengthsForLiteralsAndLengths[256]) {
                // Error: No end-of-data code is possible. Would read
                // data forever.
                return false;
            }

            // Finally build the trees.
            finalTree = buildHuffmanNodeTreeFromCodelengths(codeLengthsForLiteralsAndLengths);
            if(!finalTree || !finalTree->checkCompleteTree()) {
                // Error: Incomplete Huffman table.
                delete finalTree;
                finalTree = nullptr;
                return false;
            }

            distanceHuffmanTable = buildHuffmanNodeTreeFromCodelengths(codeLengthsForDistanceAlphabet);
            if(!distanceHuffmanTable || !distanceHuffmanTable->checkCompleteTree()) {
                // Error: Bad dynamic trees after decoding.
                delete finalTree;
                finalTree = nullptr;
                delete distanceHuffmanTable;
                distanceHuffmanTable = nullptr;
                return false;
            }

            return true;
        }

      #if EXPOP_DEFLATE_ENABLE_COMPRESSOR

        // Attempt to build an optimal node tree to compress a given
        // vector of values. FIXME: This was part of a partial
        // compressor implementation and needs to be updated to use
        // the IntermediateCode type instead of just uint32_t.
        inline HuffmanNode *buildHuffmanNodeTreeFromCodes(
            const std::vector<uint32_t> &codes,
            bool rebuildFromCodeLengths)
        {
            std::vector<HuffmanNode*> nodes;

            for(size_t i = 0; i < codes.size(); i++) {
                if(codes[i] >= nodes.size()) {
                    nodes.resize(codes[i] + 1);
                }
                if(!nodes[codes[i]]) {
                    nodes[codes[i]] = new HuffmanNode;
                    nodes[codes[i]]->value = codes[i];
                }
                nodes[codes[i]]->score++;
            }

            // Remove unused stuff.
            for(size_t i = 0; i < nodes.size(); i++) {
                if(nodes[i] && nodes[i]->score == 0) {
                    delete nodes[i];
                    nodes[i] = nullptr;
                }
                if(!nodes[i]) {
                    nodes.erase(nodes.begin() + i);
                    i--;
                }
            }

            HuffmanNodeCmp cmpOb;
            std::sort(nodes.begin(), nodes.end(), cmpOb);

            // Reduce the list of nodes to a tree.
            while(nodes.size() > 1) {

                HuffmanNode *n0 = nodes[nodes.size() - 2];
                HuffmanNode *n1 = nodes[nodes.size() - 1];
                nodes.erase(nodes.end() - 2, nodes.end());

                HuffmanNode *newHuffmanNode = new HuffmanNode;
                newHuffmanNode->isLeaf = false;

                bool n0Less = n0->getHeight() < n1->getHeight();
                newHuffmanNode->children[!n0Less] = n0;
                newHuffmanNode->children[n0Less] = n1;

                nodes.push_back(newHuffmanNode);

                std::sort(nodes.begin(), nodes.end(), cmpOb);
            }

            if(nodes[0]->getHeight() > 15) {
                // TODO: Deal with this correctly.
                std::cerr << "Resulting Huffman tree is too big. Can't compress this." << std::endl;
                EXPOP_DEFLATE_ASSERT(0);
                return nullptr;
            }

            EXPOP_DEFLATE_ASSERT(nodes.size() == 1);

            nodes[0]->checkCompleteTree();

            std::vector<uint32_t> codeLengths;
            for(size_t i = 0; i < codes.size(); i++) {
                nodes[0]->setCodelengths(codeLengths, 0);
            }

            if(rebuildFromCodeLengths) {
                delete nodes[0];
                return buildHuffmanNodeTreeFromCodelengths(codeLengths);
            }

            return nodes[0];
        }

      #endif

        // Read a code from a DEFLATE stream.
        inline bool readIntermediateCode(
            HuffmanNode *literalLengthTable,
            HuffmanNode *distanceTable,
            BitStream &data,
            IntermediateCode &output)
        {
            if(data.empty()) {
                // Error: No data to decompress.
                return false;
            }

            if(!literalLengthTable || !distanceTable) {
                // Error: Bad decompressor state.
                return false;
            }

            uint32_t value = literalLengthTable->readCodeFromBitStream(data);

            // Decoding error.
            if(value == ~(uint32_t)0) {
                // Error: Decoding error.
                return false;
            }

            // Literal value.
            if(value < 256) {
                output.type = IntermediateCode::LITERAL;
                output.literalValue = value;
                return true;
            }

            // End of data.
            if(value == 256) {
                output.type = IntermediateCode::ENDOFDATA;
                return true;
            }

            // Length value.
            if(value > 256) {

                // Read length.

                // These are the length codes from RFC 1951, which
                // this section attempts to reproduce.
                // ----------------------------------------------------------------------
                //      Extra               Extra               Extra
                // Code Bits Length(s) Code Bits Lengths   Code Bits Length(s)
                // ---- ---- ------     ---- ---- -------   ---- ---- -------
                //  257   0     3       267   1   15,16     277   4   67-82
                //  258   0     4       268   1   17,18     278   4   83-98
                //  259   0     5       269   2   19-22     279   4   99-114
                //  260   0     6       270   2   23-26     280   4  115-130
                //  261   0     7       271   2   27-30     281   5  131-162
                //  262   0     8       272   2   31-34     282   5  163-194
                //  263   0     9       273   3   35-42     283   5  195-226
                //  264   0    10       274   3   43-50     284   5  227-257
                //  265   1  11,12      275   3   51-58     285   0    258
                //  266   1  13,14      276   3   59-66
                // ----------------------------------------------------------------------

                uint32_t actualLength = 0;

                uint32_t extraBitsValue = 0;

                size_t lengthExtraBitCount = 1 + (value - 265) / 4;

                if(value < 285 && value >= 265) {
                    extraBitsValue = data.eatIntegral_bigEndian<uint32_t>(lengthExtraBitCount);
                }

                uint32_t stage      = ((value - 265) / 4);
                uint32_t multiplier = 2 << stage;
                uint32_t startPoint = 265 + 4 * stage;
                uint32_t addVal     = 3 + (8 << stage);

                if(value < 265) {

                    // 257 - 264 are a special case that doesn't use
                    // extra bits, I guess. We could probably
                    // generalize it a bit better, but with the way I
                    // set up the numbers it has to be this way.
                    actualLength = value - 254;

                } else if(value < 285) {

                    // For all the stuff between 265 and 257
                    // (inclusive), we have some math to squish the
                    // whole table into.
                    actualLength =
                        addVal + (value - startPoint) *
                        multiplier + extraBitsValue;

                } else if(value == 285) {

                    // 258 is a special case and doesn't take any
                    // extra bits. There's no way to generalize it
                    // with the rest of the math.
                    actualLength = 258;
                }

                // Read distance.

                // These are the distance codes from RFC 1951, which
                // this section attempts to reproduce.
                // ----------------------------------------------------------------------
                //      Extra           Extra               Extra
                // Code Bits Dist  Code Bits   Dist     Code Bits Distance
                // ---- ---- ----  ---- ----  ------    ---- ---- --------
                //   0   0    1     10   4     33-48    20    9   1025-1536
                //   1   0    2     11   4     49-64    21    9   1537-2048
                //   2   0    3     12   5     65-96    22   10   2049-3072
                //   3   0    4     13   5     97-128   23   10   3073-4096
                //   4   1   5,6    14   6    129-192   24   11   4097-6144
                //   5   1   7,8    15   6    193-256   25   11   6145-8192
                //   6   2   9-12   16   7    257-384   26   12  8193-12288
                //   7   2  13-16   17   7    385-512   27   12 12289-16384
                //   8   3  17-24   18   8    513-768   28   13 16385-24576
                //   9   3  25-32   19   8   769-1024   29   13 24577-32768
                // ----------------------------------------------------------------------

                uint32_t distanceVal =
                distanceTable->readCodeFromBitStream(data);

                uint32_t actualDistance = ~(uint32_t)0;

                if(distanceVal < 4) {

                    // distanceVal is literal distance (0-3).
                    actualDistance = distanceVal + 1;

                } else {

                    // distanceVal has some number of extra bits.

                    size_t extraBitCount = 1 + (distanceVal - 4) / 2;
                    size_t base          = 1 + (2 << extraBitCount);
                    size_t rangeStart    = (distanceVal / 2) * 2;
                    size_t baseAdd       = (1 << extraBitCount) * (distanceVal - rangeStart);

                    extraBitsValue = data.eatIntegral_bigEndian<uint32_t>(extraBitCount);

                    actualDistance =
                        base + baseAdd +
                        extraBitsValue;
                }

                if(actualDistance == ~(uint32_t)0) {
                    return false;
                }

                // Put together the final output.
                output.type = IntermediateCode::LENGTHDISTANCE;
                output.length = actualLength;
                output.distance = actualDistance;

                return true;
            }

            return false;
        }

        // Handle a code. Either append a literal to a stream, or copy
        // a section of the existing stream forward.
        inline bool handleIntermediateCode(
            const IntermediateCode &code,
            uint32_t &outLiteral,
            DecompressState &state)
        {
            switch(code.type) {

                case IntermediateCode::LITERAL: {

                    if(code.literalValue > 255) {
                        // That's not a literal character.
                        return false;
                    }

                    uint8_t byte = code.literalValue;
                    state.addByte(byte);
                    outLiteral = byte;

                } break;

                case IntermediateCode::LENGTHDISTANCE: {

                    if(code.distance > state.previousBytesIndex) {
                        // Not enough bytes at the start to read.
                        return false;
                    }

                    if(code.distance < 1) {
                        // Not even a valid length.
                        return false;
                    }

                    state.L777BytesRemaining = code.length;
                    state.L777Distance = code.distance;

                    if(!state.L777BytesRemaining) {
                        // Error: Bad L777 code.
                        return false;
                    }

                    outLiteral = state.getL777Byte();

                } break;

                case IntermediateCode::ENDOFDATA: {
                    // Nothing to see here.
                } break;

            }

            return true;
        }

        /// Read an uncompressed section and append it to output.
        inline bool readRawData(
            std::string &output,
            BitStream &deflateSpecificBuf,
            DecompressState &state)
        {
            // Get us back up to a byte boundary.
            deflateSpecificBuf.dropBitsToByteBoundary();

            uint16_t len = deflateSpecificBuf.eatIntegral_bigEndian<uint16_t>(16);

            uint16_t nlen = deflateSpecificBuf.eatIntegral_bigEndian<uint16_t>(16);

            // len and nlen are bitwise complements of each other. If this
            // isn't the case, something's gone wrong. See RFC 1951, section
            // 3.2.4.
            if(len != (uint16_t)~nlen) {
                return false;
            }

            // FIXME (STREAMS): Read these bytes as requested instead
            // of all at once.
            std::string newData = deflateSpecificBuf.eatRawData(len);
            output += newData;

            // FIXME (STREAMS): This will be changed when we make the
            // one-byte-at-a-time change.
            for(size_t i = 0; i < newData.size(); i++) {
                state.addByte(newData[i]);
            }

            return true;
        }

        template<typename T>
        inline void freeAndClear(T *&ptr)
        {
            delete ptr;
            ptr = nullptr;
        }

        // ----------------------------------------------------------------------
        // DecompressState function definitions.

        inline DecompressState::DecompressState(std::istream &sourceStream) :
            bitStream(sourceStream)
        {
            previousBytesIndex = 0;
            lengthAndLiteralHuffmanTable = nullptr;
            distanceHuffmanTable = nullptr;
            remainingRawData = 0;
            isFinalBlock = false;
            L777Distance = 0;
            L777BytesRemaining = 0;

            blockType = ~(uint32_t)0;
        }

        inline DecompressState::~DecompressState()
        {
            delete lengthAndLiteralHuffmanTable;
            delete distanceHuffmanTable;
        }

        inline void DecompressState::addByte(uint8_t byte)
        {
            previousBytes[previousBytesIndex & previousByteBitMask] = byte;
            previousBytesIndex++;
        }

        inline uint8_t DecompressState::getPreviousByte(size_t reverseIndex)
        {
            // This is made safe by the bitmask.
            return previousBytes[(previousBytesIndex - reverseIndex) & previousByteBitMask];
        }

        inline uint8_t DecompressState::getL777Byte()
        {
            EXPOP_DEFLATE_ASSERT(L777BytesRemaining);
            uint8_t ret = getPreviousByte(L777Distance);
            addByte(ret);
            L777BytesRemaining--;
            return ret;
        }

        // ----------------------------------------------------------------------
        // External-facing functions.

        int32_t readNextValue(DecompressState &state)
        {
            while(1) {

                if(state.blockType == 0) {

                    // Raw data block type.

                    if(state.remainingRawData) {

                        state.remainingRawData--;

                        if(!state.remainingRawData) {
                            state.blockType = ~(uint32_t)0;
                        }

                        uint8_t byte = state.bitStream.eatIntegral_bigEndian<uint8_t>();

                        state.addByte(byte);

                        return byte;

                    } else {

                        // Error: Inconsistent state.
                        return EOF;
                    }

                } else if(state.blockType == 1 || state.blockType == 2) {

                    // Huffman-compressed block type.

                    // Length/distance state.
                    if(state.L777BytesRemaining) {

                        if(state.L777Distance >= state.previousBytesIndex) {
                            // Error: Distance refers to something before
                            // our stream.
                            return EOF;
                        }

                        return state.getL777Byte();
                    }

                    // Read the next code.
                    IntermediateCode code;
                    if(!readIntermediateCode(
                            state.lengthAndLiteralHuffmanTable,
                            state.distanceHuffmanTable,
                            state.bitStream, code))
                    {
                        // Error: Failed to read code.
                        return EOF;
                    }

                    if(code.type == IntermediateCode::ENDOFDATA) {
                        // End of block. Only actual non-error EOF here.
                        state.blockType = ~(uint32_t)0;
                        continue;
                    }

                    uint32_t literal = ~(uint32_t)0;
                    if(!handleIntermediateCode(code, literal, state)) {
                        // Error: Bad intermediate code.
                        return EOF;
                    }

                    if(literal != ~(uint32_t)0) {
                        return literal;
                    }

                    // Eh?
                    return EOF;

                } else {

                    if(state.isFinalBlock) {
                        // Legitimate EOF.
                        return EOF;
                    }

                    if(state.bitStream.empty()) {
                        // Error: Not enough bits in the stream.
                        return EOF;
                    }

                    freeAndClear(state.lengthAndLiteralHuffmanTable);
                    freeAndClear(state.distanceHuffmanTable);

                    // BFINAL is true for the last chunk in a stream.
                    state.isFinalBlock = !!state.bitStream.eatIntegral_bigEndian<uint32_t>(1);

                    // BTYPE tells us what decompression method to use.
                    state.blockType = state.bitStream.eatIntegral_bigEndian<uint32_t>(2);

                    switch(state.blockType) {

                        case 0: {

                            // 0 = Uncompressed data.

                            // ----------------------------------------------------------------------

                            // Get us back up to a byte boundary.
                            state.bitStream.dropBitsToByteBoundary();

                            uint16_t len = state.bitStream.eatIntegral_bigEndian<uint16_t>(16);

                            uint16_t nlen = state.bitStream.eatIntegral_bigEndian<uint16_t>(16);

                            // len and nlen are bitwise complements of each other. If this
                            // isn't the case, something's gone wrong. See RFC 1951, section
                            // 3.2.4.
                            if(len != (uint16_t)~nlen) {
                                // Error: Bad length.
                                return EOF;
                            }

                            state.remainingRawData = len;

                        } break;

                        case 2:
                        case 1: {

                            // 1 - Fixed table compressed data.
                            // 2 = Dynamic Huffman table compressed data.

                            if(state.blockType == 2) {

                                // Read dynamic Huffman tables.
                                bool tableReadSuccess = readDynamicHuffmanTrees(
                                    state.bitStream,
                                    state.lengthAndLiteralHuffmanTable,
                                    state.distanceHuffmanTable);

                                if(!tableReadSuccess) {
                                    // Error: Failed to read tables.
                                    freeAndClear(state.lengthAndLiteralHuffmanTable);
                                    freeAndClear(state.distanceHuffmanTable);
                                    return EOF;
                                }

                            } else {

                                // Load fixed Huffman tables.
                                state.lengthAndLiteralHuffmanTable = createFixedHuffmanTree();
                                state.distanceHuffmanTable = createFixedHuffmanTreeForDistances();
                            }

                        } break;
                    }
                }
            }

            return EOF;
        }

        inline bool decompress(
            std::istream &in,
            std::string &outputBuf)
        {
            // FIXME (STREAMS): Read bytes as needed and return
            // instead of all at once.

            DecompressState state(in);

            int32_t nextChar = EOF;
            do {
                nextChar = readNextValue(state);
                if(nextChar != EOF) {
                    outputBuf.push_back(nextChar);
                }
            } while(nextChar != EOF);

            state.bitStream.dropBitsToByteBoundary();

            return true;
        }

        inline bool decompress(
            const std::string &buffer,
            std::string &outputBuf,
            size_t start,
            size_t *endPtr)
        {
            std::istringstream istr(buffer);
            istr.ignore(start);

            bool ret = decompress(istr, outputBuf);

            if(endPtr) {
                *endPtr = istr.tellg();
            }

            return ret;
        }

    }
}

