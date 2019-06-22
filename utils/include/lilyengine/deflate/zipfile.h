// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2018 Kiri Jolly
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

// Zip reading subsystem. Only supports the DEFLATE (default)
// compression method and raw stored files.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "../streams/streamsection.h"
#include "../streams/owningstream.h"
#include "deflate_streambuf.h"
#include "deflate.h"

#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <map>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Open Zip file representation. You can use this to open files
    /// inside Zips as (read only) streams, and extract some other
    /// stored data about them.
    class ZipFile
    {
    public:

        /// Create a Zip file reader object based on an already opened
        /// stream. Must use std::shared_ptr for lifetime management
        /// here to ensure that the stream stays around as long as the
        /// ZipFile.
        ZipFile(std::shared_ptr<std::istream> inSourceStream);

        /// Create a Zip file reader object by opening a file. The
        /// file will be kept open for the lifetime of the ZipFile
        /// object.
        ZipFile(const std::string &filename);

        /// Open a file inside a Zip file as an input stream. The
        /// actual resulting stream type from this will vary depending
        /// on the storage method of the file. Seeking will be
        /// supported in the resulting stream except for
        /// offset-from-end, but seeking backwards could be very
        /// expensive in compressed streams.
        std::shared_ptr<std::istream> openFile(const std::string &filename);

        /// Get a list of every file in the Zip.
        std::vector<std::string> getFileList() const;

        /// Add every filename in the Zip to an existing list.
        void fillFileList(std::vector<std::string> &fileList) const;

        /// Get a list of every directory inside the Zip.
        std::vector<std::string> getDirectoryList() const;

        /// Add every directory name in the Zip to an existing list.
        void fillDirectoryList(std::vector<std::string> &directoryList) const;

        /// Get the size of a file from the Zip. You probably want to
        /// use this to determine the size instead of seeking to the
        /// end and using tellg().
        size_t getFileSize(const std::string &filename) const;

        /// Get the CRC32 checksum of a file inside the zip.
        uint32_t getFileCRC(const std::string &filename) const;

    private:

        struct ZipFileEntry
        {
            size_t offsetFromStart;
            size_t length;
            size_t uncompressedLength;
            uint32_t crc32;
            uint16_t compressionMethod;
        };

        std::map<std::string, ZipFileEntry> fileEntries;
        std::map<std::string, bool> directoryEntries;

        std::shared_ptr<std::istream> sourceStream;

        void scanFileList();
    };
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#include "../filesystem.h"

namespace ExPop
{
    // ----------------------------------------------------------------------
    // Internal utility functions.

    inline bool systemIsLittleEndian()
    {
        uint32_t a = 0x00000001;
        return ((char*)&a)[0] == 1;
    }

    template<typename T>
    inline T littleEndianToNative(const T &val)
    {
        if(systemIsLittleEndian()) {
            return val;
        }
        return ExPop::Deflate::reverseByteOrder(val);
    }

    template<typename T>
    inline void readZipValue(std::istream &in, T &val)
    {
        in.read((char*)&val, sizeof(val));
        val = littleEndianToNative(val);
    }

    template <typename T>
    inline std::string toHex(const T &val)
    {
        std::string ret;
        ret.resize(sizeof(val) * 2);
        const char lut[] = "0123456789abcdef";
        for(size_t i = 0; i < sizeof(val) * 2; i++) {
            T tmpVal = (val >> (i * 4)) & 0xf;
            ret[sizeof(val) * 2 - i - 1] = lut[tmpVal];
        }
        return ret;
    }

    template<typename T>
    inline T reverseBits(const T &val)
    {
        T ret = 0;
        T tmp = val;
        for(size_t i = 0; i < sizeof(val) * 8; i++) {
            ret <<= 1;
            ret |= (tmp & 1);
            tmp >>= 1;
        }
        return ret;
    }

    inline uint32_t crc32(const std::string &buffer)
    {
        uint32_t crc = 0xffffffff;

        for(size_t i = 0; i < buffer.size(); i++) {
            uint32_t byte = reverseBits((uint32_t)buffer[i]);
            for(size_t j = 0; j < 8; j++) {
                if(int32_t(crc ^ byte) < 0) {
                    crc = (crc << 1) ^ 0x04c11db7;
                } else {
                    crc <<= 1;
                }
                byte <<= 1;
            }
        }

        return reverseBits(~crc);
    }

