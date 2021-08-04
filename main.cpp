
#include <iostream>

#include "utils/CliParser.hpp"
#include "Mp4Analyzer.hpp"

int main(int argc, char *argv[]) {
    std::cout << "Hello, world!" << std::endl;

    auto settings = CliParser::cliParse(argc, argv);

    if (!settings) {
        std::cout << "Unable to parse CLI parameters" << std::endl;
        return 1;
    }

    std::cout << 
            settings->path << " " << 
            settings->boxToFind << " " << 
            static_cast<int>(settings->levelOfDetails) << " " <<
            settings->tempVarForCheck << std::endl;
    
    auto mp4Analyzer = std::make_unique<Mp4Analyzer>();
    
    if (!mp4Analyzer->open(settings->path)) {
        std::cout << "Unable to open file " << settings->path << std::endl;
        return 1;
    }

    mp4Analyzer->parse();

    return 0;
}