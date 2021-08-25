
#include "Mp4Analyzer.hpp"

#include <array>
#include <iostream>
#include <vector>

#include "models/Mp4Boxes.hpp"

Mp4Analyzer::Mp4Analyzer() {}

bool Mp4Analyzer::open(const std::string& path) {
    _file.open(path, std::ios_base::in | std::ios_base::binary);

    if (!_file.is_open()) {
        return false;
    }

    _file.seekg(0, std::ios_base::end);
    _length = _file.tellg();
    _file.seekg(0, std::ios_base::beg);

    std::cout << "File is open, length: " << _length << std::endl;

    return true;
}

namespace {

template<typename IntegerType>
IntegerType bytesToInt(const uint8_t* bytes, int countBytes = sizeof(IntegerType), bool littleEndian = false) {
    IntegerType result = 0;
    if (littleEndian) {
        for (int n = countBytes; n >= 0; n--) {
            result = (result << 8) + bytes[n];
        }
    } else {
        for (int n = 0; n < countBytes; n++) {
            result = (result << 8) + bytes[n];
        }
    }
    return result;
}

template<int SIZE>
struct BLOCK {
    std::array<uint8_t, SIZE> data;

    void print() const {
        for (int i = 0; i < SIZE; i++) {
            std::cout << data[i];
        }
    }

    std::string toString() const {
        std::string result;
        for (int i = 0; i < SIZE; i++) {
            result += data[i];
        }
        return result;
    }
};

}

namespace {

    Mp4Boxes::BoxHeader readBoxHeader(std::fstream& file, size_t startPos) {
        Mp4Boxes::BoxHeader boxHeader;
        BLOCK<4> block4Byte;

        file.read((char*)&block4Byte.data[0], 4);
        boxHeader.size = bytesToInt<unsigned int>(&block4Byte.data[0]);

        file.read((char*)&block4Byte.data[0], 4);
        boxHeader.type = block4Byte.toString();

        if (boxHeader.size == 1) {
            BLOCK<8> block8Byte;
            file.read((char*)&block8Byte.data[0], 8);
            boxHeader.size = bytesToInt<unsigned long int>(&block8Byte.data[0]);
        }

        if (boxHeader.type == "uuid") {
            /**
             * @todo
             * support block with uuid type
             */
            throw std::runtime_error("Block with type uuid unsupported yet");
        }

        return boxHeader;
    }

    void readFullBox(std::fstream& file, Mp4Boxes::FullBox* box) {
        BLOCK<1> block1Byte;
        file.read((char*)&block1Byte.data[0], 1);
        box->version = bytesToInt<unsigned int>(&block1Byte.data[0], 1);

        BLOCK<3> block3Byte;
        file.read((char*)&block3Byte.data[0], 3);
        box->flags = bytesToInt<unsigned int>(&block3Byte.data[0], 3);
    }

    std::function<Mp4Boxes::Box*(std::fstream&, size_t, size_t, Mp4Boxes::BoxHeader)> selectAction(const std::string& type);

    Mp4Boxes::Box* recursiveReader(
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        auto box = new Mp4Boxes::Box(header);

        size_t offset = startPos;
        while (file.good() && offset < endPos) {

            auto boxHeader = readBoxHeader(file, offset);

            auto action = selectAction(boxHeader.type);
            if (action) {
                box->children.emplace_back(action(file, offset + 8, offset + boxHeader.size, boxHeader));
            } else {
                box->children.emplace_back(new Mp4Boxes::Box(boxHeader));
            }

            offset += boxHeader.size;

            file.seekg(offset, std::ios_base::beg);
        }

        return box;
    };

    Mp4Boxes::Box* ftypReader(
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        auto ftypBox = new Mp4Boxes::FtypBox(header);

        size_t offset = startPos;

        BLOCK<4> block4Byte;
        file.read((char*)&block4Byte.data[0], 4);
        ftypBox->majorBrand = block4Byte.toString();

        file.read((char*)&block4Byte.data[0], 4);
        ftypBox->minorVersion = bytesToInt<unsigned int>(&block4Byte.data[0]);

        offset += 8;

        while (offset < endPos) {
            file.read((char*)&block4Byte.data[0], 4);

            if (file.gcount() == 4) {
                ftypBox->compatibleBrands.push_back(block4Byte.toString());
            }

            offset += file.gcount();
        }
        
        auto ftyp = ftypBox->toString();

        std::cout << ftyp << std::endl;

        return ftypBox;
    }

