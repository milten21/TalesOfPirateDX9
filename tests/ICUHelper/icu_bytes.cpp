#include "ResourceBundleManage.h"

#include <cstdio>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{

enum class ArgKind { Str, Long, Int64, Double };

struct Arg
{
	ArgKind kind;
	const char* str;
	int64_t integer;
	double real;
};

Arg S(const char* v) { return Arg{ ArgKind::Str, v, 0, 0 }; }
Arg L(int32_t v) { return Arg{ ArgKind::Long, nullptr, v, 0 }; }
Arg I64(int64_t v) { return Arg{ ArgKind::Int64, nullptr, v, 0 }; }
Arg D(double v) { return Arg{ ArgKind::Double, nullptr, 0, v }; }

struct FormatCase
{
	const char* server;
	const char* key;
	std::vector<Arg> args;
};

const char* const kNonAscii = "Zo\xEB \xC4\xE3\xBA\xC3";

std::vector<FormatCase> RealFormatCases()
{
	return {
		{ "GameServer", "GM_CHARACTERCMD_CPP_00006", { S("Player"), L(3), S("Item") } },
		{ "GameServer", "GM_CHARACTERCMD_CPP_00006", { S(kNonAscii), L(-3), S(kNonAscii) } },
		{ "GameServer", "GM_CHARSTALL_CPP_00052", { S("Buyer"), L(2), S("Ruby"), L(1234567), L(617283), L(99999999) } },
		{ "GameServer", "GM_CHARSTALL_CPP_00054", { S("Seller"), L(1), S("Sapphire"), L(2000000000), L(1000), L(0) } },
		{ "GameServer", "GM_CHARTRADE_CPP_00076", { S("Bob"), L(50000) } },
		{ "GameServer", "GM_CHARTRADE_CPP_00076", { S(kNonAscii), I64(5000000000LL) } },
		{ "GameServer", "GM_CHARTRADE_CPP_00076", { S("Bob"), D(1234.5) } },
		{ "GameServer", "GM_WEATHER_CPP_00001", { S("12"), S("34"), S("Storm") } },
	};
}

std::string Hex(const char* data, size_t length)
{
	static const char digits[] = "0123456789abcdef";
	std::string out;
	out.reserve(length * 2);
	for (size_t i = 0; i < length; ++i)
	{
		unsigned char c = static_cast<unsigned char>(data[i]);
		out.push_back(digits[c >> 4]);
		out.push_back(digits[c & 15]);
	}
	return out;
}

bool ReadFile(const std::string& path, std::string& out)
{
	std::ifstream in(path, std::ios::binary);
	if (!in)
		return false;
	std::ostringstream ss;
	ss << in.rdbuf();
	out = ss.str();
	out.erase(std::remove(out.begin(), out.end(), '\r'), out.end());
	return true;
}

std::string ExecutableDir()
{
#ifdef _WIN32
	char path[MAX_PATH];
	GetModuleFileNameA(nullptr, path, MAX_PATH);
	std::string p = path;
	return p.substr(0, p.find_last_of("\\/"));
#else
	return ".";
#endif
}

std::string FormatLine(CResourceBundleManage& manager, const char* key, const std::vector<Arg>& args)
{
	CFormatParameter param(static_cast<int>(args.size()));
	for (size_t i = 0; i < args.size(); ++i)
	{
		int index = static_cast<int>(i);
		switch (args[i].kind)
		{
		case ArgKind::Str: param.setString(index, args[i].str); break;
		case ArgKind::Long: param.setLong(index, static_cast<int32_t>(args[i].integer)); break;
		case ArgKind::Int64: param.setInt64(index, args[i].integer); break;
		case ArgKind::Double: param.setDouble(index, args[i].real); break;
		}
	}
	std::vector<char> buffer(1 << 20, 0);
	int len = manager.Format(key, param, buffer.data());
	return Hex(buffer.data(), static_cast<size_t>(len));
}

bool Dump(const std::string& server, const std::string& serverDir, std::string& out)
{
	std::string loc;
	if (!ReadFile(serverDir + "/" + server + ".loc", loc))
	{
		std::cerr << "cannot read " << serverDir << "/" << server << ".loc\n";
		return false;
	}

	std::string testLoc = "icubytes_" + server + ".loc";
	{
		std::string testLocPath = ExecutableDir() + "/" + testLoc;
		std::ofstream f(testLocPath, std::ios::binary);
		if (!f)
		{
			std::cerr << "cannot write " << testLocPath << "\n";
			return false;
		}
		f << loc << "\npath = " << serverDir << "\n";
	}

	CResourceBundleManage manager(testLoc.c_str());

	std::ostringstream o;
	o << "server " << server << "\n";

	std::vector<Arg> mixed = { S(kNonAscii), L(-1234567), I64(5000000000LL), D(1234.5) };

	for (const auto& entry : manager.GetResStrings())
	{
		const char* key = entry.first;
		const char* value = manager.LoadResString(key);
		o << "S " << key << " " << Hex(value, std::strlen(value)) << "\n";
		o << "F0 " << key << " " << FormatLine(manager, key, {}) << "\n";
		o << "F4 " << key << " " << FormatLine(manager, key, mixed) << "\n";
	}

	for (const auto& c : RealFormatCases())
		if (server == c.server)
			o << "FR " << c.key << " " << FormatLine(manager, c.key, c.args) << "\n";

	out = o.str();
	return true;
}

}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: ICUBytesCheck <server> <serverDir> <golden-file|--dump> [--default-codepage NAME]\n";
		return 2;
	}

	std::string server = argv[1];
	std::string serverDir = argv[2];
	std::string golden = argv[3];

	for (int i = 4; i + 1 < argc; ++i)
	{
		if (std::string(argv[i]) == "--default-codepage")
		{
			ucnv_setDefaultName(argv[i + 1]);
			break;
		}
	}

	std::string actual;
	if (!Dump(server, serverDir, actual))
		return 2;

	if (golden == "--dump")
	{
		std::fwrite(actual.data(), 1, actual.size(), stdout);
		return 0;
	}

	std::string expected;
	if (!ReadFile(golden, expected))
	{
		std::cerr << "cannot read golden file " << golden << "\n";
		return 2;
	}

	if (actual == expected)
	{
		std::cout << server << ": " << actual.size() << " bytes of output match the golden file\n";
		return 0;
	}

	std::istringstream a(actual), e(expected);
	std::string la, le;
	int shown = 0;
	while (shown < 20)
	{
		bool ha = static_cast<bool>(std::getline(a, la));
		bool he = static_cast<bool>(std::getline(e, le));
		if (!ha && !he)
			break;
		if (!ha) la.clear();
		if (!he) le.clear();
		if (la != le)
		{
			std::cerr << "expected: " << le << "\n  actual: " << la << "\n";
			++shown;
		}
	}
	std::cerr << server << ": output differs from the golden file\n";
	return 1;
}
