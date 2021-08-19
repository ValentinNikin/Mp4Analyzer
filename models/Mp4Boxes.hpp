
#pragma once

#include <string>
#include <vector>

namespace Mp4Boxes {
    struct BoxHeader {
        unsigned long int size {0};
        std::string type;

        std::string toString() const noexcept;

        bool isInit() const noexcept {
            return size != 0 && !type.empty();
        }
    };

    struct BoxInfo {
        BoxHeader header;
        unsigned int version {0};
        unsigned int flags {0};

        std::string toString() const;
    };

    struct TfhdBox {
        BoxInfo info;

        unsigned int trackId {0};
        unsigned long int baseDataOffset {0};
        unsigned int sampleDescriptionIndex {0};
        unsigned int defaultSampleDuration {0};
        unsigned int defaultSampleSize {0};
        unsigned int defaultSampleFlags {0};

        bool durationIsEmpty {false};
        bool defaultBaseIsMoof {false};

        std::string toString() const;
    };

    struct TfdtBox {
        BoxInfo info;
        unsigned long int baseMediaDecodeTime;

        std::string toString() const;
    };

    struct TrunBox {
        struct TrunSample {
            unsigned int sampleDuration {0};
            unsigned int sampleSize {0};
            unsigned int sampleFlags {0};
            unsigned int sampleCompositionTimeOffset {0};
            int sampleCompositionTimeOffsetSigned {0};

            std::string toString() const;
        };

        BoxInfo info;
        unsigned int sampleCount {0};
        int dataOffset {0};
        unsigned int firstSampleFlags {0};
        std::vector<TrunSample> samples;

        std::string toString() const;
    };
}