    // ----------------------------------------------------------------------
    // Zip internal structures

    struct ZipFileDataDescriptor
    {
        uint32_t crc32;
        uint32_t compressedSize;
        uint32_t uncompressedSize;

        void read(std::istream &in)
        {
            readZipValue(in, crc32);
            readZipValue(in, compressedSize);
            readZipValue(in, uncompressedSize);
        }

        void dump(const std::string &prefix = "")
        {
            std::cout << prefix << "ZipFileDataDescriptor" << std::endl;
            std::cout << prefix << "| crc32: " << crc32 << std::endl;
            std::cout << prefix << "| compressedSize: " << compressedSize << std::endl;
            std::cout << prefix << "\\ uncompressedSize: " << uncompressedSize << std::endl;
        }
    };

    struct ZipCommonHeader
    {
        uint16_t versionNeeded;
        uint16_t generalPurpose;
        uint16_t compressionMethod;
        uint16_t lastModFileTime;
        uint16_t lastModFileDate;
        ZipFileDataDescriptor dataDescriptor;
        uint16_t filenameLength;
        uint16_t extraFieldLength;

        void read(std::istream &in)
        {
            readZipValue(in, versionNeeded);
            readZipValue(in, generalPurpose);
            readZipValue(in, compressionMethod);
            readZipValue(in, lastModFileTime);
            readZipValue(in, lastModFileDate);
            dataDescriptor.read(in);
            readZipValue(in, filenameLength);
            readZipValue(in, extraFieldLength);
        }

        void dump(const std::string &prefix = "")
        {
            std::cout << prefix << "| versionNeeded: " << versionNeeded << std::endl;
            std::cout << prefix << "| generalPurpose: " << generalPurpose << std::endl;
            std::cout << prefix << "| compression method (should be 8 or 0): " << compressionMethod << std::endl;
            std::cout << prefix << "| lastModFileTime: " << lastModFileTime << std::endl;
            std::cout << prefix << "| lastModFileDate: " << lastModFileDate << std::endl;
            dataDescriptor.dump(prefix + "| ");
            std::cout << prefix << "| filenameLength: " << filenameLength << std::endl;
            std::cout << prefix << "| extraFieldLength: " << extraFieldLength << std::endl;
        }
    };

    struct ZipFileExtraFields
    {
        std::map<uint16_t, std::string> extraFields;

        void read(std::istream &in, size_t extraFieldSize)
        {
            size_t extraFieldDataRead = 0;
            while(extraFieldDataRead < extraFieldSize) {

                uint16_t identifier;
                uint16_t size;
                std::string extraData;

                readZipValue(in, identifier);
                readZipValue(in, size);
                extraData.resize(size);
                in.read(&extraData[0], size);

                extraFields[identifier] = extraData;

                extraFieldDataRead += 4 + size;

                if(!in.good()) return;
            }

            // // FIXME: Make this a normal error.
            // assert(extraFieldDataRead == extraFieldSize);
        }

        void dump(const std::string &prefix = "")
        {
            std::cout << prefix << "ZipFileExtraFields" << std::endl;
            for(auto i = extraFields.begin(); i != extraFields.end(); i++) {
                std::cout
                    << prefix << "| 0x"
                    << std::hex << i->first << std::dec
                    << " = " << i->second.size() << " bytes"
                    << std::endl;
            }
            std::cout << prefix << "\\ (" << extraFields.size() << " fields)" << std::endl;
        }
    };

    struct ZipLocalFileHeader
    {
        ZipCommonHeader commonData;

        std::string filename;
        ZipFileExtraFields extraFields;

        void read(std::istream &in)
        {
            commonData.read(in);

            filename.resize(commonData.filenameLength);
            in.read(&filename[0], commonData.filenameLength);

            extraFields.read(in, commonData.extraFieldLength);
        }

        void dump(const std::string &prefix = "")
        {
            std::cout << prefix << "ZipLocalFileHeader" << std::endl;
            commonData.dump(prefix);
            std::cout << prefix << "| filename: " << filename << std::endl;
            extraFields.dump(prefix + "| ");
        }
    };