    Mp4Boxes::Box* mfhdReader(
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        
        auto mfhdBox = new Mp4Boxes::MfhdBox(header);

        readFullBox(file, mfhdBox);

        BLOCK<4> block4Byte;
        file.read((char*)&block4Byte.data[0], 4);
        mfhdBox->sequenceNumber = bytesToInt<unsigned int>(&block4Byte.data[0]);

        std::cout << mfhdBox->toString() << std::endl;

        return mfhdBox;
    };

    Mp4Boxes::Box* tfhdReader(
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        
        auto tfhdBox = new Mp4Boxes::TfhdBox(header);

        readFullBox(file, tfhdBox);

        auto baseDataOffsetPresent = tfhdBox->flags & 0x00000001;
        auto sampleDescriptionIndexPresent = tfhdBox->flags & 0x00000002;
        auto defaultSampleDurationPresent = tfhdBox->flags & 0x00000008;
        auto defaultSampleSizePresent = tfhdBox->flags & 0x00000010;
        auto defaultSampleFlagsPresent = tfhdBox->flags & 0x00000020;
        auto durationIsEmpty = tfhdBox->flags & 0x00010000;
        auto defaultBaseIsMoof = tfhdBox->flags & 0x00020000;

        BLOCK<4> block4Byte;
        BLOCK<8> block8Byte;

        file.read((char*)&block4Byte.data[0], 4);
        tfhdBox->trackId = bytesToInt<unsigned int>(&block4Byte.data[0]);

        if (baseDataOffsetPresent) {
            file.read((char*)&block8Byte.data[0], 8);
            tfhdBox->baseDataOffset = bytesToInt<unsigned long int>(&block8Byte.data[0]);
        }

        if (sampleDescriptionIndexPresent) {
            file.read((char*)&block4Byte.data[0], 4);
            tfhdBox->sampleDescriptionIndex = bytesToInt<unsigned int>(&block4Byte.data[0]);
        }

        if (defaultSampleSizePresent) {
            file.read((char*)&block4Byte.data[0], 4);
            tfhdBox->defaultSampleSize = bytesToInt<unsigned int>(&block4Byte.data[0]);
        }

        if (defaultSampleFlagsPresent) {
            file.read((char*)&block4Byte.data[0], 4);
            tfhdBox->defaultSampleFlags = bytesToInt<unsigned int>(&block4Byte.data[0]);
        }

        tfhdBox->durationIsEmpty = durationIsEmpty;
        tfhdBox->defaultBaseIsMoof = defaultBaseIsMoof;

        std::cout << tfhdBox->toString() << std::endl;

        return tfhdBox;
    };

    Mp4Boxes::Box* tfdtReader(
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        
        auto tfdtBox = new Mp4Boxes::TfdtBox(header);

        readFullBox(file, tfdtBox);

        if (tfdtBox->version == 1) {
            BLOCK<8> block8Byte;
            file.read((char*)&block8Byte.data[0], 8);
            tfdtBox->baseMediaDecodeTime = bytesToInt<unsigned long>(&block8Byte.data[0]);
        } else {
            BLOCK<4> block4Byte;
            file.read((char*)&block4Byte.data[0], 4);
            tfdtBox->baseMediaDecodeTime = bytesToInt<unsigned int>(&block4Byte.data[0]);
        }

        std::cout << tfdtBox->toString() << std::endl;

        return tfdtBox;
    };

