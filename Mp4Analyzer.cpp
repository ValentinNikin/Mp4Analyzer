
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

    std::string generateOffset(size_t depth) {
        std::string result;
        for (size_t i = 0; i < depth; i++) {
            result += "--- ";
        }
        return result;
    }

    auto boxHeaderReader = [](std::fstream& file, size_t startPos) {
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
    };

    std::function<void(std::fstream&, size_t, size_t, Mp4Boxes::BoxHeader)> selectAction(const std::string& type);

    auto recursiveReader = [](
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        size_t offset = startPos;
        if (header.isInit()) {
            std::cout << header.toString() << std::endl;
        }
        while (file.good() && offset < endPos) {

            auto boxHeader = boxHeaderReader(file, offset);

            auto action = selectAction(boxHeader.type);
            if (action) {
                action(file, offset + 8, offset + boxHeader.size, boxHeader);
            } else {
                std::cout << boxHeader.toString() << " --- action not found" << std::endl;
            }

            offset += boxHeader.size;

            file.seekg(offset, std::ios_base::beg);
        }
    };

    auto tfhdReader = [](
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        
        Mp4Boxes::BoxInfo boxInfo;
        boxInfo.header = header;

        BLOCK<1> block1Byte;
        file.read((char*)&block1Byte.data[0], 1);
        boxInfo.version = bytesToInt<unsigned int>(&block1Byte.data[0], 1);

        BLOCK<3> block3Byte;
        file.read((char*)&block3Byte.data[0], 3);
        boxInfo.flags = bytesToInt<unsigned int>(&block3Byte.data[0], 3);

        auto baseDataOffsetPresent = boxInfo.flags & 0x00000001;
        auto sampleDescriptionIndexPresent = boxInfo.flags & 0x00000002;
        auto defaultSampleDurationPresent = boxInfo.flags & 0x00000008;
        auto defaultSampleSizePresent = boxInfo.flags & 0x00000010;
        auto defaultSampleFlagsPresent = boxInfo.flags & 0x00000020;
        auto durationIsEmpty = boxInfo.flags & 0x00010000;
        auto defaultBaseIsMoof = boxInfo.flags & 0x00020000;

        Mp4Boxes::TfhdBox tfhdBox;
        tfhdBox.info = boxInfo;

        BLOCK<4> block4Byte;
        BLOCK<8> block8Byte;

        file.read((char*)&block4Byte.data[0], 4);
        tfhdBox.trackId = bytesToInt<unsigned int>(&block4Byte.data[0]);

        if (baseDataOffsetPresent) {
            file.read((char*)&block8Byte.data[0], 8);
            tfhdBox.baseDataOffset = bytesToInt<unsigned long int>(&block8Byte.data[0]);
        }

        if (sampleDescriptionIndexPresent) {
            file.read((char*)&block4Byte.data[0], 4);
            tfhdBox.sampleDescriptionIndex = bytesToInt<unsigned int>(&block4Byte.data[0]);
        }

        if (defaultSampleSizePresent) {
            file.read((char*)&block4Byte.data[0], 4);
            tfhdBox.defaultSampleSize = bytesToInt<unsigned int>(&block4Byte.data[0]);
        }

        if (defaultSampleFlagsPresent) {
            file.read((char*)&block4Byte.data[0], 4);
            tfhdBox.defaultSampleFlags = bytesToInt<unsigned int>(&block4Byte.data[0]);
        }

        tfhdBox.durationIsEmpty = durationIsEmpty;
        tfhdBox.defaultBaseIsMoof = defaultBaseIsMoof;

        std::cout << tfhdBox.toString() << std::endl;
    };

    auto tfdtReader = [](
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        
        Mp4Boxes::BoxInfo boxInfo;
        boxInfo.header = header;

        BLOCK<1> block1Byte;
        file.read((char*)&block1Byte.data[0], 1);
        boxInfo.version = bytesToInt<unsigned int>(&block1Byte.data[0], 1);

        BLOCK<3> block3Byte;
        file.read((char*)&block3Byte.data[0], 3);
        boxInfo.flags = bytesToInt<unsigned int>(&block3Byte.data[0], 3);

        Mp4Boxes::TfdtBox tftdBox;
        tftdBox.info = boxInfo;

        if (boxInfo.version == 1) {
            BLOCK<8> block8Byte;
            file.read((char*)&block8Byte.data[0], 8);
            tftdBox.baseMediaDecodeTime = bytesToInt<unsigned long int>(&block8Byte.data[0]);
        } else {
            BLOCK<4> block8Byte;
            file.read((char*)&block8Byte.data[0], 4);
            tftdBox.baseMediaDecodeTime = bytesToInt<unsigned long int>(&block8Byte.data[0]);
        }

        std::cout << tftdBox.toString() << std::endl;
    };

    auto trunReader = [](
                std::fstream& file, 
                size_t startPos, 
                size_t endPos, 
                Mp4Boxes::BoxHeader header) {
        
        Mp4Boxes::BoxInfo boxInfo;
        boxInfo.header = header;

        BLOCK<1> block1Byte;
        file.read((char*)&block1Byte.data[0], 1);
        boxInfo.version = bytesToInt<unsigned int>(&block1Byte.data[0], 1);

        BLOCK<3> block3Byte;
        file.read((char*)&block3Byte.data[0], 3);
        boxInfo.flags = bytesToInt<unsigned int>(&block3Byte.data[0], 3);

        auto dataOffsetPresent = boxInfo.flags & 0x00000001;
        auto firstSampleFlagsPresent = boxInfo.flags & 0x00000004;
        auto sampleDurationPresent = boxInfo.flags & 0x00000100;
        auto sampleSizePresent = boxInfo.flags & 0x00000200;
        auto sampleFlagsPresent = boxInfo.flags & 0x00000400;
        auto sampleCompositionTimeOffsetPresent = boxInfo.flags & 0x00000800;

        Mp4Boxes::TrunBox trunBox;
        trunBox.info = boxInfo;

        BLOCK<4> block4Byte;
        file.read((char*)&block4Byte.data[0], 4);
        trunBox.sampleCount = bytesToInt<unsigned int>(&block4Byte.data[0]);

        if (dataOffsetPresent) {
            file.read((char*)&block4Byte.data[0], 4);
            trunBox.dataOffset = bytesToInt<int>(&block4Byte.data[0]);
        }

        if (firstSampleFlagsPresent) {
            file.read((char*)&block4Byte.data[0], 4);
            trunBox.firstSampleFlags = bytesToInt<unsigned int>(&block4Byte.data[0]);
        }

        for (unsigned int i = 0; i < trunBox.sampleCount; i++) {
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

            if (boxInfo.version == 0) {
                file.read((char*)&block4Byte.data[0], 4);
                sample.sampleCompositionTimeOffset = bytesToInt<unsigned int>(&block4Byte.data[0]);
            }
            else {
                file.read((char*)&block4Byte.data[0], 4);
                sample.sampleCompositionTimeOffsetSigned = bytesToInt<int>(&block4Byte.data[0]);
            }

            trunBox.samples.push_back(sample);
        }

        std::cout << trunBox.toString() << std::endl;
    };

    std::function<void(std::fstream&, size_t, size_t, Mp4Boxes::BoxHeader)> selectAction(const std::string& type) {
        if (type == "moov") {
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

    recursiveReader(_file, 0, _length, {});
}