    struct ZipFileCentralDirectoryEntry
    {
        uint16_t versionMadeBy;
        ZipCommonHeader commonData;
        uint16_t fileCommentLength;
        uint16_t diskNumberStart;
        uint16_t internalFileAttributes;
        uint32_t externalFileAttributes;
        uint32_t relativeOffsetOfLocalHeader;

        std::string filename;
        std::string fileComment;

        ZipFileExtraFields extraFields;

        void read(std::istream &in)
        {
            readZipValue(in, versionMadeBy);
            commonData.read(in);
            readZipValue(in, fileCommentLength);
            readZipValue(in, diskNumberStart);
            readZipValue(in, internalFileAttributes);
            readZipValue(in, externalFileAttributes);
            readZipValue(in, relativeOffsetOfLocalHeader);

            filename.resize(commonData.filenameLength);
            in.read(&filename[0], filename.size());

            extraFields.read(in, commonData.extraFieldLength);

            fileComment.resize(fileCommentLength);
            in.read(&fileComment[0], fileComment.size());
        }

        void dump(const std::string &prefix = "")
        {
            std::cout << prefix << "ZipFileCentralDirectoryEntry" << std::endl;

            std::cout << prefix << "| versionMadeBy: " << versionMadeBy << std::endl;
            commonData.dump(prefix + "| ");
            std::cout << prefix << "| fileCommentLength: " << fileCommentLength << std::endl;
            std::cout << prefix << "| diskNumberStart: " << diskNumberStart << std::endl;
            std::cout << prefix << "| internalFileAttributes: " << internalFileAttributes << std::endl;
            std::cout << prefix << "| externalFileAttributes: " << externalFileAttributes << std::endl;
            std::cout << prefix << "| relativeOffsetOfLocalHeader: " << relativeOffsetOfLocalHeader << std::endl;
            std::cout << prefix << "| filename: " << filename << std::endl;
            std::cout << prefix << "| extraFields:" << std::endl;
            extraFields.dump(prefix + "| ");
            std::cout << prefix << "\\ fileComment: " << fileComment << std::endl;
        }
    };

    struct ZipFileEndOfCentralDirectory
    {
        uint16_t numberOfThisDisk;
        uint16_t numberOfDiskWithStart;
        uint16_t totalEntriesOnThisDisk;
        uint16_t totalEntriesInDirectory;
        uint32_t sizeOfCentralDirectory;
        uint32_t offsetOfStartOfCentralDirectoryWRTStartingDisk; // What.

        uint16_t zipFileCommentLength;

        std::string zipFileComment;

        void read(std::istream &in)
        {
            readZipValue(in, numberOfThisDisk);
            readZipValue(in, numberOfDiskWithStart);
            readZipValue(in, totalEntriesOnThisDisk);
            readZipValue(in, totalEntriesInDirectory);
            readZipValue(in, sizeOfCentralDirectory);
            readZipValue(in, offsetOfStartOfCentralDirectoryWRTStartingDisk);
            readZipValue(in, zipFileCommentLength);

            zipFileComment.resize(zipFileCommentLength);
            in.read(&zipFileComment[0], zipFileComment.size());
        }

        void dump(const std::string &prefix = "")
        {
            std::cout << prefix << "TODO" << std::endl;
        }
    };

    struct Zip64ExtendedInformationField
    {
        uint64_t uncompressedSize;
        uint64_t compressedSize;
        uint64_t localHeaderRecordOffset;
        uint32_t diskStart;

        void read(std::istream &in)
        {
            readZipValue(in, uncompressedSize);
            readZipValue(in, compressedSize);
            readZipValue(in, localHeaderRecordOffset);
            readZipValue(in, diskStart);
        }

        void dump(const std::string &prefix = "")
        {
            std::cout << prefix << "TODO" << std::endl;
        }

    };

    struct Zip64EndOfCentralDirectoryRecord
    {
        uint16_t versionMadeBy;
        uint16_t versionNeeded;
        uint32_t diskNumber;
        uint32_t diskNumberWithStartOfCentralDir;
        uint64_t entriesInCentralDirOnDisk;
        uint64_t entriesInCentralDir;
        uint64_t sizeOfCentralDir;
        uint64_t offsetOfStartOfCentralDirectoryWRTStartingDisk; // What.

        // std::string extensibleDataSector;

