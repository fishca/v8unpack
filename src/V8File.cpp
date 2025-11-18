/*----------------------------------------------------------
This Source Code Form is subject to the terms of the
Mozilla Public License, v.2.0. If a copy of the MPL
was not distributed with this file, You can obtain one
at http://mozilla.org/MPL/2.0/.
----------------------------------------------------------*/
/////////////////////////////////////////////////////////////////////////////
//	Author:			disa_da
//	E-mail:			disa_da2@mail.ru
/////////////////////////////////////////////////////////////////////////////
/**
    2014-2021       dmpas       sergey(dot)batanov(at)dmpas(dot)ru
    2019-2020       fishca      fishcaroot(at)gmail(dot)com
 */

#include "V8File.h"
#include "VersionFile.h"
#include "logger.h"
#include "SystemClasses/String.hpp"
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <locale>
#include <codecvt>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <utility>
#include <memory>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <boost/filesystem/fstream.hpp>
#include <regex>
#include <unordered_map>

namespace v8unpack {

using namespace std;
namespace fs = boost::filesystem;

Logger logger("Debug\\app.log");

int RecursiveUnpack(
		const string                &directory,
		      basic_istream<char>   &file,
		const vector<string>        &filter,
		      bool                   boolInflate,
		      bool                   UnpackWhenNeed
);

template<typename format>
int SmartUnpackToString(basic_istream<char> &file, bool NeedUnpack, std::string &result);

int RecursiveUnpackToString(basic_istream<char> &file, const vector<string> &filter, bool boolInflate, std::string &result);

CV8File::CV8File()
{
    IsDataPacked = true;
}

CV8File::CV8File(const CV8File &src)
    : FileHeader(src.FileHeader), IsDataPacked(src.IsDataPacked)
{
    ElemsAddrs.assign(src.ElemsAddrs.begin(), src.ElemsAddrs.end());
    Elems.assign(src.Elems.begin(), src.Elems.end());
}

CV8Elem::CV8Elem(const string &name)
{
	auto HeaderSize = CV8Elem::stElemHeaderBegin::Size() + name.size() * 2 + 4; // последние четыре всегда нули?
	resizeHeader(HeaderSize);
	memset(header.data(), 0, HeaderSize);

	SetName(name);
}

string CV8Elem::GetName() const
{
	auto ElemNameLen = (header.size() - CV8Elem::stElemHeaderBegin::Size()) / 2;

	// Optimized: direct string construction instead of stringstream
	std::string result;
	result.reserve(ElemNameLen);

	auto currentChar = header.data() + CV8Elem::stElemHeaderBegin::Size();
	for (size_t j = 0; j < ElemNameLen * 2; j += 2, currentChar += 2) {
		if (*currentChar == '\0') {
			break;
		}
		result.push_back(*currentChar);
	}

	return result;
}

void CV8Elem::resizeHeader(size_t newSize)
{
	header.resize(newSize, 0);
}

int CV8Elem::SetName(const string &ElemName)
{
	uint32_t pos = CV8Elem::stElemHeaderBegin::Size();

	// Optimized: avoid division in loop by using separate index
	for (size_t i = 0; i < ElemName.size(); ++i, pos += 2) {
		header[pos] = ElemName[i];
		header[pos + 1] = 0;
	}

	return 0;
}

void CV8Elem::Dispose()
{
	IsV8File = false;
}

template<typename format>
static size_t
ReadBlockData(basic_istream<char> &file, const typename format::block_header_t &firstBlockHeader, char *pBlockData)
{
	auto data_size = firstBlockHeader.data_size();
	auto Header = firstBlockHeader;
	auto pBlockHeader = &Header;

	uint32_t read_in_bytes = 0;
	while (read_in_bytes < data_size) {

		auto page_size = pBlockHeader->page_size();
		auto next_page_addr = pBlockHeader->next_page_addr();
		auto bytes_to_read = MIN(page_size, data_size - read_in_bytes);

		file.read(&pBlockData[read_in_bytes], bytes_to_read);

		read_in_bytes += bytes_to_read;

		if (next_page_addr != format::UNDEFINED_VALUE) { // есть следующая страница
			file.seekg(next_page_addr + format::BASE_OFFSET, ios_base::beg);
			file.read((char*)&Header, Header.Size());
		}
		else
			break;
	}

	return data_size;
}

template<typename format>
static size_t
ReadBlockData(basic_istream<char> &file, const typename format::block_header_t &firstBlockHeader, vector<char> &out)
{
 	auto data_size = firstBlockHeader.data_size();
	out.resize(data_size);
	auto pBlockData = out.data();
	auto Header = firstBlockHeader;
	auto pBlockHeader = &Header;

	uint32_t read_in_bytes = 0;
	while (read_in_bytes < data_size) {

		auto page_size = pBlockHeader->page_size();
		auto next_page_addr = pBlockHeader->next_page_addr();
		auto bytes_to_read = MIN(page_size, data_size - read_in_bytes);

		file.read(&pBlockData[read_in_bytes], bytes_to_read);

		read_in_bytes += bytes_to_read;

		if (next_page_addr != format::UNDEFINED_VALUE) { // есть следующая страница
			file.seekg(next_page_addr + format::BASE_OFFSET, ios_base::beg);
			file.read((char*)&Header, Header.Size());
		}
		else
			break;
	}

	return data_size;
}

template<typename format>
static size_t
ReadBlockData(basic_istream<char> &file, const typename format::block_header_t &firstBlockHeader, basic_ostream<char> &out)
{
	uint32_t read_in_bytes;

	auto data_size = firstBlockHeader.data_size();
	auto Header = firstBlockHeader;
	auto pBlockHeader = &Header;

	// Optimized buffer size: 64KB for better I/O performance
	const int buf_size = 65536;
	std::vector<char> buffer(buf_size);

	read_in_bytes = 0;
	while (read_in_bytes < data_size) {

		auto page_size = pBlockHeader->page_size();
		auto next_page_addr = pBlockHeader->next_page_addr();
		auto bytes_to_read = MIN(page_size, data_size - read_in_bytes);

		uint32_t read_done = 0;
		while (read_done < bytes_to_read) {
			file.read(buffer.data(), MIN(buf_size, bytes_to_read - read_done));
			uint32_t rd = file.gcount();
			out.write(buffer.data(), rd);
			read_done += rd;
		}

		read_in_bytes += bytes_to_read;

		if (next_page_addr != format::UNDEFINED_VALUE) { // есть следующая страница
			file.seekg(next_page_addr + format::BASE_OFFSET, ios_base::beg);
			file.read((char*)&Header, Header.Size());
		}
		else
			break;
	}

	return V8UNPACK_OK;
}

template<typename format>
static size_t
DumpBlockData(basic_istream<char> &file, const typename format::block_header_t &firstBlockHeader, const fs::path &path)
{
	fs::ofstream out(path, ios_base::binary);
	return ReadBlockData<format>(file, firstBlockHeader, out);
}


template<typename format, typename in_stream_t, typename out_stream_t>
static size_t
ReadBlockData(in_stream_t &file, out_stream_t &out)
{
	typename format::block_header_t firstBlockHeader;
	file.read((char*)&firstBlockHeader, firstBlockHeader.Size());
	return ReadBlockData<format>(file, firstBlockHeader, out);
}

template<typename format, typename element_t, typename in_stream_t>
static vector<element_t>
ReadVector(in_stream_t &file)
{
	typename format::block_header_t firstBlockHeader;
	file.read((char*)&firstBlockHeader, firstBlockHeader.Size());
	auto elements_count = firstBlockHeader.data_size() / sizeof(element_t);

	// Optimized: resize() is sufficient, no need for reserve()
	vector<element_t> result(elements_count);

	auto bytes_read = ReadBlockData<format>(file, firstBlockHeader, reinterpret_cast<char*>(result.data()));
	result.resize(bytes_read / sizeof(element_t));

	return result;
}

template<typename format, typename in_stream_t>
static vector<typename format::elem_addr_t>
ReadElementsAllocationTable(in_stream_t &file)
{
	return ReadVector<format, typename format::elem_addr_t, in_stream_t>(file);
}

template<typename format, typename in_stream_t, typename out_stream_t>
static bool
SafeReadBlockData(in_stream_t &file, out_stream_t &out, size_t &data_size)
{
	typename format::block_header_t firstBlockHeader;
	file.read((char*)&firstBlockHeader, firstBlockHeader.Size());
	if (!firstBlockHeader.IsCorrect()) {
		return false;
	}
	data_size = ReadBlockData<format>(file, firstBlockHeader, out);
	return true;
}

template<typename format, typename in_stream_t, typename out_stream_t>
static bool
SafeReadBlockData(in_stream_t &file, out_stream_t &out)
{
	size_t data_size;
	return SafeReadBlockData<format>(file, out, data_size);
}

template<typename format>
static
int SaveBlockDataToBuffer(char *&cur_pos, const char *pBlockData, uint32_t BlockDataSize, uint32_t PageSize = format::DEFAULT_PAGE_SIZE)
{
	if (PageSize < BlockDataSize)
		PageSize = BlockDataSize;

	typename format::block_header_t CurBlockHeader = format::block_header_t::create(BlockDataSize, PageSize, format::UNDEFINED_VALUE);
	memcpy(cur_pos, (char*)&CurBlockHeader, format::block_header_t::Size());
	cur_pos += format::block_header_t::Size();

	memcpy(cur_pos, pBlockData, BlockDataSize);
	cur_pos += BlockDataSize;

	// Optimized: use memset instead of loop
	uint32_t padding_size = PageSize - BlockDataSize;
	if (padding_size > 0) {
		memset(cur_pos, 0, padding_size);
		cur_pos += padding_size;
	}

	return 0;
}

template<typename format>
int SaveBlockData(basic_ostream<char> &file_out, const vector<char> &data, size_t PageSize = format::DEFAULT_PAGE_SIZE)
{
	auto BlockDataSize = data.size();
	if (PageSize < BlockDataSize)
		PageSize = BlockDataSize;

	auto CurBlockHeader = format::block_header_t::create(BlockDataSize, PageSize);
	file_out.write(reinterpret_cast<char *>(&CurBlockHeader), CurBlockHeader.Size());
	file_out.write(data.data(), BlockDataSize);

	// Optimized: write zeros in blocks instead of byte-by-byte
	if (PageSize > BlockDataSize) {
		size_t padding_size = PageSize - BlockDataSize;
		static const std::vector<char> zeros(4096, 0); // Reusable 4KB zero buffer
		while (padding_size > 0) {
			size_t write_size = std::min(padding_size, zeros.size());
			file_out.write(zeros.data(), write_size);
			padding_size -= write_size;
		}
	}

	return V8UNPACK_OK;
}

template<typename format>
int SaveBlockData(basic_ostream<char> &file_out, basic_istream<char> &file_in, size_t BlockDataSize, size_t PageSize = format::DEFAULT_PAGE_SIZE)
{
	if (PageSize < BlockDataSize)
		PageSize = BlockDataSize;

	auto CurBlockHeader = format::block_header_t::create(BlockDataSize, PageSize);
	file_out.write(reinterpret_cast<char *>(&CurBlockHeader), CurBlockHeader.Size());
	full_copy(file_in, file_out);

	// Optimized: write zeros in blocks instead of byte-by-byte
	if (PageSize > BlockDataSize) {
		size_t padding_size = PageSize - BlockDataSize;
		static const std::vector<char> zeros(4096, 0); // Reusable 4KB zero buffer
		while (padding_size > 0) {
			size_t write_size = std::min(padding_size, zeros.size());
			file_out.write(zeros.data(), write_size);
			padding_size -= write_size;
		}
	}

	return V8UNPACK_OK;
}

template<typename format>
int SaveBlockData(basic_ostream<char> &file_out, fs::path &in_file_path)
{
	auto BlockDataSize = fs::file_size(in_file_path);
	auto PageSize = BlockDataSize;

	fs::ifstream file_in(in_file_path, std::ios_base::binary);

	return SaveBlockData<format>(file_out, file_in, BlockDataSize, PageSize);
}

template<typename format>
int SaveBlockData(basic_ostream<char> &file_out, const char *pBlockData, size_t BlockDataSize, size_t PageSize = format::DEFAULT_PAGE_SIZE)
{
	if (PageSize < BlockDataSize)
		PageSize = BlockDataSize;

	auto CurBlockHeader = format::block_header_t::create(BlockDataSize, PageSize);
	file_out.write(reinterpret_cast<char *>(&CurBlockHeader), CurBlockHeader.Size());
	file_out.write(reinterpret_cast<const char *>(pBlockData), BlockDataSize);

	// Optimized: write zeros in blocks instead of byte-by-byte
	if (PageSize > BlockDataSize) {
		size_t padding_size = PageSize - BlockDataSize;
		static const std::vector<char> zeros(4096, 0); // Reusable 4KB zero buffer
		while (padding_size > 0) {
			size_t write_size = std::min(padding_size, zeros.size());
			file_out.write(zeros.data(), write_size);
			padding_size -= write_size;
		}
	}

	return V8UNPACK_OK;
}

static int
directory_container_compatibility(const string &in_dirname)
{
	{ // распакованный файл version (после Parse)
		auto version_file_path = fs::path(in_dirname) / "version";
		if (fs::exists(version_file_path)) {
			fs::ifstream version_in(version_file_path);
			auto v = VersionFile::parse(version_in);
			return v.compatibility();
		}
	}
	{ // нераспакованный файл version (после Unpack)
		auto version_file_path = fs::path(in_dirname) / "version.data";
		if (fs::exists(version_file_path)) {
			fs::ifstream version_in(version_file_path);
			std::stringstream contentStream;
			try_inflate(version_in, contentStream);
			contentStream.seekg(0);
			auto v = VersionFile::parse(contentStream);
			return v.compatibility();
		}
	}
	return VersionFile::COMPATIBILITY_DEFAULT;
}

void CV8File::Dispose()
{
	vector<CV8Elem>::iterator elem;
	for (elem = Elems.begin(); elem != Elems.end(); ++elem) {
		elem->Dispose();
	}
	Elems.clear();
}

// Нѣкоторый условный предѣл
const size_t SmartLimit = 00 *1024;
const size_t SmartUnpackedLimit = 20 *1024*1024;

/*
	Лучше всѣго сжимается текст
	Берём степень сжатія текста в 99% (объём распакованных данных в 100 раз больше)
	Берём примѣрный порог использованія памяти в 20МБ (в этот объём должы влезть распакованные данные)
	Дѣлим 20МБ на 100 и получаем 200 КБ
	Упакованные данные размѣром до 200 КБ можно спокойно обрабатывать в памяти

	В дальнейшем этот показатель всё же будет вынесен в параметр командной строки
*/

class data_source_t
{
public:
	virtual istream &stream() = 0;
	virtual void save_as(const fs::path &dest) = 0;
	virtual ~data_source_t() = default;
};

class temp_file_data_source_t : public data_source_t
{
public:
	explicit temp_file_data_source_t(const fs::path &name) :
		path(name),
		file(name, ios_base::binary)
		{}

