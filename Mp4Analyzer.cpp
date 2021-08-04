
#include "Mp4Analyzer.hpp"

#include <array>
#include <iostream>

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
IntegerType bytesToInt(const uint8_t* bytes, bool littleEndian = false) {
    IntegerType result = 0;
    if (littleEndian) {
        for (int n = sizeof(result); n >= 0; n--) {
            result = (result << 8) + bytes[n];
        }
    } else {
        for (int n = 0; n < sizeof(result); n++) {
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

    std::function<void(std::fstream&, const std::string&, size_t, size_t, size_t)> selectAction(const std::string& type);

    auto recursiveReader = [](std::fstream& file, const std::string& blockName, size_t startPos, size_t endPos, size_t depth) {
        size_t offset = startPos;
        std::cout << "|" << blockName << "|" << std::endl;
        while (file.good() && offset < endPos) {

            BLOCK<4> block4Byte;
            file.read((char*)&block4Byte.data[0], 4);
            auto size = bytesToInt<unsigned int>(&block4Byte.data[0]);

            file.read((char*)&block4Byte.data[0], 4);
            auto type = block4Byte.toString();

            if (size == 1) {
                BLOCK<8> block8Byte;
                file.read((char*)&block8Byte.data[0], 8);
                size = bytesToInt<unsigned long int>(&block8Byte.data[0]);
            }

            if (type == "uuid") {
                /**
                 * @todo
                 * support block with uuid type
                 */
                throw std::runtime_error("Block with type uuid unsupported yet");
            }

            std::cout << generateOffset(depth) << type << " block -> size: " << size << " ---" << std::endl;

            auto action = selectAction(type);
            if (action) {
                action(file, type, offset + 8, offset + size, depth + 1);
            }

            offset += size;

            file.seekg(offset, std::ios_base::beg);
        }
    };

    auto moofReader = [](const std::fstream& file) {

    };

    std::function<void(std::fstream&, const std::string&, size_t, size_t, size_t)> selectAction(const std::string& type) {
        if (type == "moof") {
            return recursiveReader;
        }
        return nullptr;
    }

}


void Mp4Analyzer::parse() {
    if (!_file.is_open()) {
        throw std::runtime_error("Target file doesn't open");
    }

    recursiveReader(_file, "file", 0, _length, 1);
}