        void read(std::istream &in)
        {
            uint64_t totalSizeOfRecord = 0;
            readZipValue(in, totalSizeOfRecord);

            readZipValue(in, versionMadeBy);
            readZipValue(in, versionNeeded);
            readZipValue(in, diskNumber);
            readZipValue(in, diskNumberWithStartOfCentralDir);
            readZipValue(in, entriesInCentralDirOnDisk);
            readZipValue(in, entriesInCentralDir);
            readZipValue(in, sizeOfCentralDir);
            readZipValue(in, offsetOfStartOfCentralDirectoryWRTStartingDisk);

            if(totalSizeOfRecord > 44) {
                uint64_t remainingSize = totalSizeOfRecord - 44;

                // We don't really have anything we want to do with
                // the extensible data sector, so let's skip it.
                in.ignore(remainingSize);

                // extensibleDataSector.resize(remainingSize);
                // in.read(&extensibleDataSector[0], extensibleDataSector.size());
            }
        }

        void dump(const std::string &prefix = "")
        {
            std::cout << prefix << "TODO" << std::endl;
        }
    };

    struct Zip64EndOfCentralDirectoryLocator
    {
        uint32_t numberOfDiskWithStartOfCentralDir;
        uint64_t relativeOffsetOfEndOfCentralDirRecord;
        uint32_t totalNumberOfDisks;

        void read(std::istream &in)
        {
            readZipValue(in, numberOfDiskWithStartOfCentralDir);
            readZipValue(in, relativeOffsetOfEndOfCentralDirRecord);
            readZipValue(in, totalNumberOfDisks);
        }

        void dump(const std::string &prefix = "")
        {
            std::cout << prefix << "TODO" << std::endl;
        }
    };


    // ----------------------------------------------------------------------
    // ZipFile function definitons.

    inline ZipFile::ZipFile(std::shared_ptr<std::istream> inSourceStream)
    {
        sourceStream = inSourceStream;
        scanFileList();
    }

    inline ZipFile::ZipFile(const std::string &filename)
    {
        std::shared_ptr<std::istream> fileStream = ExPop::FileSystem::openReadFile(filename);
        sourceStream = fileStream;
        scanFileList();
    }

    inline std::shared_ptr<std::istream> ZipFile::openFile(const std::string &filename)
    {
        auto i = fileEntries.find(filename);
        if(i == fileEntries.end()) {
            return nullptr;
        }

        sourceStream->seekg(i->second.offsetFromStart);
        std::shared_ptr<StreamSection> sectionBuf(new StreamSection(sourceStream, i->second.length));
        std::shared_ptr<OwningIStream> sectionStream(new OwningIStream(sectionBuf));

        if(i->second.compressionMethod == 0) {

            // Stored (no compression).
            return sectionStream;

        } else if(i->second.compressionMethod == 8) {

            // DEFLATE compression.
            std::shared_ptr<Deflate::DecompressorStreamBuf> dcStreamBuf(
                new Deflate::DecompressorStreamBuf(sectionStream));

            std::shared_ptr<ExPop::OwningIStream> dcStream(
                new ExPop::OwningIStream(dcStreamBuf));

            return dcStream;
        }

        // Error: Unknown compression algorithm.
        return nullptr;
    }

    inline std::vector<std::string> ZipFile::getFileList() const
    {
        std::vector<std::string> ret;
        fillFileList(ret);
        return ret;
    }

    inline void ZipFile::fillFileList(std::vector<std::string> &fileList) const
    {
        for(auto i = fileEntries.begin(); i != fileEntries.end(); i++) {
            fileList.push_back(i->first);
        }
    }

    inline std::vector<std::string> ZipFile::getDirectoryList() const
    {
        std::vector<std::string> ret;
        fillDirectoryList(ret);
        return ret;
    }

    inline void ZipFile::fillDirectoryList(std::vector<std::string> &directoryList) const
    {
        for(auto i = directoryEntries.begin(); i != directoryEntries.end(); i++) {
            directoryList.push_back(i->first);
        }
    }

    inline size_t ZipFile::getFileSize(const std::string &filename) const
    {
        auto i = fileEntries.find(filename);
        if(i == fileEntries.end()) {
            // Error: File not found.
            return 0;
        }
        return i->second.uncompressedLength;
    }

    inline uint32_t ZipFile::getFileCRC(const std::string &filename) const
    {
        auto i = fileEntries.find(filename);
        if(i == fileEntries.end()) {
            // Error: File not found.
            return 0;
        }
        return i->second.crc32;
    }