    Mp4Boxes::Box* trunReader(
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        
        auto trunBox = new Mp4Boxes::TrunBox(header);

        readFullBox(file, trunBox);

        auto dataOffsetPresent = trunBox->flags & 0x00000001;
        auto firstSampleFlagsPresent = trunBox->flags & 0x00000004;
        auto sampleDurationPresent = trunBox->flags & 0x00000100;
        auto sampleSizePresent = trunBox->flags & 0x00000200;
        auto sampleFlagsPresent = trunBox->flags & 0x00000400;
        auto sampleCompositionTimeOffsetPresent = trunBox->flags & 0x00000800;

        BLOCK<4> block4Byte;
        file.read((char*)&block4Byte.data[0], 4);
        trunBox->sampleCount = bytesToInt<unsigned int>(&block4Byte.data[0]);

        if (dataOffsetPresent) {
            file.read((char*)&block4Byte.data[0], 4);
            trunBox->dataOffset = bytesToInt<int>(&block4Byte.data[0]);
        }

        if (firstSampleFlagsPresent) {
            file.read((char*)&block4Byte.data[0], 4);
            trunBox->firstSampleFlags = bytesToInt<unsigned int>(&block4Byte.data[0]);
        }

        for (unsigned int i = 0; i < trunBox->sampleCount; i++) {
            Mp4Boxes::TrunBox::TrunSample sample;
            if (sampleDurationPresent) {
                file.read((char*)&block4Byte.data[0], 4);
                sample.sampleDuration = bytesToInt<unsigned int>(&block4Byte.data[0]);
            }

            if (sampleSizePresent) {
                file.read((char*)&block4Byte.data[0], 4);
                sample.sampleSize = bytesToInt<unsigned int>(&block4Byte.data[0]);
            }

            if (sampleFlagsPresent) {
                file.read((char*)&block4Byte.data[0], 4);
                sample.sampleFlags = bytesToInt<unsigned int>(&block4Byte.data[0]);
            }

            if (trunBox->version == 0) {
                file.read((char*)&block4Byte.data[0], 4);
                sample.sampleCompositionTimeOffset = bytesToInt<unsigned int>(&block4Byte.data[0]);
            }
            else {
                file.read((char*)&block4Byte.data[0], 4);
                sample.sampleCompositionTimeOffsetSigned = bytesToInt<int>(&block4Byte.data[0]);
            }

            trunBox->samples.push_back(sample);
        }

        std::cout << trunBox->toString() << std::endl;

        return trunBox;
    };

    std::function<Mp4Boxes::Box*(std::fstream&, size_t, size_t, Mp4Boxes::BoxHeader)> selectAction(const std::string& type) {
        if (type == "ftyp") {
            return ftypReader;
        } else if (type == "moov") {
            return recursiveReader;
        } else if (type == "trak") {
            return recursiveReader;
        } else if (type == "edts") {
            return recursiveReader;
        } else if (type == "mdia") {
            return recursiveReader;
        } else if (type == "minf") {
            return recursiveReader;
        } else if (type == "dinf") {
            return recursiveReader;
        } else if (type == "stbl") {
            return recursiveReader;
        } else if (type == "mvex") {
            return recursiveReader;
        } else if (type == "moof") {
            return recursiveReader;
        } else if (type == "mfhd") {
            return mfhdReader;
        } else if (type == "traf") {
            return recursiveReader;
        } else if (type == "tfhd") {
            return tfhdReader;
        } else if (type == "tfdt") {
            return tfdtReader;
        } else if (type == "trun") {
            return trunReader;
        } else if (type == "mfra") {
            return recursiveReader;
        } else if (type == "skip") {
            return recursiveReader;
        } else if (type == "udta") {
            return recursiveReader;
        } else if (type == "strk") {
            return recursiveReader;
        } else if (type == "meta") {
            return recursiveReader;
        } else if (type == "dinf") {
            return recursiveReader;
        } else if (type == "ipro") {
            return recursiveReader;
        } else if (type == "sinf") {
            return recursiveReader;
        } else if (type == "fiin") {
            return recursiveReader;
        } else if (type == "paen") {
            return recursiveReader;
        } else if (type == "meco") {
            return recursiveReader;
        } else if (type == "mere") {
            return recursiveReader;
        }
        return nullptr;
    }

}


void Mp4Analyzer::parse() {
    if (!_file.is_open()) {
        throw std::runtime_error("Target file doesn't open");
    }

    auto rootBox = recursiveReader(_file, 0, _length, { _length, "root" });

    /*auto trunBox = (Mp4Boxes::TrunBox*)rootBox->children[0]->children[1]->children[2];
    auto dataOffset = trunBox->dataOffset;*/
}
