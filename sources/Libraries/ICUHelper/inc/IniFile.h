#pragma once

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

class CIniFile
{
public:
	CIniFile() {}

	bool Load(const std::string& path)
	{
		m_Values.clear();

		std::ifstream in(path, std::ios::binary);
		if (!in)
			return false;

		std::string line;
		std::string section;
		bool first = true;
		while (std::getline(in, line))
		{
			if (first)
			{
				first = false;
				if (line.compare(0, 3, "\xEF\xBB\xBF") == 0)
					line.erase(0, 3);
			}

			line = Trim(line);
			if (line.empty() || line[0] == ';')
				continue;

			if (line[0] == '[')
			{
				size_t close = line.find(']');
				if (close != std::string::npos)
					section = Lower(Trim(line.substr(1, close - 1)));
				continue;
			}

			size_t eq = line.find('=');
			if (eq == std::string::npos)
				continue;

			std::string key = Lower(Trim(line.substr(0, eq)));
			std::string value = Trim(line.substr(eq + 1));
			if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') && value.back() == value.front())
				value = value.substr(1, value.size() - 2);

			m_Values.emplace(section + "\n" + key, value);
		}

		return true;
	}

	std::string GetString(const std::string& section, const std::string& key, const std::string& fallback) const
	{
		auto it = m_Values.find(Lower(section) + "\n" + Lower(key));
		return it == m_Values.end() ? fallback : it->second;
	}

	int GetInt(const std::string& section, const std::string& key, int fallback) const
	{
		auto it = m_Values.find(Lower(section) + "\n" + Lower(key));
		if (it == m_Values.end())
			return fallback;

		const char* begin = it->second.c_str();
		char* end = nullptr;
		long parsed = std::strtol(begin, &end, 10);
		return end == begin ? 0 : static_cast<int>(parsed);
	}

private:
	static std::string Trim(const std::string& s)
	{
		size_t b = 0;
		size_t e = s.size();
		while (b < e && std::isspace(static_cast<unsigned char>(s[b])))
			++b;
		while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1])))
			--e;
		return s.substr(b, e - b);
	}

	static std::string Lower(std::string s)
	{
		for (auto& c : s)
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		return s;
	}

	std::map<std::string, std::string> m_Values;
};
