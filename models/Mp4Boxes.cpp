
#include "Mp4Boxes.hpp"

Mp4Boxes::Box::Box(Mp4Boxes::BoxHeader bHeader)
    : size{bHeader.size},
    type{bHeader.type} {}

Mp4Boxes::Box::~Box() {
    for (size_t i = 0; i < children.size(); i++) {
        delete children[i];
    }
}

std::string Mp4Boxes::Box::toString() const noexcept {
    return type + " ->\r\n\r\tsize: " + std::to_string(size) + "\r\n";
}

Mp4Boxes::FullBox::FullBox(Mp4Boxes::BoxHeader bHeader)
    : Box{bHeader} {}

std::string Mp4Boxes::FullBox::toString() const noexcept {
    return 
        Box::toString() + 
        "\r\tversion: "  + std::to_string(version) + 
        "\r\n\r\tflags: " + std::to_string(flags) + "\r\n";
}

Mp4Boxes::TfhdBox::TfhdBox(Mp4Boxes::BoxHeader bHeader)
    : FullBox{bHeader} {}

std::string Mp4Boxes::TfhdBox::toString() const noexcept {
    return FullBox::toString() +
        "\r\ttrack id: " + std::to_string(trackId) +
        "\r\n\r\tbase data offset: " + std::to_string(baseDataOffset) +
        "\r\n\r\tsample description index: " + std::to_string(sampleDescriptionIndex) +
        "\r\n\r\tdefault sample duration: " + std::to_string(defaultSampleDuration) +
        "\r\n\r\tdefault sample size: " + std::to_string(defaultSampleSize) +
        "\r\n\r\tdefault sample flags: " + std::to_string(defaultSampleFlags) + "\r\n";
}

Mp4Boxes::TfdtBox::TfdtBox(Mp4Boxes::BoxHeader bHeader)
    : FullBox{bHeader} {}

std::string Mp4Boxes::TfdtBox::toString() const noexcept {
    return FullBox::toString() +
        "\r\tbase media decode time: " + std::to_string(baseMediaDecodeTime) + "\r\n";
}

Mp4Boxes::TrunBox::TrunBox(Mp4Boxes::BoxHeader bHeader)
    : FullBox{bHeader} {}

std::string Mp4Boxes::TrunBox::TrunSample::toString() const {
    return "\r\tsample duration: " + std::to_string(sampleDuration) + 
            "\r\n\r\tsample size: " + std::to_string(sampleSize) + 
            "\r\n\r\tsample flags: " + std::to_string(sampleFlags) + 
            "\r\n\r\tsample composition time offset: " + std::to_string(sampleCompositionTimeOffset) + 
            "\r\n\r\tsample composition time offset signed: " + std::to_string(sampleCompositionTimeOffsetSigned) + "\r\n";
}

std::string Mp4Boxes::TrunBox::toString() const noexcept {
    auto msg = FullBox::toString() + 
        "\r\tsample count: " + std::to_string(sampleCount) +
        "\r\n\r\tdata offset: " + std::to_string(dataOffset) + 
        "\r\n\r\tfirst sample flags: " + std::to_string(firstSampleFlags) + 
        "\r\nSamples:\r\n"
        "\r\n";

    // for (int i = 0; i < samples.size(); i++) {
    //     msg += "--- " + std::to_string(i) + " ---\r\n" + samples[i].toString() + "\r\n";
    // }

    return msg;
}