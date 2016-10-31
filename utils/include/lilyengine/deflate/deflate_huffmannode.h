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

// This is a type internal to the DEFLATE implementation and not
// meant to be used externally.

// This is the Huffman table binary tree used in the DEFLATE
// implementation.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "deflate_common.h"

#include "deflate_bitstream.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

// ----------------------------------------------------------------------
// Internal declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Deflate
    {
        // HuffmanNode in this context is a part of a Huffman table.
        // One of these will also represent the root of the table.
        struct HuffmanNode
        {
            HuffmanNode();
            ~HuffmanNode();

            // ----------------------------------------------------------------------
            // Data decoding

            // Get the next code from the stream.
            uint32_t readCodeFromBitStream(BitStream &vec);

            // ----------------------------------------------------------------------
            // Tree building

            // Get the use score for this node. Higher numbers
            // indicate more often use. This is for generating an
            // optimal table.
            int getScore() const;

            // Get the height of the tree under this.
            int getHeight() const;

            // Modify the input codeLengths array to contain the code
            // lengths for each element in the tree, indexed by the
            // value they represent.
            void setCodelengths(
                std::vector<uint32_t> &codeLengths, uint32_t depth) const;

            // Set a new value in the Huffman tree, creating internal
            // nodes as needed.
            void setValueWithCode(uint32_t value, const std::string &code);

            // Check that all possible paths in the tree lead to a
            // leaf node eventually.
            bool checkCompleteTree();

          #if EXPOP_DEFLATE_ENABLE_COMPRESSOR

            // Take this node and all child nodes of it, and produce a
            // new tree where they all have equal relative value. The
            // new tree is returned. This is to reduce the height of
            // the tree in cases where end up with some codes longer
            // than the maximum value allowed by DEFLATE.
            HuffmanNode *flatten() const;

            // Modify the tree after a given depth by replacing
            // everything beyond that depth with the results of
            // flatten().
            void flattenAfterDepth(size_t depth, size_t currentDepth = 0);

          #endif

            // Get the leaf node count, even on an incomplete tree.
            // Sometimes we end up with an incomplete tree because
            // there was only a single code. See razor/lower.md3 in
            // Quake 3's pak0.pk3 for an example of this.
            size_t getNonNullLeafCount();

            // For the single-code case. We don't care where the one
            // leaf node is. Just find it and return it.
            HuffmanNode *findFirstLeaf();

            // Public debugging stuff.
            void dumpTree(size_t indent = 0) const;

            // This stuff is used all throughout the rest of the DEFLATE code
            // directly, but this class shouldn't be a public-facing part of
            // the API, so I don't feel too bad about having these public.
            HuffmanNode *children[2];
            uint32_t value;
            bool isLeaf;
            int score;

        private:

            // Mostly for debugging.
            std::string findCode(uint32_t c);
            std::string getValue() const;
            void dump() const;

            friend struct HuffmanNodeCmp;
        };

        // Comparison object for HuffmanNodes with std::sort.
        struct HuffmanNodeCmp
        {
            bool operator()(const HuffmanNode *a, const HuffmanNode *b);
        };

    }
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Deflate
    {

      #if EXPOP_DEFLATE_ENABLE_COMPRESSOR
        // This is part of the DEFLATE code.
        HuffmanNode *buildHuffmanNodeTreeFromCodes(
            const std::vector<uint32_t> &codes,
            bool rebuildFromCodeLengths = true);
      #endif

        // ----------------------------------------------------------------------
        // HuffmanNode class function definitions.

        inline HuffmanNode::HuffmanNode()
        {
            children[0] = nullptr;
            children[1] = nullptr;
            value = 0;
            score = 0;
            isLeaf = true;
        }

        inline HuffmanNode::~HuffmanNode()
        {
            if(!isLeaf) {
                delete children[0];
                delete children[1];
                children[0] = nullptr;
                children[1] = nullptr;
            }
        }

        inline int HuffmanNode::getScore() const
        {
            if(isLeaf) {
                return score;
            }

            return children[0]->getScore() + children[1]->getScore();
        }

        // Debugging-only function.
        inline char makeDrawable(uint32_t c)
        {
            if(c >= 0x20 && c <= 0x7e) {
                return c;
            }
            return '_';
        }

        // Debugging-only function.
        inline std::string HuffmanNode::getValue() const
        {
            std::string ret;
            if(isLeaf) {
                ret.append(1, makeDrawable(value));
            } else {
                ret = children[0]->getValue() + children[1]->getValue();
            }
            return ret;
        }

        // Debugging-only function.
        inline void HuffmanNode::dump() const
        {
            if(isLeaf) {
                std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << uint32_t(value) << " '" << makeDrawable(value) << "': " << getScore() << std::endl;
            } else {
                std::cout << "'" << getValue() << "': " << getScore() << std::endl;
            }
        }

        // Debugging-only function.
        inline void HuffmanNode::dumpTree(size_t indent) const
        {
            for(size_t i = 0; i < indent; i++) {
                std::cout << "  ";
            }

            // std::cout << indent << " ";

            if(isLeaf) {

                std::cout
                    << "0x"
                    << std::hex << std::setw(2) << std::setfill('0')
                    << uint32_t(value)
                    << " '" << makeDrawable(value) << "': " << getScore()
                    << std::endl;

            } else {

                std::cout << "Node" << std::endl;

                // std::cout << "'" << // getValue() <<
                //     "': " << getScore() << std::endl;

                if(children[0]) {
                    children[0]->dumpTree(indent + 1);
                } else {
                    for(size_t i = 0; i < indent+1; i++) {
                        std::cout << "  ";
                    }
                    std::cout << " " << "<null>" << std::endl;
                }

                if(children[1]) {
                    children[1]->dumpTree(indent + 1);
                } else {
                    for(size_t i = 0; i < indent+1; i++) {
                        std::cout << "  ";
                    }
                    std::cout << " " << "<null>" << std::endl;
                }
            }
        }

        inline int HuffmanNode::getHeight() const
        {
            if(isLeaf) return 1;

            int h0 = children[0]->getHeight();
            int h1 = children[1]->getHeight();

            if(h0 > h1) return h0 + 1;
            return h1 + 1;
        }

        inline std::string HuffmanNode::findCode(uint32_t c)
        {
            if(!isLeaf) {

                if(children[0]->isLeaf) {
                    if(children[0]->value == c) {
                        return "0";
                    }
                } else {
                    std::string code = children[0]->findCode(c);
                    if(code.size()) {
                        return "0" + code;
                    }
                }

                if(children[1]->isLeaf) {
                    if(children[1]->value == c) {
                        return "1";
                    }
                } else {
                    std::string code = children[1]->findCode(c);
                    if(code.size()) {
                        return "1" + code;
                    }
                }
            }

            return "";
        }

        inline void HuffmanNode::setCodelengths(
            std::vector<uint32_t> &codeLengths, uint32_t depth) const
        {
            if(isLeaf) {
                if(value >= codeLengths.size()) {
                    codeLengths.resize(value + 1);
                }
                codeLengths[value] = depth;
            } else {
                children[0]->setCodelengths(codeLengths, depth + 1);
                children[1]->setCodelengths(codeLengths, depth + 1);
            }
        }

        inline void HuffmanNode::setValueWithCode(uint32_t value, const std::string &code)
        {
            EXPOP_DEFLATE_ASSERT(!isLeaf);

            bool path     = (code[0] == '1');
            bool makeLeaf = (code.size() == 1);

            if(makeLeaf) {

                HuffmanNode *newHuffmanNode = new HuffmanNode;
                newHuffmanNode->value = value;
                newHuffmanNode->isLeaf = true;

                EXPOP_DEFLATE_ASSERT(!children[path]);

                children[path] = newHuffmanNode;

            } else {

                if(!children[path]) {
                    children[path] = new HuffmanNode;
                    children[path]->isLeaf = false;
                }

                children[path]->setValueWithCode(value, code.substr(1));
            }
        }

        inline bool HuffmanNode::checkCompleteTree()
        {
            if(isLeaf) {

                return true;

            } else {

                bool ret = true;
                for(size_t i = 0; i < 2; i++) {
                    if(children[i]) {
                        ret = ret && children[i]->checkCompleteTree();
                    } else {
                        // Found a non-leaf node that lacks at least one
                        // child. Not a complete tree.
                        return false;
                    }
                }

                return ret;
            }
        }

      #if EXPOP_DEFLATE_ENABLE_COMPRESSOR

        inline HuffmanNode *HuffmanNode::flatten() const
        {
            if(isLeaf) {
                HuffmanNode *newHuffmanNode = new HuffmanNode;
                newHuffmanNode->value = value;
                newHuffmanNode->isLeaf = true;
                return newHuffmanNode;
            }

            std::vector<uint32_t> codes;

            std::vector<const HuffmanNode*> nodeStack;
            nodeStack.push_back(this);

            while(nodeStack.size()) {

                // Pop node.
                const HuffmanNode *lastHuffmanNode = nodeStack[nodeStack.size() - 1];
                nodeStack.erase(nodeStack.end() - 1);

                if(lastHuffmanNode->isLeaf) {
                    codes.push_back(lastHuffmanNode->value);
                } else {
                    nodeStack.push_back(lastHuffmanNode->children[0]);
                    nodeStack.push_back(lastHuffmanNode->children[1]);
                }
            }

            return ExPop::Deflate::buildHuffmanNodeTreeFromCodes(codes, false);
        }

        inline void HuffmanNode::flattenAfterDepth(size_t depth, size_t currentDepth)
        {
            if(isLeaf) {
                return;
            }

            if(currentDepth < depth) {
                children[0]->flattenAfterDepth(depth, currentDepth + 1);
                children[1]->flattenAfterDepth(depth, currentDepth + 1);
            } else {
                for(size_t i = 0; i < 2; i++) {
                    HuffmanNode *newChild = children[i]->flatten();
                    delete children[i];
                    children[i] = newChild;
                }
            }
        }

      #endif

        inline uint32_t HuffmanNode::readCodeFromBitStream(BitStream &vec)
        {
            if(isLeaf) {
                return value;
            }

            if(vec.empty()) {
                return ~(uint32_t)0;
            }

            uint32_t bit = vec.eatIntegral_littleEndian<uint32_t>(1);

            if(bit) {
                return children[1]->readCodeFromBitStream(vec);
            } else {
                return children[0]->readCodeFromBitStream(vec);
            }
        }

        inline size_t HuffmanNode::getNonNullLeafCount()
        {
            if(isLeaf) {
                return 1;
            }

            size_t ret = 0;

            if(children[0]) {
                ret += children[0]->getNonNullLeafCount();
            }

            if(children[1]) {
                ret += children[1]->getNonNullLeafCount();
            }

            return ret;
        }

        inline HuffmanNode *HuffmanNode::findFirstLeaf()
        {
            if(isLeaf) {
                return this;
            }

            HuffmanNode *ret = nullptr;
            if(children[0]) {
                ret = children[0]->findFirstLeaf();
            }

            if(children[1] && !ret) {
                ret = children[1]->findFirstLeaf();
            }

            return ret;
        }

        // ----------------------------------------------------------------------
        // HuffmanNodeCmp function definitions.

        inline bool HuffmanNodeCmp::operator()(const HuffmanNode *a, const HuffmanNode *b)
        {
            int ascore = a->getScore();
            int bscore = b->getScore();

            if(ascore == bscore) {
                return a->value < b->value;
            }
            return ascore > bscore;
        }

    }
}