    inline void ZipFile::scanFileList()
    {
        if(!sourceStream || sourceStream->fail()) {
            return;
        }

        sourceStream->seekg(0);

        while(sourceStream->good()) {

            // Read next section signature.
            uint32_t signature;
            sourceStream->read((char*)&signature, sizeof(signature));
            signature = littleEndianToNative(signature);

            if(sourceStream->eof()) {
                break;
            }

            // Check the part that's common to every signature: "PK"
            if((signature & 0x0000ffff) == 0x4b50) {

                switch((signature & 0xffff0000) >> 16) {

                    case 0x0403: {

                        // Local file header found.
                        ZipLocalFileHeader header;
                        header.read(*sourceStream);

                        // Tentatively read the sizes and CRC. This
                        // will get replaced by the Zip64 version if
                        // the extra field for it is found.
                        uint64_t compressedSize = header.commonData.dataDescriptor.compressedSize;
                        uint64_t uncompressedSize = header.commonData.dataDescriptor.uncompressedSize;
                        uint32_t savedCrc = header.commonData.dataDescriptor.crc32;

                        // Search for an extended field for Zip64
                        // data.
                        auto it = header.extraFields.extraFields.find(0x0001);
                        if(it != header.extraFields.extraFields.end()) {

                            // Read the Zip64 data and replace the
                            // sizes we read with what we found there.
                            Zip64ExtendedInformationField zip64ext;
                            std::istringstream istr(it->second);
                            zip64ext.read(istr);

                            compressedSize = zip64ext.compressedSize;
                            uncompressedSize = zip64ext.uncompressedSize;
                        }

                        size_t fileStartOffset = sourceStream->tellg();
                        size_t fileEndOffset = fileStartOffset + compressedSize;

                        if(header.filename.size() &&
                            header.filename[header.filename.size() - 1] == '/' &&
                            fileStartOffset == fileEndOffset)
                        {

                            // FIXME: I think this is a directory
                            // name, but I can't tell for sure. It
                            // ends with a '/' and is 0-bytes. Can't
                            // find information in APPNOTE.TXT.
                            directoryEntries[header.filename.substr(0, header.filename.size() - 1)] = true;

                        } else {

                            // Definitely a normal file entry.
                            ZipFileEntry entry;
                            entry.offsetFromStart = fileStartOffset;
                            entry.length = compressedSize;
                            entry.uncompressedLength = uncompressedSize;
                            entry.crc32 = savedCrc;
                            entry.compressionMethod = header.commonData.compressionMethod;
                            fileEntries[header.filename] = entry;
                        }

                        // Skip to next header.
                        sourceStream->seekg(compressedSize, std::ios_base::cur);

                    } break;

                    case 0x0806: {

                        // Extra data record
                        uint32_t extraFieldLength;
                        readZipValue(*sourceStream, extraFieldLength);
                        sourceStream->seekg(extraFieldLength, std::ios_base::cur);

                    } break;

                    case 0x0807: {

                        // Data descriptor (easy to skip)
                        ZipFileDataDescriptor ds;
                        ds.read(*sourceStream);

                    } break;

                    case 0x0201: {

                        // Central header
                        ZipFileCentralDirectoryEntry cde;
                        cde.read(*sourceStream);

                    } break;

                    case 0x0505: {

                        // Digital signature
                        uint16_t dataSize;
                        readZipValue(*sourceStream, dataSize);
                        sourceStream->seekg(dataSize, std::ios_base::cur);

                    } break;

                    case 0x0605: {

                        // End of central directory.
                        ZipFileEndOfCentralDirectory eod;
                        eod.read(*sourceStream);

                    } break;

                    case 0x0606: {

                        // Zip64 end of central dir.
                        Zip64EndOfCentralDirectoryRecord eod;
                        eod.read(*sourceStream);

                    } break;

                    case 0x0706: {

                        // Zip64 locator somethingorother
                        Zip64EndOfCentralDirectoryLocator loc;
                        loc.read(*sourceStream);

                    } break;

                    default:

                        // Error: Unknown Zip header type.
                        return;
                }

            } else {

                // Error: Bad header.
                return;
            }
        }

        // We hit the EOF, so the EOF bit is still present. We should
        // clear that.
        sourceStream->clear(sourceStream->eofbit);
    }

}

