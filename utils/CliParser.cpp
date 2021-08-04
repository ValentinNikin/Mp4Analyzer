
#include "CliParser.hpp"

#include <getopt.h>
#include <stdarg.h>
#include <cstring>
#include <iostream>
#include <cassert>

namespace {
    constexpr const char short_opts[] = "p:f:l:t:h";
    constexpr std::array<option, 6> long_opts = {
        option{ "path", 1, nullptr, 'p' },
        option{ "find", 1, nullptr, 'f' },
        option{ "level", 1, nullptr, 'l' },
        option{ "temp", 1, nullptr, 't' },
        option{ "help", 0, nullptr, 'h' },
        option{ nullptr, 0, nullptr, 0 }
    };

    void help(const char *const app, const char* problem = nullptr) {
        if (problem) {
            std::cout << "Problem: " << problem << std::endl;
        }

        std::cout << "Usage: " << app << std::endl;
        std::cout << "Supported options:" << std::endl
                    << "--path $path:       path to mp4 file" << std::endl
                    << "--find $string:     block name to find" << std::endl
                    << "--level $string:    level of the details (low/middle/high)" << std::endl
                    << "--temp $int:        temp value, only for check" << std::endl;
    }

    void error(
            const char *const app, 
            const char *const optarg,
            const int option,
            const char* problemDescription) {
        std::string optname;
        size_t optIndex {0};

        for (optIndex = 0; optIndex < long_opts.size(); optIndex++) {
            if (long_opts[optIndex].val == option) {
                optname = std::string("--") + std::string(long_opts[optIndex].name);
                break;
            }
        }

        if (optname.empty()) {
            assert(0 && "Unable to define option name");
        }

        help(app,
            ("Incorrect argument " + std::string(optarg) + 
                " for option " + optname + ", " + problemDescription).c_str());
    }

    bool parseLong(
                        const char *const optarg,
                        const int option,
                        const char *const app,
                        long& value) {
        char* end;
        auto tempValue = strtol(optarg, &end, 0);
        if (*end || end == optarg) {
            error(app, optarg, 't', "a value must be long");
            return false;
        }

        value = tempValue;
        return true;
    }

    bool parseLevelOfDetails(
                        const char *const optarg,
                        const int option,
                        const char *const app,
                        CliParser::Level& level) {
        CliParser::Level levelOfDetails {CliParser::Level::UNKNOWN};

        if (!strcmp(optarg, "low")) {
            levelOfDetails = CliParser::Level::LOW;
        } else if (!strcmp(optarg, "middle")) {
            levelOfDetails = CliParser::Level::MIDDLE;
        } else if (!strcmp(optarg, "high")) {
            levelOfDetails = CliParser::Level::HIGH;
        }

        if (levelOfDetails == CliParser::Level::UNKNOWN) {
            error(app, optarg, 'l', "a valid level of details is low/middle/high");
            return false;
        }

        level = levelOfDetails;
        return true;
    }
}

std::unique_ptr<CliParser::CliSettings> CliParser::cliParse(
        const int argc, char *const *const argv) {
    auto settings = std::make_unique<CliSettings>();

    int o;

    while ((o = getopt_long_only(argc, argv, short_opts, &long_opts[0], nullptr)) > 0) {
        switch (o)
        {
        case 'h':
            help(argv[0]);
            break;
        case 'p':
            settings->path = optarg;
            break;
        case 'f':
            settings->boxToFind = optarg;
            break;
        case 'l':
            CliParser::Level level;
            if (!parseLevelOfDetails(optarg, 'l', argv[0], level)) {
                return nullptr;
            }
            settings->levelOfDetails = level;
            break;
        case 't':
            long value;
            if (!parseLong(optarg, 't', argv[0], value)) {
                return nullptr;
            }
            settings->tempVarForCheck = value;
            break;
        
        default:
            break;
        }
    }

    return settings;
}
