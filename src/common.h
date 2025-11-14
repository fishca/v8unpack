#ifndef CommonH
#define CommonH

#include <System.Classes.hpp>
#include <sstream>
#include <iomanip>

namespace fs = boost::filesystem;

const char str_cfu[] = ".cfu";
const char str_cfe[] = ".cfe";
const char str_cf[] = ".cf";
const char str_epf[] = ".epf";
const char str_erf[] = ".erf";
const char str_backslash[] = "\\";

// шаблон заголовка блока
const char _BLOCK_HEADER_TEMPLATE[] = "\r\n00000000 00000000 00000000 \r\n";
const char _EMPTY_CATALOG_TEMPLATE[16] = { '\xff','\xff','\xff','\x7f',0,2,0,0,0,0,0,0,0,0,0,0 };

const int32_t LAST_BLOCK = std::numeric_limits<int>::max();
const uint32_t BLOCK_HEADER_LEN = 32U;
const uint32_t CATALOG_HEADER_LEN = 16U;

const int64_t EPOCH_START_WIN = 504911232000000;

constexpr uint32_t HEX_INT_LEN = sizeof(int32_t) * 2;

//---------------------------------------------------------------------------
struct v8header_struct {
	int64_t time_create;
	int64_t time_modify;
	int32_t zero;
};

//---------------------------------------------------------------------------
struct catalog_header {
	int32_t start_empty; // начало первого пустого блока
	int32_t page_size;   // размер страницы по умолчанию
	int32_t version;     // версия
	int32_t zero;        // всегда ноль?
};

//---------------------------------------------------------------------------
struct fat_item {
	int header_start;
	int data_start;
	int ff; // всегда 7fffffff
};

//---------------------------------------------------------------------------
enum FileIsCatalog {
	iscatalog_unknown,
	iscatalog_true,
	iscatalog_false
};


enum class block_header : int {
	doc_len = 2,
	block_len = 11,
	nextblock = 20
};

void time1CD_to_FileTime(System::FILETIME* ft, unsigned char* time1CD);
unsigned int reverse_byte_order(unsigned int value);
String GUIDas1C(const unsigned char* fr);
String GUIDasMS(const unsigned char* fr);
String GUID_to_string(const System::TGUID& guid);
bool string_to_GUID(const String& str, System::TGUID* guid);
String GUID_to_string_flat(System::TGUID* guid);
bool string_to_GUID_flat(const String& str, System::TGUID* guid);
bool two_hex_digits_to_byte(const wchar_t hi, const wchar_t lo, unsigned char& res);
bool string1C_to_date(const String &str, void *bytedate);
bool string_to_date(const String &str, void *bytedate);
String date_to_string1C(const void *bytedate);
String date_to_string(const void *bytedate);
String hexstring(const char *buf, int n);
String hexstring(TStream* str);
String toXML(const String &in);
unsigned char from_hex_digit(char digit);

template< typename T >
String to_hex_string( T num, bool prefix = true ) {
	std::stringstream stream;
	if(prefix) {
		stream << "0x";
	};
	stream << std::setfill('0') << std::setw(sizeof(T) * 2)
		   << std::hex << num;
	return String(stream.str());
}

template< typename T >
std::wstring to_hex_wstring( T num, bool prefix = true ) {
	std::wstringstream stream;
	if(prefix) {
		stream << L"0x";
	};
	stream << std::setfill(L'0') << std::setw(sizeof(T) * 2)
		   << std::hex << num;
	return stream.str();
}

bool directory_exists(fs::path& check_path, bool create_directory = false);



#endif
