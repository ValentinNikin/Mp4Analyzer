
#pragma once

#include <string>
#include <fstream>
#include <functional>

class Mp4Analyzer {
public:
    Mp4Analyzer();

    bool open(const std::string& path);

    void parse();

private:
    std::fstream _file;
    size_t _length;
};