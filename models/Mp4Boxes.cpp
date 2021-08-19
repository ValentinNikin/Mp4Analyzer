
#include "Mp4Boxes.hpp"

std::string Mp4Boxes::BoxHeader::toString() const noexcept {
    return type + " ->\r\n\r\tsize: " + std::to_string(size) + "\r\n";
}

std::string Mp4Boxes::BoxInfo::toString() const {
    return 
        header.toString() + 
        "\r\tversion: "  + std::to_string(version) + 
        "\r\n\r\tflags: " + std::to_string(flags) + "\r\n";
}

std::string Mp4Boxes::TfhdBox::toString() const {
    return info.toString() +
        "\r\ttrack id: " + std::to_string(trackId) +
        "\r\n\r\tbase data offset: " + std::to_string(baseDataOffset) +
        "\r\n\r\tsample description index: " + std::to_string(sampleDescriptionIndex) +
        "\r\n\r\tdefault sample duration: " + std::to_string(defaultSampleDuration) +
        "\r\n\r\tdefault sample size: " + std::to_string(defaultSampleSize) +
        "\r\n\r\tdefault sample flags: " + std::to_string(defaultSampleFlags) + "\r\n";
}

std::string Mp4Boxes::TfdtBox::toString() const {
    return info.toString() +
        "\r\tbase media decode time: " + std::to_string(baseMediaDecodeTime) + "\r\n";
}

std::string Mp4Boxes::TrunBox::TrunSample::toString() const {
    return "\r\tsample duration: " + std::to_string(sampleDuration) + 
            "\r\n\r\tsample size: " + std::to_string(sampleSize) + 
            "\r\n\r\tsample flags: " + std::to_string(sampleFlags) + 
            "\r\n\r\tsample composition time offset: " + std::to_string(sampleCompositionTimeOffset) + 
            "\r\n\r\tsample composition time offset signed: " + std::to_string(sampleCompositionTimeOffsetSigned) + "\r\n";
}

std::string Mp4Boxes::TrunBox::toString() const {
    auto msg = info.toString() + 
        "\r\tsample count: " + std::to_string(sampleCount) +
        "\r\n\r\tdata offset: " + std::to_string(dataOffset) + 
        "\r\n\r\tfirst sample flags: " + std::to_string(firstSampleFlags) + 
        "\r\nSamples:\r\n"
        "\r\n";

    for (int i = 0; i < samples.size(); i++) {
        msg += "--- " + std::to_string(i) + " ---\r\n" + samples[i].toString() + "\r\n";
    }

    return msg;
}