	istream &stream() override { return file; }

	void save_as(const fs::path &dest) override
	{
		file.close();
		boost::system::error_code error;
		fs::rename(path, dest, error);
	}

	~temp_file_data_source_t() override
	{
		if (file) {
			file.close();
			boost::system::error_code ec;
			fs::remove(path, ec);
		}
	}
private:
	fs::path path;
	fs::ifstream file;
};

class vector_data_source_t : public data_source_t
{
public:
	explicit vector_data_source_t(vector<char> data) :
			__data(move(data)),
			__stream(__data.data(), __data.size())
	{}

	istream &stream() override { return __stream; }

	void save_as(const fs::path &dest) override
	{
		__stream.seekg(0);
		fs::ofstream out(dest, ios_base::binary);
		full_copy(__stream, out);
	}

	~vector_data_source_t() override = default;

private:
	vector<char> __data;
	boost::iostreams::stream<boost::iostreams::array_source> __stream;
};

template<typename format>
unique_ptr<data_source_t>
prepare_smart_source(basic_istream<char> &file, bool NeedUnpack, fs::path &elem_path)
{
	typename format::block_header_t header;

	file.read((char*)&header, header.Size());
	auto data_size = header.data_size();

	if (NeedUnpack && data_size < SmartLimit) {
		vector<char> source_data;
		ReadBlockData<format>(file, header, source_data);
		try_inflate(source_data);

		return make_unique<vector_data_source_t>(source_data);
	}

	auto tmp_path = elem_path.parent_path() / ".v8unpack.tmp";
	DumpBlockData<format>(file, header, tmp_path);

	if (NeedUnpack) {
		auto inf_path = elem_path.parent_path() / ".v8unpack.inf";
		try_inflate(tmp_path, inf_path);
		fs::remove(tmp_path);

		return make_unique<temp_file_data_source_t>(inf_path);
	}

	return make_unique<temp_file_data_source_t>(tmp_path);
}

template<typename format>
String prepare_smart_source_to_string(basic_istream<char>& file, bool NeedUnpack)
{
	typename format::block_header_t header;
	file.read((char*)&header, header.Size());

	// Optimized: removed dead code and unnecessary copies
	vector<char> source_data;
	ReadBlockData<format>(file, header, source_data);

	if (NeedUnpack) {
		try_inflate(source_data);
	}

	return std::string(source_data.begin(), source_data.end());
}



template<typename format>
int SmartUnpack(basic_istream<char> &file, bool NeedUnpack, fs::path &elem_path)
{
	auto src = prepare_smart_source<format>(file, NeedUnpack, elem_path);
	auto unpack_result = RecursiveUnpack(elem_path.string(), src->stream(), {}, false, false);
	if (unpack_result != V8UNPACK_OK) {
		src->save_as(elem_path);
	}
	return V8UNPACK_OK;
}

// Optimized: O(1) lookup with unordered_set instead of O(n) vector search
static bool NameInFilter(const string &name, const vector<string> &filter)
{
	if (filter.empty()) {
		return true;
	}

	// Create static unordered_set for O(1) lookup
	// This is computed once and reused across calls
	static std::unordered_set<string> filter_set;
	static const vector<string>* last_filter = nullptr;

	// Rebuild set only if filter changed
	if (&filter != last_filter) {
		filter_set.clear();
		filter_set.insert(filter.begin(), filter.end());
		last_filter = &filter;
	}

	return filter_set.find(name) != filter_set.end();
}

template<typename format>
static int recursive_unpack(const string& directory, basic_istream<char>& file, const vector<string>& filter, bool boolInflate, bool UnpackWhenNeed)
{
	int ret = 0;

	fs::path p_dir(directory);

	if (!fs::exists(p_dir)) {
		if (!fs::create_directory(directory)) {
			logger.log("RecursiveUnpack. Ошибка создания каталога распаковки");
			cerr << "RecursiveUnpack. Error in creating directory!" << endl;
			return ret;
		}
	}

	typename format::file_header_t FileHeader;

	ifstream::pos_type offset = format::BASE_OFFSET;
	file.seekg(offset);
	file.read((char*)& FileHeader, FileHeader.Size());

	auto pElemsAddrs = ReadElementsAllocationTable<format>(file);
	auto ElemsNum = pElemsAddrs.size();

	logger.log("RecursiveUnpack. Найдено " + std::to_string(ElemsNum) + " файлов");

	for (uint32_t i = 0; i < ElemsNum; i++) {

		if (pElemsAddrs[i].fffffff != format::UNDEFINED_VALUE) {
			ElemsNum = i;
			break;
		}

		file.seekg(pElemsAddrs[i].elem_header_addr + format::BASE_OFFSET, ios_base::beg);

		CV8Elem elem;

		if (!SafeReadBlockData<format>(file, elem.header)) {
			ret = V8UNPACK_HEADER_ELEM_NOT_CORRECT;
			break;
		}
		string ElemName = elem.GetName();

		logger.log("RecursiveUnpack. Обрабатывается файл " + ElemName);

		if (!NameInFilter(ElemName, filter)) {
			continue;
		}

		fs::path elem_path= fs::absolute(p_dir / ElemName);

		logger.log("RecursiveUnpack. Создаем каталог " + elem_path.string());

		//080228 Блока данных может не быть, тогда адрес блока данных равен 0xffffffffffffffff
		if (pElemsAddrs[i].elem_data_addr != format::UNDEFINED_VALUE) {
			file.seekg(pElemsAddrs[i].elem_data_addr + format::BASE_OFFSET, ios_base::beg);
			SmartUnpack<format>(file, boolInflate, elem_path);
		}

	} // for i = ..ElemsNum

	//logger.log("RecursiveUnpack. Завершена обработка всех файлов...");

	return ret;
}

template<typename format>
static int recursive_unpack_to_string(basic_istream<char>& file, const vector<string>& filter, bool boolInflate, std::string& result)
{
	int ret = V8UNPACK_OK;

	typename format::file_header_t FileHeader;

	ifstream::pos_type offset = format::BASE_OFFSET;
	file.seekg(offset);
	file.read((char*)& FileHeader, FileHeader.Size());

	auto pElemsAddrs = ReadElementsAllocationTable<format>(file);
	auto ElemsNum = pElemsAddrs.size();

	for (uint32_t i = 0; i < ElemsNum; i++) {

		if (pElemsAddrs[i].fffffff != format::UNDEFINED_VALUE) {
			ElemsNum = i;
			break;
		}

		file.seekg(pElemsAddrs[i].elem_header_addr + format::BASE_OFFSET, ios_base::beg);

		CV8Elem elem;

		if (!SafeReadBlockData<format>(file, elem.header)) {
			ret = V8UNPACK_HEADER_ELEM_NOT_CORRECT;
			break;
		}
		string ElemName = elem.GetName();

		if (!NameInFilter(ElemName, filter)) {
			continue;
		}

		// Добавляем имя элемента в результат
		result += "--- " + ElemName + " ---\n";

		//080228 Блока данных может не быть, тогда адрес блока данных равен 0xffffffffffffffff
		if (pElemsAddrs[i].elem_data_addr != format::UNDEFINED_VALUE) {
			file.seekg(pElemsAddrs[i].elem_data_addr + format::BASE_OFFSET, ios_base::beg);
			SmartUnpackToString<format>(file, boolInflate, result);
		}

		result += "\n";

	} // for i = ..ElemsNum

	return ret;
}

template<typename format>
static int list_files(fs::ifstream &file)
{
	typename format::file_header_t FileHeader;

	file.seekg(format::BASE_OFFSET);
	file.read((char*)&FileHeader, FileHeader.Size());

	auto pElemsAddrs = ReadElementsAllocationTable<format>(file);
	auto ElemsNum = pElemsAddrs.size();

	for (uint32_t i = 0; i < ElemsNum; i++) {
		if (pElemsAddrs[i].fffffff != format::UNDEFINED_VALUE) {
			ElemsNum = i;
			break;
		}

		file.seekg(pElemsAddrs[i].elem_header_addr + format::BASE_OFFSET, ios_base::beg);

		CV8Elem elem;

		if (!SafeReadBlockData<format>(file, elem.header)) {
			continue;
		}

		cout << elem.GetName() << endl;
	}

	return V8UNPACK_OK;
}

int ListFiles(const string &filename)
{
	fs::ifstream file(filename, ios_base::binary);

	if (!file) {
		cerr << "ListFiles `" << filename << "`. Input file not found!" << endl;
		return V8UNPACK_SOURCE_DOES_NOT_EXIST;
	}

	if (!IsV8File(file)) {
		return V8UNPACK_NOT_V8_FILE;
	}

	if (IsV8File16(file)) {
		return list_files<Format16>(file);
	}

	return list_files<Format15>(file);
}

template<typename format>
static int unpack_to_folder(fs::ifstream &file, const string &dirname, const string &UnpackElemWithName, bool print_progress)
{
	int ret = V8UNPACK_OK;

	fs::path p_dir(dirname);

	if (!fs::exists(p_dir)) {
		if (!fs::create_directory(dirname)) {
			cerr << "RecursiveUnpack. Error in creating directory!" << endl;
			return ret;
		}
	}

	typename format::file_header_t FileHeader;

	ifstream::pos_type offset = format::BASE_OFFSET;
	file.seekg(offset);
	file.read((char*)&FileHeader, FileHeader.Size());

	if (UnpackElemWithName.empty()) {
		fs::path filename_out(dirname);
		filename_out /= "FileHeader";
		fs::ofstream file_out(filename_out, ios_base::binary);
		file_out.write((char*)&FileHeader, FileHeader.Size());
		file_out.close();
	}

	auto pElemsAddrs = ReadElementsAllocationTable<format>(file);
	auto ElemsNum = pElemsAddrs.size();

	for (uint32_t i = 0; i < ElemsNum; i++) {

		if (pElemsAddrs[i].fffffff != format::UNDEFINED_VALUE) {
			ElemsNum = i;
			break;
		}

		file.seekg(pElemsAddrs[i].elem_header_addr + offset, ios_base::beg);

		CV8Elem elem;
		if (!SafeReadBlockData<format>(file, elem.header)) {
			ret = V8UNPACK_HEADER_ELEM_NOT_CORRECT;
			break;
		}

		string ElemName = elem.GetName();

		// если передано имя блока для распаковки, пропускаем все остальные
		if (!UnpackElemWithName.empty() && UnpackElemWithName != ElemName) {
			continue;
		}

		fs::ofstream header_out;
		header_out.open(p_dir / (ElemName + ".header"), ios_base::binary);
		if (!header_out) {
			cerr << "UnpackToFolder. Error in creating file!" << endl;
			return -1;
		}
		header_out.write(elem.header.data(), elem.header.size());
		header_out.close();

		fs::ofstream data_out;
		data_out.open(p_dir / (ElemName + ".data"), ios_base::binary);
		if (!data_out) {
			cerr << "UnpackToFolder. Error in creating file!" << endl;
			return -1;
		}
		if (pElemsAddrs[i].elem_data_addr != format::UNDEFINED_VALUE) {
			file.seekg(pElemsAddrs[i].elem_data_addr + offset, ios_base::beg);
			ReadBlockData<format>(file, data_out);
		}
		data_out.close();
	}

	return V8UNPACK_OK;
}

int UnpackToFolder(const string &filename_in, const string &dirname, const string &UnpackElemWithName, bool print_progress)
{
	int ret = 0;

	fs::ifstream file(filename_in, ios_base::binary);

	if (!file) {
		cerr << "UnpackToFolder. Input file not found!" << endl;
		return -1;
	}

	if (!IsV8File(file)) {
		return V8UNPACK_NOT_V8_FILE;
	}

	if (IsV8File16(file)) {
		return unpack_to_folder<Format16>(file, dirname, UnpackElemWithName, print_progress);
	}

	return unpack_to_folder<Format15>(file, dirname, UnpackElemWithName, print_progress);
}

template <typename format>
static bool checkV8File(basic_istream<char> &file)
{
	bool result = false;

	auto offset = file.tellg();
	file.seekg(0, file.end);
	auto file_size = file.tellg();

	typename format::file_header_t FileHeader;
	if (file_size >= format::BASE_OFFSET + FileHeader.Size()) {

		file.seekg(format::BASE_OFFSET);
		file.read((char *) &FileHeader, FileHeader.Size());

		typename format::block_header_t BlockHeader;
		if (file_size >= format::BASE_OFFSET + FileHeader.Size() + BlockHeader.Size()) {
			memset(&BlockHeader, 0, BlockHeader.Size());
			file.read((char *) &BlockHeader, BlockHeader.Size());
			result = BlockHeader.IsCorrect();
		} else {
			// Если в файле нет первого блока, значит адрес страницы должен быть UNDEFINED
			result = (FileHeader.next_page_addr == format::UNDEFINED_VALUE);
		}
	}
	file.seekg(offset);
	file.clear();

	return result;
}

bool IsV8File(basic_istream<char> &file)
{
	return checkV8File<Format15>(file);
}

bool IsV8File16(basic_istream<char>& file)
{
	return checkV8File <Format16> (file);
}

struct PackElementEntry {
	fs::path  header_file;
	fs::path  data_file;
	size_t                   header_size = 0;
	size_t                   data_size = 0;
};

template<typename format>
static int
pack_from_folder(const fs::path &p_curdir, fs::ofstream &file_out)
{
	file_out << format::placeholder;
	{
		fs::ifstream file_in(p_curdir / "FileHeader", ios_base::binary);
		full_copy(file_in, file_out);
	}

	fs::directory_iterator d_end;
	fs::directory_iterator it(p_curdir);

	vector<PackElementEntry> Elems;

	for (; it != d_end; it++) {
		fs::path current_file(it->path());
		if (current_file.extension().string() == ".header") {

			PackElementEntry elem;

			elem.header_file = current_file;
			elem.header_size = fs::file_size(current_file);

			elem.data_file = current_file.replace_extension(".data");
			elem.data_size = fs::file_size(elem.data_file);

			Elems.push_back(elem);

		}
	} // for it

	auto ElemsNum = Elems.size();
	vector<typename format::elem_addr_t> ElemsAddrs;
	ElemsAddrs.reserve(ElemsNum);

	// cur_block_addr - смещение текущего блока
	// мы должны посчитать:
	//  + [0] заголовок файла
	//  + [1] заголовок блока с адресами
	//  + [2] размер самого блока адресов (не менее одной страницы?)
	//  + для каждого блока:
	//      + [3] заголовок блока метаданных (header)
	//      + [4] сами метаданные (header)
	//      + [5] заголовок данных
	//      + [6] сами данные (не менее одной страницы?)

	// [0] + [1]
	uint32_t cur_block_addr = format::file_header_t::Size() + format::block_header_t::Size();
	size_t addr_block_size = MAX(format::elem_addr_t::Size() * ElemsNum, format::DEFAULT_PAGE_SIZE);
	cur_block_addr += addr_block_size; // +[2]

	for (const auto &elem : Elems) {
		typename format::elem_addr_t addr;

		addr.elem_header_addr = cur_block_addr;
		cur_block_addr += format::block_header_t::Size() + elem.header_size; // +[3]+[4]

		addr.elem_data_addr = cur_block_addr;
		cur_block_addr += format::block_header_t::Size(); // +[5]
		cur_block_addr += MAX(elem.data_size, format::DEFAULT_PAGE_SIZE); // +[6]

		addr.fffffff = format::UNDEFINED_VALUE;

		ElemsAddrs.push_back(addr);
	}

	SaveBlockData<format>(file_out, (char*) ElemsAddrs.data(), format::elem_addr_t::Size() * ElemsNum);

	for (const auto &elem : Elems) {

		fs::ifstream header_in(elem.header_file, ios_base::binary);
		SaveBlockData<format>(file_out, header_in, elem.header_size, elem.header_size);

		fs::ifstream data_in(elem.data_file, ios_base::binary);
		SaveBlockData<format>(file_out, data_in, elem.data_size, V8_DEFAULT_PAGE_SIZE);
	}

	file_out.close();

	return V8UNPACK_OK;
}

int PackFromFolder(const string &dirname, const string &filename_out)
{
	fs::path p_curdir(dirname);
	fs::ofstream file_out(filename_out, ios_base::binary);
	if (!file_out) {
		cerr << "SaveFile. Error in creating file: " << filename_out << endl;
		return -1;
	}

	int compatibility = directory_container_compatibility(dirname);
	if (compatibility >= VersionFile::COMPATIBILITY_V80316) {
		return pack_from_folder<Format16>(p_curdir, file_out);
	}

	return pack_from_folder<Format15>(p_curdir, file_out);
}

int RecursiveUnpack(const string &directory, basic_istream<char> &file, const vector<string> &filter, bool boolInflate, bool UnpackWhenNeed)
{
	if (!IsV8File(file)) {
		//logger.log("Переданный файл не является файлом конфигурации 1С");
		return V8UNPACK_NOT_V8_FILE;
	}

	if (IsV8File16(file)) {
		logger.log("Обнаружен формат файла 8.3.16");
		return recursive_unpack<Format16>(directory, file, filter, boolInflate, UnpackWhenNeed);
	}

	return recursive_unpack<Format15>(directory, file, filter, boolInflate, UnpackWhenNeed);
}

template<typename format>
int SmartUnpackToString(basic_istream<char> &file, bool NeedUnpack, std::string &result)
{
	auto element_data = prepare_smart_source_to_string<format>(file, NeedUnpack);

	// Если элемент содержит вложенные данные V8 - распаковываем их рекурсивно в строку
	std::istringstream element_stream(element_data);
	auto unpack_result = RecursiveUnpackToString(element_stream, {}, NeedUnpack, result);
	if (unpack_result != V8UNPACK_OK) {
		// Если не удалось распаковать как V8 файл, добавляем данные как есть
		result += element_data;
	}
	return V8UNPACK_OK;
}

/**
 * @brief Рекурсивное распаковывание в строку
 * Функция аналогична RecursiveUnpack, но собирает все данные в строку вместо файловой системы
 */
int RecursiveUnpackToString(basic_istream<char> &file, const vector<string> &filter, bool boolInflate, std::string &result)
{
	if (!IsV8File(file)) {
		return V8UNPACK_NOT_V8_FILE;
	}

	if (IsV8File16(file)) {
		return recursive_unpack_to_string<Format16>(file, filter, boolInflate, result);
	}

	return recursive_unpack_to_string<Format15>(file, filter, boolInflate, result);
}

int Parse(const string &filename_in, const string &dirname, const vector< string > &filter)
{
    int ret = 0;

    fs::ifstream file_in(filename_in, ios_base::binary);

	logger.log("Начало распаковки конфигурации");

    if (!file_in) {
        cerr << "Parse. `" << filename_in << "` not found!" << endl;
        return -1;
    }

    ret = RecursiveUnpack(dirname, file_in, filter, true, false);

    if (ret == V8UNPACK_NOT_V8_FILE) {
        cerr << "Parse. `" << filename_in << "` is not V8 file!" << endl;
        return ret;
    }

    cout << "Parse `" << filename_in << "`: ok" << endl << flush;

	logger.log("Окончание распаковки");

    return ret;
}

/**
 * @brief Распаковывание конфигурации 1C v8 в строку
 * Функция аналогична Parse, но сохраняет все распакованные данные в строку вместо директория
 * @param filename_in Входной файл V8 (cf, epf, erf)
 * @param filter Список имен элементов для распаковки (если пустой - все элементы)
 * @param result Выходная строка с распакованными данными
 * @return Код возврата (0 - успешно, отрицательные значения - ошибки)
 */
int ParseToString(const string &filename_in, const vector<string> &filter, std::string &result)
{
    int ret = 0;

    fs::ifstream file_in(filename_in, ios_base::binary);

	logger.log("Начало распаковки конфигурации в строку");

    if (!file_in) {
        cerr << "ParseToString. `" << filename_in << "` not found!" << endl;
        return -1;
    }

    ret = RecursiveUnpackToString(file_in, filter, true, result);

    if (ret == V8UNPACK_NOT_V8_FILE) {
        cerr << "ParseToString. `" << filename_in << "` is not V8 file!" << endl;
        return ret;
    }

    logger.log("Окончание распаковки в строку");

    return ret;
}

int Parse_Test(const string& filename_in, const string& dirname, const vector< string >& filter)
{
	int ret = 0;

	fs::ifstream file_in(filename_in, ios_base::binary);

	if (!file_in) {
		cerr << "Parse. `" << filename_in << "` not found!" << endl;
		return -1;
	}

	ret = RecursiveUnpack(dirname, file_in, filter, true, false);

	if (ret == V8UNPACK_NOT_V8_FILE) {
		cerr << "Parse. `" << filename_in << "` is not V8 file!" << endl;
		return ret;
	}

	cout << "Parse `" << filename_in << "`: ok" << endl << flush;

	return ret;
}

std::wstring string_to_wstring(const std::string& str) {
	if (str.empty()) return L"";

#ifdef _WIN32
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
	return wstr;
#else
	// Linux: use codecvt for UTF-8 to wstring conversion
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
#endif
}

std::string wstring_to_string(const std::wstring& wstr, bool utf8) {
	if (wstr.empty())
		return "";

#ifdef _WIN32
	UINT code_page = utf8 ? CP_UTF8 : CP_ACP;
	int size_needed = WideCharToMultiByte(code_page, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string str(size_needed, 0);
	WideCharToMultiByte(code_page, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
	return str;
#else
	// Linux: use codecvt for wstring to UTF-8 conversion
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
#endif
}

// std::wstring (UTF-16) -> std::string (UTF-8)
std::string wstring_to_utf8(const std::wstring& str) {
	if (str.empty())
		return std::string();

#ifdef _WIN32
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0, nullptr, nullptr);
	std::string result(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size_needed, nullptr, nullptr);
	return result;
#else
	// Linux: use codecvt
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(str);
#endif
}

String GetDataFromFile1C(basic_istream<char>& file, const string& FileName)
{
	int ret = 0;

	typename Format16::file_header_t FileHeader;

	ifstream::pos_type offset = Format16::BASE_OFFSET;
	file.seekg(offset);
	file.read((char*)&FileHeader, FileHeader.Size());

	auto pElemsAddrs = ReadElementsAllocationTable<Format16>(file);
	auto ElemsNum = pElemsAddrs.size();

	for (uint32_t i = 0; i < ElemsNum; i++)
	{
		if (pElemsAddrs[i].fffffff != Format16::UNDEFINED_VALUE) {
			ElemsNum = i;
			break;
		}

		file.seekg(pElemsAddrs[i].elem_header_addr + Format16::BASE_OFFSET, ios_base::beg);

		CV8Elem elem;

		if (!SafeReadBlockData<Format16>(file, elem.header)) {
			ret = V8UNPACK_HEADER_ELEM_NOT_CORRECT;
			break;
		}
		string ElemName = elem.GetName();
		fs::path elem_path;

		if (ElemName == FileName)
		{
			if (pElemsAddrs[i].elem_data_addr != Format16::UNDEFINED_VALUE) {
				file.seekg(pElemsAddrs[i].elem_data_addr + Format16::BASE_OFFSET, ios_base::beg);
				//SmartUnpack<Format16>(file, boolInflate, elem_path);
				//auto src = prepare_smart_source<Format16>(file, true, elem_path);
				auto Result = prepare_smart_source_to_string<Format16>(file, true);
				//auto unpack_result = RecursiveUnpack(elem_path.string(), src->stream(), {}, false, false);
				return Result;
			}
			
		}
	}

	
}



String getDataFromFile1C(const string& filename_in, const string& FileName)
{
	fs::ifstream file_in(filename_in, ios_base::binary);

	return GetDataFromFile1C(file_in, FileName);

}

wstring wGetDataFromFile1C(basic_istream<char>& file, const string& FileName, const string& DataDir = "1")
{
	int ret = 0;

	typename Format16::file_header_t FileHeader;

	ifstream::pos_type offset = Format16::BASE_OFFSET;
	file.seekg(offset);
	file.read((char*)&FileHeader, FileHeader.Size());

	auto pElemsAddrs = ReadElementsAllocationTable<Format16>(file);
	auto ElemsNum = pElemsAddrs.size();

	for (uint32_t i = 0; i < ElemsNum; i++)
	{
		if (pElemsAddrs[i].fffffff != Format16::UNDEFINED_VALUE) {
			ElemsNum = i;
			break;
		}

		file.seekg(pElemsAddrs[i].elem_header_addr + Format16::BASE_OFFSET, ios_base::beg);

		CV8Elem elem;

		if (!SafeReadBlockData<Format16>(file, elem.header)) {
			ret = V8UNPACK_HEADER_ELEM_NOT_CORRECT;
			break;
		}
		string ElemName = elem.GetName();
		fs::path elem_path;

		if (ElemName == FileName)
		{
			if (pElemsAddrs[i].elem_data_addr != Format16::UNDEFINED_VALUE) {
				file.seekg(pElemsAddrs[i].elem_data_addr + Format16::BASE_OFFSET, ios_base::beg);
				auto Result = prepare_smart_source_to_string<Format16>(file, true);
				return string_to_wstring(Result);
			}

		}
	}


}


wstring wgetDataFromFile1C(const string& filename_in, const string& FileName, const string& DataFileName)
{
	fs::ifstream file_in(filename_in, ios_base::binary);

	return wGetDataFromFile1C(file_in, FileName, DataFileName);

}




int CV8File::LoadFileFromFolder(const string &dirname)
{
	typedef Format15 format;

    FileHeader.next_page_addr = format::UNDEFINED_VALUE;
    FileHeader.page_size = format::DEFAULT_PAGE_SIZE;
    FileHeader.storage_ver = 0;
    FileHeader.reserved = 0;

    Elems.clear();

    fs::directory_iterator d_end;
    fs::directory_iterator dit(dirname);

    for (; dit != d_end; ++dit) {
        fs::path current_file(dit->path());
        if (current_file.filename().string().at(0) == '.')
            continue;

		CV8Elem elem(current_file.filename().string());

		if (fs::is_directory(current_file)) {

			elem.IsV8File = true;

			elem.UnpackedData.LoadFileFromFolder(current_file.string());
			elem.Pack(false);

        } else {
            elem.IsV8File = false;

			elem.data.resize(fs::file_size(current_file));

			fs::ifstream file_in(current_file, ios_base::binary);
			file_in.read(elem.data.data(), elem.data.size());
        }

        Elems.push_back(elem);
    } // for directory_iterator

	return V8UNPACK_OK;
}

static bool
is_dot_file(const fs::path &path)
{
	return path.filename().string() == "."
		|| path.filename().string() == "..";
}

template<typename format>
static int
recursive_pack(const string &in_dirname, const string &out_filename, bool dont_deflate)
{
	uint32_t ElemsNum = 0;
	{
		fs::directory_iterator d_end;
		fs::directory_iterator dit(in_dirname);

		for (; dit != d_end; ++dit) {
			if (!is_dot_file(dit->path())) {
				++ElemsNum;
			}
		}
	}

	typename format::file_header_t FileHeader;

	//Предварительные расчеты длины заголовка таблицы содержимого TOC файла
	FileHeader.next_page_addr = format::UNDEFINED_VALUE;
	FileHeader.page_size = format::DEFAULT_PAGE_SIZE;
	FileHeader.storage_ver = 0;
	FileHeader.reserved = 0;

	auto cur_block_addr = format::file_header_t::Size() + format::block_header_t::Size();
	// Optimized: use vector instead of raw pointer for exception safety
	std::vector<typename format::elem_addr_t> pTOC(ElemsNum);
	cur_block_addr += MAX(format::elem_addr_t::Size() * ElemsNum, format::DEFAULT_PAGE_SIZE);

	fs::ofstream file_out(out_filename, ios_base::binary);
	//Открываем выходной файл контейнер на запись
	if (!file_out) {
		cout << "SaveFile. Error in creating file!" << endl;
		return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
	}

	file_out << format::placeholder;

	//Резервируем место в начале файла под заголовок и TOC
	for(unsigned i=0; i < cur_block_addr; i++) {
		file_out << '\0';
	}

	uint32_t ElemNum = 0;

	fs::directory_iterator d_end;
	fs::directory_iterator dit(in_dirname);
	for (; dit != d_end; ++dit) {

		if (is_dot_file(dit->path())) {
			continue;
		}

		fs::path current_file(dit->path());
		string name = current_file.filename().string();

		CV8Elem pElem(name);

		pTOC[ElemNum].elem_header_addr = file_out.tellp() - format::BASE_OFFSET;
		SaveBlockData<format>(file_out, pElem.header.data(), pElem.header.size(), pElem.header.size());

		pTOC[ElemNum].elem_data_addr = file_out.tellp() - format::BASE_OFFSET;
		pTOC[ElemNum].fffffff = format::UNDEFINED_VALUE;

		if (fs::is_directory(current_file)) {

			pElem.IsV8File = true;

			pElem.UnpackedData.LoadFileFromFolder(current_file.string());
			pElem.Pack(!dont_deflate);

			SaveBlockData<format>(file_out, pElem.data.data(), pElem.data.size());

		} else {

			pElem.IsV8File = false;

			auto DataSize = fs::file_size(current_file);

			fs::path p_filename(in_dirname);
			p_filename /= name;
			fs::ifstream file_in(p_filename, ios_base::binary);

			if (DataSize < SmartUnpackedLimit) {

				pElem.data.resize(DataSize);
				file_in.read(pElem.data.data(), pElem.data.size());
				pElem.Pack(!dont_deflate);
				SaveBlockData<format>(file_out, pElem.data);

			} else {

				if (dont_deflate) {
					SaveBlockData<format>(file_out, file_in, DataSize);
				} else {
					// Упаковка через промежуточный файл
					fs::path tmp_file_path = fs::temp_directory_path() / fs::unique_path();

					Deflate(file_in, tmp_file_path.string());
					SaveBlockData<format>(file_out, tmp_file_path);

					fs::remove(tmp_file_path);
				}
			}
		}

		ElemNum++;
	}

	//Записывем заголовок файла
	file_out.seekp(format::BASE_OFFSET, ios_base::beg);
	file_out.write(reinterpret_cast<const char*>(&FileHeader), format::file_header_t::Size());

	//Записываем блок TOC
	SaveBlockData<format>(file_out, (const char*) pTOC.data(), format::elem_addr_t::Size() * ElemsNum);

	cout << endl << "Build `" << out_filename << "` OK!" << endl << flush;

	return V8UNPACK_OK;
}

int BuildCfFile(const string &in_dirname, const string &out_filename, bool dont_deflate)
{
	//filename can't be empty
	if (in_dirname.empty()) {
		cerr << "Argument error - Set of `in_dirname' argument" << endl;
		return V8UNPACK_SHOW_USAGE;
	}

	if (out_filename.empty()) {
		cerr << "Argument error - Set of `out_filename' argument" << endl;
		return V8UNPACK_SHOW_USAGE;
	}

	if (!fs::exists(in_dirname)) {
		cerr << "Source directory does not exist!" << endl;
		return V8UNPACK_SOURCE_DOES_NOT_EXIST;
	}

	int compatibility = directory_container_compatibility(in_dirname);

	if (compatibility >= VersionFile::COMPATIBILITY_V80316) {
		return recursive_pack<Format16>(in_dirname, out_filename, dont_deflate);
	}

	return recursive_pack<Format15>(in_dirname, out_filename, dont_deflate);
}

int CV8Elem::Pack(bool deflate)
{
	int ret = 0;
	if (!IsV8File) {

		if (deflate) {

			char *DeflateBuffer = nullptr;
			uint32_t DeflateSize = 0;

			ret = Deflate(data.data(), &DeflateBuffer, data.size(), &DeflateSize);
			if (ret) {
				return ret;
			}

			data.resize(DeflateSize);
			memcpy(data.data(), DeflateBuffer, DeflateSize);

			delete [] DeflateBuffer;
		}

	} else {

		UnpackedData.GetData(data);
		UnpackedData.Dispose();

		if (deflate) {

			char *DeflateBuffer = nullptr;
			uint32_t DeflateSize = 0;

			ret = Deflate(data.data(), &DeflateBuffer, data.size(), &DeflateSize);
			if (ret) {
				return ret;
			}

			data.resize(DeflateSize);
			memcpy(data.data(), DeflateBuffer, DeflateSize);

			delete [] DeflateBuffer;

		}

		IsV8File = false;
	}

	return V8UNPACK_OK;
}

int CV8File::GetData(vector<char> &data)
{
	typedef Format15 format;

	auto ElemsNum = Elems.size();

	// заголовок блока и данные блока - адреса элементов с учетом минимальной страницы 512 байт
	auto NeedDataBufferSize = format::file_header_t::Size()
			+ format::block_header_t::Size()
			+ MAX(format::elem_addr_t::Size() * ElemsNum, format::DEFAULT_PAGE_SIZE);

	for (auto &elem : Elems) {

		// заголовок блока и данные блока - заголовок элемента
		NeedDataBufferSize += format::block_header_t::Size()  + elem.header.size();

		if (elem.IsV8File) {
			elem.UnpackedData.GetData(elem.data);
			elem.IsV8File = false;
		}
		NeedDataBufferSize += format::block_header_t::Size() + MAX(elem.data.size(), format::DEFAULT_PAGE_SIZE);
	}

	data.resize(NeedDataBufferSize);

	// Создаем и заполняем данные по адресам элементов
	vector<format::elem_addr_t> pTempElemsAddrs(ElemsNum);
	auto pCurrentTempElem = pTempElemsAddrs.begin();

	auto cur_block_addr = format::file_header_t::Size() + format::block_header_t::Size();
	cur_block_addr += MAX(format::elem_addr_t::Size() * ElemsNum, format::DEFAULT_PAGE_SIZE);

	for (const auto &elem : Elems) {

		pCurrentTempElem->elem_header_addr = cur_block_addr;
		cur_block_addr += format::block_header_t::Size() + elem.header.size();

		pCurrentTempElem->elem_data_addr = cur_block_addr;
		cur_block_addr += format::block_header_t::Size();
		cur_block_addr += MAX(elem.data.size(), format::DEFAULT_PAGE_SIZE);

		pCurrentTempElem->fffffff = format::UNDEFINED_VALUE;
		++pCurrentTempElem;
	}

	char *cur_pos = data.data();

	// записываем заголовок
	memcpy(cur_pos, (char*) &FileHeader, format::file_header_t::Size());
	cur_pos += format::file_header_t::Size();

	// записываем адреса элементов
	SaveBlockDataToBuffer<format>(cur_pos, (char*) pTempElemsAddrs.data(), format::elem_addr_t::Size() * ElemsNum);

	// записываем элементы (заголовок и данные)
	for (const auto &elem : Elems) {
		SaveBlockDataToBuffer<format>(cur_pos, elem.header.data(), elem.header.size(), elem.header.size());
		SaveBlockDataToBuffer<format>(cur_pos, elem.data.data(), elem.data.size());
	}

	return V8UNPACK_OK;
}

stBlockHeader stBlockHeader::create(uint32_t block_data_size, uint32_t page_size)
{
	return create(block_data_size, page_size, UNDEFINED_VALUE);
}

stBlockHeader stBlockHeader::create(uint32_t block_data_size, uint32_t page_size, uint32_t next_page_addr)
{
	stBlockHeader BlockHeader;
	BlockHeader.set_data_size(block_data_size);
	BlockHeader.set_page_size(page_size);
	BlockHeader.set_next_page_addr(next_page_addr);
	return BlockHeader;
}

stBlockHeader64 stBlockHeader64::create(uint64_t block_data_size, uint64_t page_size)
{
	return create(block_data_size, page_size, UNDEFINED_VALUE);
}

stBlockHeader64 stBlockHeader64::create(uint64_t  block_data_size, uint64_t  page_size, uint64_t next_page_addr)
{
	stBlockHeader64 BlockHeader;
	BlockHeader.set_data_size(block_data_size);
	BlockHeader.set_page_size(page_size);
	BlockHeader.set_next_page_addr(next_page_addr);
	return BlockHeader;
}

/**
 * @brief Helper function to find GUIDs in text
 * @param text Text to search for GUIDs
 * @return Vector of found GUIDs
 */
std::vector<std::string> find_guids(const std::string& text) {
	std::vector<std::string> guids;
	// Регулярное выражение для поиска GUID
	std::regex guid_pattern(R"([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})");

	std::sregex_iterator it(text.begin(), text.end(), guid_pattern);
	std::sregex_iterator end;

	while (it != end) {
		guids.push_back(it->str());
		++it;
	}

	return guids;
}

/**
 * @brief Helper function to create directory if it doesn't exist
 * @param path Path to create
 * @return true if successful
 */
bool ensure_directory(const fs::path& path) {
	if (!fs::exists(path)) {
		return fs::create_directories(path);
	}
	return true;
}

/**
 * @brief Распаковывание конфигурации 1C v8 в организованную файловую структуру по GUID метаданных
 * Функция анализирует результаты ParseToString и сохраняет данные в поддиректории по типам метаданных
 * @param config_string Входная строка с распакованными данными от ParseToString
 * @param dirname Каталог для сохранения файлов
 * @return Код возврата (0 - успешно, отрицательные значения - ошибки)
 */
int ParseToStringWithFiles(const std::string &config_string, const std::string &dirname) {
	// Create base directory
	fs::path base_dir(dirname);
	if (!ensure_directory(base_dir)) {
		cerr << "ParseToStringWithFiles: Failed to create base directory: " << dirname << endl;
		return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
	}

	// Metadata type mapping based on GUID
	std::unordered_map<std::string, std::string> guid_to_type = {
		// Subsystems
		{"37f2fa9a-b276-11d4-9435-004095e12fc7", "Subsystems"},
		// Common modules, attributes, forms, templates, commands, pictures, XDTO packages, web services, HTTP services, WS references
		{"0fe48980-252d-11d6-a3c7-0050bae0a776", "CommonModules"},
		{"15794563-ccec-41f6-a83c-ec5f7b9a5bc1", "CommonAttributes"},
		{"07ee8426-87f1-11d5-b99c-0050bae0a95d", "CommonForms"},
		{"0c89c792-16c3-11d5-b96b-0050bae0a95d", "CommonTemplates"},
		{"2f1a5187-fb0e-4b05-9489-dc5dd6412348", "CommonCommands"},
		{"7dcd43d9-aca5-4926-b549-1842e6a4e8cf", "CommonPictures"},
		{"cc9df798-7c94-4616-97d2-7aa0b7bc515e", "XDTOPackages"},
		{"8657032e-7740-4e1d-a3ba-5dd6e8afb78f", "WebServices"},
		{"0fffc09c-8f4c-47cc-b41c-8d5c5a221d79", "HTTPServices"},
		{"d26096fb-7a5d-4df9-af63-47d04771fa9b", "WSReferences"},
		// Styles, languages, interfaces
		{"3e5404af-6ef8-4c73-ad11-91bd2dfac4c8", "Styles"},
		{"9cd510ce-abfc-11d4-9434-004095e12fc7", "Languages"},
		{"39bddf6a-0c3c-452b-921c-d99cfa1c2f1b", "Interfaces"},
		// Roles, session parameters, exchange plans, filter criteria, event subscriptions, scheduled jobs, functional options
		{"09736b02-9cac-4e3f-b4f7-d3e9576ab948", "Roles"},
		{"24c43748-c938-45d0-8d14-01424a72b11e", "SessionParameters"},
		{"857c4a91-e5f4-4fac-86ec-787626f1c108", "ExchangePlans"},
		{"3e7bfcc0-067d-11d6-a3c7-0050bae0a776", "FilterCriteria"},
		{"4e828da6-0f44-4b5b-b1c0-a2b3cfe7bdcc", "EventSubscriptions"},
		{"11bdaf85-d5ad-4d91-bb24-aa0eee139052", "ScheduledJobs"},
		{"af547940-3268-434f-a3e7-e47d6d2638c3", "FunctionalOptions"},
		// Defined types, settings storages, command groups, style items, integration services
		{"c045099e-13b9-4fb6-9d50-fca00202971e", "DefinedTypes"},
		{"46b4cd97-fd13-4eaa-aba2-3bddd7699218", "SettingsStorages"},
		{"1c57eabe-7349-44b3-b1de-ebfeab67b47d", "CommandGroups"},
		{"58848766-36ea-4076-8800-e91eb49590d7", "StyleItems"},
		// Constants, documents, documents journals, enumerations
		{"0195e80c-b157-11d4-9435-004095e12fc7", "Constants"},
		{"061d872a-5787-460e-95ac-ed74ea3a3e84", "Documents"},
		{"4612bd75-71b7-4a5c-8cc5-2b0b65f9fa0d", "DocumentJournals"},
		{"f6a80749-5ad7-400b-8519-39dc5dff2542", "Enums"},
		// Catalogs, reports, data processors
		{"cf4abea6-37b2-11d4-940f-008048da11f9", "Catalogs"},
		{"631b75a0-29e2-11d6-a3c7-0050bae0a776", "Reports"},
		{"bf845118-327b-4682-b5c6-285d2a0eb296", "DataProcessors"},
		// Charts of characteristic types, accounts, calculation types
		{"82a1b659-b220-4d94-a9bd-14d757b95a48", "ChartOfCharacteristicTypes"},
		{"238e7e88-3c5f-48b2-8a3b-81ebbecb20ed", "ChartOfAccounts"},
		{"30b100d6-b29f-47ac-aec7-cb8ca8a54767", "ChartOfCalculationTypes"},
		// Information registers, accumulation registers, accounting registers, calculation registers
		{"13134201-f60b-11d5-a3c7-0050bae0a776", "InformationRegisters"},
		{"b64d9a40-1642-11d6-a3c7-0050bae0a776", "AccumulationRegisters"},
		{"2deed9b8-0056-4ffe-a473-c20a6c32a0bc", "AccountingRegisters"},
		{"f2de87a8-64e5-45eb-a22d-b3aedab050e7", "CalculationRegisters"},
		// Business processes, tasks, external data sources
		{"fcd3404e-1523-48ce-9bc0-ecdb822684a1", "BusinessProcesses"},
		{"3e63355c-1378-4953-be9b-1deb5fb6bec5", "Tasks"},
		{"5274d9fc-9c3a-4a71-8f5e-a0db8ab23de5", "ExternalDataSources"}
	};

	// Split string into sections based on "--- " delimiter
	std::istringstream stream(config_string);
	std::string line;
	std::string current_section;
	std::string current_data;
	int file_count = 0;

	while (std::getline(stream, line)) {
		if (line.find("--- ") == 0 && line.find(" ---") != std::string::npos) {
			// Save previous section if exists
			if (!current_section.empty() && !current_data.empty()) {
				// Find GUIDs in current data
				auto guids = find_guids(current_data);

				// Determine metadata type based on first found GUID
				std::string metadata_type = "Unknown";
				for (const auto& guid : guids) {
					auto it = guid_to_type.find(guid);
					if (it != guid_to_type.end()) {
						metadata_type = it->second;
						break;
					}
				}

				// Create subdirectory for metadata type
				fs::path type_dir = base_dir / metadata_type;
				if (!ensure_directory(type_dir)) {
					cerr << "ParseToStringWithFiles: Failed to create type directory: " << type_dir.string() << endl;
					continue;
				}

				// Create filename based on section name (clean it up)
				std::string filename = current_section;
				// Remove illegal filename characters
				std::replace(filename.begin(), filename.end(), '"', '_');
				std::replace(filename.begin(), filename.end(), '/', '_');
				std::replace(filename.begin(), filename.end(), '\\', '_');
				std::replace(filename.begin(), filename.end(), ':', '_');
				std::replace(filename.begin(), filename.end(), '*', '_');
				std::replace(filename.begin(), filename.end(), '?', '_');
				std::replace(filename.begin(), filename.end(), '<', '_');
				std::replace(filename.begin(), filename.end(), '>', '_');
				std::replace(filename.begin(), filename.end(), '|', '_');

				// Add file extension
				filename += ".txt";

				// Save file
				fs::path file_path = type_dir / filename;
				std::ofstream out_file(file_path.string(), std::ios::binary);
				if (out_file) {
					out_file.write(current_data.c_str(), current_data.size());
					out_file.close();
					file_count++;

					logger.log("Saved file: " + file_path.string() + " (type: " + metadata_type + ")");
				} else {
					cerr << "ParseToStringWithFiles: Failed to create file: " << file_path.string() << endl;
				}
			}

			// Start new section
			size_t start_pos = line.find("--- ") + 4;
			size_t end_pos = line.find(" ---");
			if (end_pos > start_pos) {
				current_section = line.substr(start_pos, end_pos - start_pos);
			} else {
				current_section = line.substr(start_pos);
			}
			current_data.clear();
		} else {
			// Add data to current section
			if (!current_data.empty()) {
				current_data += "\n";
			}
			current_data += line;
		}
	}

	// Save the last section
	if (!current_section.empty() && !current_data.empty()) {
		// Find GUIDs in current data
		auto guids = find_guids(current_data);

		// Determine metadata type based on first found GUID
		std::string metadata_type = "Unknown";
		for (const auto& guid : guids) {
			auto it = guid_to_type.find(guid);
			if (it != guid_to_type.end()) {
				metadata_type = it->second;
				break;
			}
		}

		// Create subdirectory for metadata type
		fs::path type_dir = base_dir / metadata_type;
		if (!ensure_directory(type_dir)) {
			cerr << "ParseToStringWithFiles: Failed to create type directory: " << type_dir.string() << endl;
		} else {
			// Create filename
			std::string filename = current_section;
			std::replace(filename.begin(), filename.end(), '"', '_');
			std::replace(filename.begin(), filename.end(), '/', '_');
			std::replace(filename.begin(), filename.end(), '\\', '_');
			std::replace(filename.begin(), filename.end(), ':', '_');
			std::replace(filename.begin(), filename.end(), '*', '_');
			std::replace(filename.begin(), filename.end(), '?', '_');
			std::replace(filename.begin(), filename.end(), '<', '_');
			std::replace(filename.begin(), filename.end(), '>', '_');
			std::replace(filename.begin(), filename.end(), '|', '_');
			filename += ".txt";

			// Save file
			fs::path file_path = type_dir / filename;
			std::ofstream out_file(file_path.string(), std::ios::binary);
			if (out_file) {
				out_file.write(current_data.c_str(), current_data.size());
				out_file.close();
				file_count++;

				logger.log("Saved file: " + file_path.string() + " (type: " + metadata_type + ")");
			} else {
				cerr << "ParseToStringWithFiles: Failed to create file: " << file_path.string() << endl;
			}
		}
	}

	logger.log("ParseToStringWithFiles completed. Saved " + std::to_string(file_count) + " files.");
	cout << "ParseToStringWithFiles: Saved " << file_count << " files in organized structure." << endl;

	return V8UNPACK_OK;
}

}
