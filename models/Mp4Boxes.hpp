
#pragma once

#include <string>
#include <vector>

namespace Mp4Boxes {

    struct BoxHeader {
        unsigned long int size {0};
        std::string type;
    };
    
    struct Box {
        explicit Box(BoxHeader bHeader);
        ~Box();

        unsigned long int size {0};
        std::string type;

        std::vector<Box*> children;

        virtual std::string toString() const noexcept;
    };

    struct FullBox : Box {
        explicit FullBox(BoxHeader bHeader);

        unsigned int version {0};
        unsigned int flags {0};

        std::string toString() const noexcept override;
    };

    struct MfhdBox : FullBox {
        explicit MfhdBox(BoxHeader bHeader);

        unsigned int sequenceNumber {0};

        std::string toString() const noexcept override;
    };

    struct TfhdBox : FullBox {
        explicit TfhdBox(BoxHeader bHeader);

        unsigned int trackId {0};
        unsigned long int baseDataOffset {0};
        unsigned int sampleDescriptionIndex {0};
        unsigned int defaultSampleDuration {0};
        unsigned int defaultSampleSize {0};
        unsigned int defaultSampleFlags {0};

        bool durationIsEmpty {false};
        bool defaultBaseIsMoof {false};

        std::string toString() const noexcept override;
    };

    struct TfdtBox : FullBox {
        explicit TfdtBox(BoxHeader bHeader);

        unsigned long int baseMediaDecodeTime;

        std::string toString() const noexcept override;
    };

    struct TrunBox : FullBox {
        explicit TrunBox(BoxHeader bHeader);

        struct TrunSample {
            unsigned int sampleDuration {0};
            unsigned int sampleSize {0};
            unsigned int sampleFlags {0};
            unsigned int sampleCompositionTimeOffset {0};
            int sampleCompositionTimeOffsetSigned {0};

            std::string toString() const;
        };

        unsigned int sampleCount {0};
        int dataOffset {0};
        unsigned int firstSampleFlags {0};
        std::vector<TrunSample> samples;

        std::string toString() const noexcept override;
    };
}