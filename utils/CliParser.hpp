#pragma once

#include <string>
#include <memory>

namespace CliParser {

	enum class Level : uint8_t {
        UNKNOWN,
		LOW,
		MIDDLE,
		HIGH
	};

	struct CliSettings {
		std::string path;
		std::string boxToFind;
		Level levelOfDetails{ Level::UNKNOWN };
		long tempVarForCheck {0};
	};

	std::unique_ptr<CliSettings> cliParse(const int argc, char *const *const argv);

}