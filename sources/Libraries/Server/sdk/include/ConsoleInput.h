#pragma once

#include <functional>
#include <memory>
#include <string>

namespace dbc
{
class ConsoleInput
{
public:
	struct Lines;

	explicit ConsoleInput(std::function<bool()> exitRequested);

	bool ReadLine(std::string& line);

private:
	std::function<bool()> m_exitRequested;
	std::shared_ptr<Lines> m_lines;
};
}
