#include <unicode/resbund.h>
#include <unicode/unistr.h>
#include <cstdio>
#include <string>

using namespace icu;

static std::string Escape(const UnicodeString& s)
{
	std::string out;
	for (int32_t i = 0; i < s.length();)
	{
		UChar32 c = s.char32At(i);
		i += U16_LENGTH(c);
		switch (c)
		{
		case '\\': out += "\\\\"; break;
		case '"': out += "\\\""; break;
		case '\n': out += "\\n"; break;
		case '\r': out += "\\r"; break;
		case '\t': out += "\\t"; break;
		default:
			if (c < 0x20 || c == 0x7f)
			{
				char buf[16];
				std::snprintf(buf, sizeof buf, "\\u%04X", (unsigned)c);
				out += buf;
			}
			else
			{
				std::string u8;
				UnicodeString(c).toUTF8String(u8);
				out += u8;
			}
		}
	}
	return out;
}

int main(int argc, char** argv)
{
	if (argc < 4)
		return 2;
	UErrorCode status = U_ZERO_ERROR;
	ResourceBundle rb(argv[1], Locale(argv[2]), status);
	if (U_FAILURE(status))
	{
		std::fprintf(stderr, "open failed %d\n", (int)status);
		return 1;
	}
	std::FILE* f = std::fopen(argv[3], "wb");
	std::fprintf(f, "%s{\n", argv[2]);
	rb.resetIterator();
	while (rb.hasNext())
	{
		ResourceBundle b = rb.getNext(status);
		UnicodeString v = b.getString(status);
		std::fprintf(f, "    %s { \"%s\" }\n", b.getKey(), Escape(v).c_str());
	}
	std::fprintf(f, "}\n");
	std::fclose(f);
	return U_FAILURE(status) ? 1 : 0;
}
