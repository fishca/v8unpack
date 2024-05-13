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

#pragma ide diagnostic ignored "misc-no-recursion"

#include "V8File.h"
#include "VersionFile.h"
#include "v8constants.h"
#include "v8Tree.h"
#include "tree.h"
//#include "tree_utils.h"
#include <iostream>
#include <sstream>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <utility>
#include <memory>
#include <boost/filesystem/fstream.hpp>
#include <codecvt>
#include <io.h>                             // для функции _setmode
#include <fcntl.h>                          // для константы _O_U16TEXT

#include <boost/foreach.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace v8unpack {

using namespace std;

int RecursiveUnpack(
		const string                &directory,
		      basic_istream<char>   &file,
		const vector<string>        &filter,
		      bool                   boolInflate,
		      bool                   UnpackWhenNeed
);

namespace ptree = boost::property_tree;

static bool is_dot_file(const boost::filesystem::path& path);
void get_files(vector<string>& paths, const string& current_path);

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
	stringstream ss;

	auto currentChar = header.data() + CV8Elem::stElemHeaderBegin::Size();
	for (auto j = 0; j < ElemNameLen * 2; j += 2, currentChar += 2) {
		if (*currentChar == '\0') {
			break;
		}
		ss << *currentChar;
	}

	return ss.str();
}

void CV8Elem::resizeHeader(size_t newSize)
{
	header.resize(newSize, 0);
}

int CV8Elem::SetName(const string &ElemName)
{
	uint32_t pos = CV8Elem::stElemHeaderBegin::Size();

	for (uint32_t j = 0; j < ElemName.size() * 2; j += 2, pos += 2) {
		header[pos] = ElemName[j / 2];
		header[pos + 1] = 0;
	}

	return 0;
}

void CV8Elem::Dispose()
{
	IsV8File = false;
}

std::wstring readFile(const char* filename)
{
	std::wifstream wif(filename);
	wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	std::wstringstream wss;
	wss << wif.rdbuf();

	return wss.str();
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

	const int buf_size = 1024; // TODO: Настраиваемый размер буфера
	char *pBlockData = new char[buf_size];

	read_in_bytes = 0;
	while (read_in_bytes < data_size) {

		auto page_size = pBlockHeader->page_size();
		auto next_page_addr = pBlockHeader->next_page_addr();
		auto bytes_to_read = MIN(page_size, data_size - read_in_bytes);

		uint32_t read_done = 0;
		while (read_done < bytes_to_read) {
			file.read(pBlockData, MIN(buf_size, bytes_to_read - read_done));
			uint32_t rd = file.gcount();
			out.write(pBlockData, rd);
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

	delete[] pBlockData;

	return V8UNPACK_OK;
}

template<typename format>
static size_t
DumpBlockData(basic_istream<char> &file, const typename format::block_header_t &firstBlockHeader, const boost::filesystem::path &path)
{
	boost::filesystem::ofstream out(path, ios_base::binary);
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

	vector<element_t> result;
	result.reserve(elements_count + 1);
	result.resize(elements_count);

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

	for(uint32_t i = 0; i < PageSize - BlockDataSize; i++) {
		*cur_pos = 0;
		++cur_pos;
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

	if (PageSize > BlockDataSize) {
		for (auto i = PageSize - BlockDataSize; i; i--) {
			file_out << '\0';
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

	if (PageSize > BlockDataSize) {
		for (auto i = PageSize - BlockDataSize; i; i--) {
			file_out << '\0';
		}
	}

	return V8UNPACK_OK;
}

template<typename format>
int SaveBlockData(basic_ostream<char> &file_out, boost::filesystem::path &in_file_path)
{
	auto BlockDataSize = boost::filesystem::file_size(in_file_path);
	auto PageSize = BlockDataSize;

	boost::filesystem::ifstream file_in(in_file_path, std::ios_base::binary);

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

	if (PageSize > BlockDataSize) {
		for (auto i = PageSize - BlockDataSize; i; i--) {
			file_out << '\0';
		}
	}

	return V8UNPACK_OK;
}

static int
directory_container_compatibility(const string &in_dirname)
{
	{ // распакованный файл version (после Parse)
		auto version_file_path = boost::filesystem::path(in_dirname) / "version";
		if (boost::filesystem::exists(version_file_path)) {
			boost::filesystem::ifstream version_in(version_file_path);
			auto v = VersionFile::parse(version_in);
			return v.compatibility();
		}
	}
	{ // нераспакованный файл version (после Unpack)
		auto version_file_path = boost::filesystem::path(in_dirname) / "version.data";
		if (boost::filesystem::exists(version_file_path)) {
			boost::filesystem::ifstream version_in(version_file_path);
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
// Некоторый условный предел
const size_t SmartLimit = 00 * 1024;
const size_t SmartUnpackedLimit = 20 * 1024 * 1024;

/*
	Лучше всего сжимается текст
	Берём степень сжатия текста в 99% (объём распакованных данных в 100 раз больше)
	Берём примерный порог использования памяти в 20МБ (в этот объём должы влезть распакованные данные)
	Делим 20МБ на 100 и получаем 200 КБ
	Упакованные данные размером до 200 КБ можно спокойно обрабатывать в памяти

	В дальнейшем этот показатель всё же будет вынесен в параметр командной строки
*/

class data_source_t
{
public:
	virtual istream &stream() = 0;
	virtual void save_as(const boost::filesystem::path &dest) = 0;
	virtual ~data_source_t() = default;
};

class temp_file_data_source_t : public data_source_t
{
public:
	explicit temp_file_data_source_t(const boost::filesystem::path &name) :
		path(name),
		file(name, ios_base::binary)
		{}

	istream &stream() override { return file; }

	void save_as(const boost::filesystem::path &dest) override
	{
		file.close();
		boost::system::error_code error;
		boost::filesystem::rename(path, dest, error);
	}

	~temp_file_data_source_t() override
	{
		if (file) {
			file.close();
			boost::system::error_code ec;
			boost::filesystem::remove(path, ec);
		}
	}
private:
	boost::filesystem::path path;
	boost::filesystem::ifstream file;
};

class vector_data_source_t : public data_source_t
{
public:
	explicit vector_data_source_t(vector<char> data) :
			__data(move(data)),
			__stream(__data.data(), __data.size())
	{}

	istream &stream() override { return __stream; }

	void save_as(const boost::filesystem::path &dest) override
	{
		__stream.seekg(0);
		boost::filesystem::ofstream out(dest, ios_base::binary);
		full_copy(__stream, out);
	}

	~vector_data_source_t() override = default;

private:
	vector<char> __data;
	boost::iostreams::stream<boost::iostreams::array_source> __stream;
};

template<typename format>
unique_ptr<data_source_t>
prepare_smart_source(basic_istream<char> &file, bool NeedUnpack, boost::filesystem::path &elem_path)
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
		boost::filesystem::remove(tmp_path);

		return make_unique<temp_file_data_source_t>(inf_path);
	}

	return make_unique<temp_file_data_source_t>(tmp_path);
}

template<typename format>
int SmartUnpack(basic_istream<char> &file, bool NeedUnpack, boost::filesystem::path &elem_path)
{
	auto src = prepare_smart_source<format>(file, NeedUnpack, elem_path);
	auto unpack_result = RecursiveUnpack(elem_path.string(), src->stream(), {}, false, false);
	if (unpack_result != V8UNPACK_OK) {
		src->save_as(elem_path);
	}
	return V8UNPACK_OK;
}

template<typename format>
int SmartUnpack2(basic_istream<char>& file, bool NeedUnpack, boost::filesystem::path& elem_path)
{
	auto src = prepare_smart_source<format>(file, NeedUnpack, elem_path);
	auto unpack_result = RecursiveUnpack2(elem_path.string(), src->stream(), {}, false, false);
	if (unpack_result != V8UNPACK_OK) {
		src->save_as(elem_path);
	}
	return V8UNPACK_OK;
}


static bool NameInFilter(const string &name, const vector<string> &filter)
{
	return filter.empty()
		|| find(filter.begin(), filter.end(), name) != filter.end();
}

template<typename format>
static int recursive_unpack(const string& directory, basic_istream<char>& file, const vector<string>& filter, bool boolInflate, bool UnpackWhenNeed)
{
	int ret = 0;

	boost::filesystem::path p_dir(directory);

	if (!boost::filesystem::exists(p_dir)) {
		if (!boost::filesystem::create_directory(directory)) {
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

		boost::filesystem::path elem_path= boost::filesystem::absolute(p_dir / ElemName);

		//080228 Блока данных может не быть, тогда адрес блока данных равен 0xffffffffffffffff
		if (pElemsAddrs[i].elem_data_addr != format::UNDEFINED_VALUE) {
			file.seekg(pElemsAddrs[i].elem_data_addr + format::BASE_OFFSET, ios_base::beg);
			SmartUnpack<format>(file, boolInflate, elem_path);
		}

	} // for i = ..ElemsNum

	return ret;
}

template<typename format>
static int recursive_unpack2(const string& directory, basic_istream<char>& file, const vector<string>& filter, bool boolInflate, bool UnpackWhenNeed)
{
	int ret = 0;

	boost::filesystem::path rootfile;

	boost::filesystem::path p_dir(directory);

	if (!boost::filesystem::exists(p_dir)) {
		if (!boost::filesystem::create_directory(directory)) {
			cerr << "RecursiveUnpack. Error in creating directory!" << endl;
			return ret;
		}
	}

	typename format::file_header_t FileHeader;

	ifstream::pos_type offset = format::BASE_OFFSET;
	file.seekg(offset);
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
			ret = V8UNPACK_HEADER_ELEM_NOT_CORRECT;
			break;
		}
		string ElemName = elem.GetName();

		if (!NameInFilter(ElemName, filter)) {
			continue;
		}

		boost::filesystem::path elem_path = boost::filesystem::absolute(p_dir / ElemName);

		if (ElemName == "root") {
			rootfile = elem_path;
		}

		//080228 Блока данных может не быть, тогда адрес блока данных равен 0xffffffffffffffff
		if (pElemsAddrs[i].elem_data_addr != format::UNDEFINED_VALUE) {
			file.seekg(pElemsAddrs[i].elem_data_addr + format::BASE_OFFSET, ios_base::beg);
			SmartUnpack2<format>(file, boolInflate, elem_path);
		}

	} // for i = ..ElemsNum

	return ret;
}


template<typename format>
static int list_files(boost::filesystem::ifstream &file)
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

template<typename format>
static int list_files_to_map(boost::filesystem::ifstream& file, std::unordered_map<std::string, int> *data)
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


		//cout << elem.GetName() << endl;
		(*data).insert(std::make_pair(elem.GetName(), pElemsAddrs[i].elem_header_addr + format::BASE_OFFSET));
	}

	return V8UNPACK_OK;
}


int ListFiles(const string &filename)
{
	boost::filesystem::ifstream file(filename, ios_base::binary);

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

std::string FindRoot()
{

	return std::string();
}

template<typename format>
static int unpack_to_folder(boost::filesystem::ifstream &file, const string &dirname, const string &UnpackElemWithName, bool print_progress)
{
	int ret = V8UNPACK_OK;

	boost::filesystem::path p_dir(dirname);

	if (!boost::filesystem::exists(p_dir)) {
		if (!boost::filesystem::create_directory(dirname)) {
			cerr << "RecursiveUnpack. Error in creating directory!" << endl;
			return ret;
		}
	}

	typename format::file_header_t FileHeader;

	ifstream::pos_type offset = format::BASE_OFFSET;
	file.seekg(offset);
	file.read((char*)&FileHeader, FileHeader.Size());

	if (UnpackElemWithName.empty()) {
		boost::filesystem::path filename_out(dirname);
		filename_out /= "FileHeader";
		boost::filesystem::ofstream file_out(filename_out, ios_base::binary);
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

		boost::filesystem::ofstream header_out;
		header_out.open(p_dir / (ElemName + ".header"), ios_base::binary);
		if (!header_out) {
			cerr << "UnpackToFolder. Error in creating file!" << endl;
			return -1;
		}
		header_out.write(elem.header.data(), elem.header.size());
		header_out.close();

		boost::filesystem::ofstream data_out;
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

	boost::filesystem::ifstream file(filename_in, ios_base::binary);

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
	boost::filesystem::path  header_file;
	boost::filesystem::path  data_file;
	size_t                   header_size;
	size_t                   data_size;
};

template<typename format>
static int
pack_from_folder(const boost::filesystem::path &p_curdir, boost::filesystem::ofstream &file_out)
{
	file_out << format::placeholder;
	{
		boost::filesystem::ifstream file_in(p_curdir / "FileHeader", ios_base::binary);
		full_copy(file_in, file_out);
	}

	boost::filesystem::directory_iterator d_end;
	boost::filesystem::directory_iterator it(p_curdir);

	vector<PackElementEntry> Elems;

	for (; it != d_end; it++) {
		boost::filesystem::path current_file(it->path());
		if (current_file.extension().string() == ".header") {

			PackElementEntry elem;

			elem.header_file = current_file;
			elem.header_size = boost::filesystem::file_size(current_file);

			elem.data_file = current_file.replace_extension(".data");
			elem.data_size = boost::filesystem::file_size(elem.data_file);

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

		boost::filesystem::ifstream header_in(elem.header_file, ios_base::binary);
		SaveBlockData<format>(file_out, header_in, elem.header_size, elem.header_size);

		boost::filesystem::ifstream data_in(elem.data_file, ios_base::binary);
		SaveBlockData<format>(file_out, data_in, elem.data_size, V8_DEFAULT_PAGE_SIZE);
	}

	file_out.close();

	return V8UNPACK_OK;
}

int PackFromFolder(const string &dirname, const string &filename_out)
{
	boost::filesystem::path p_curdir(dirname);
	boost::filesystem::ofstream file_out(filename_out, ios_base::binary);
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
		return V8UNPACK_NOT_V8_FILE;
	}

	if (IsV8File16(file)) {
		return recursive_unpack<Format16>(directory, file, filter, boolInflate, UnpackWhenNeed);
	}

	return recursive_unpack<Format15>(directory, file, filter, boolInflate, UnpackWhenNeed);
}

int RecursiveUnpack2(const string& directory, basic_istream<char>& file, const vector<string>& filter, bool boolInflate, bool UnpackWhenNeed)
{
	if (!IsV8File(file)) {
		return V8UNPACK_NOT_V8_FILE;
	}

	if (IsV8File16(file)) {
		return recursive_unpack2<Format16>(directory, file, filter, boolInflate, UnpackWhenNeed);
	}

	return recursive_unpack2<Format15>(directory, file, filter, boolInflate, UnpackWhenNeed);
}


int Parse(const string &filename_in, const string &dirname, const vector< string > &filter)
{
    int ret = 0;

    boost::filesystem::ifstream file_in(filename_in, ios_base::binary);

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

void get_files(vector<string>& paths, const string& current_path)
{
	boost::filesystem::recursive_directory_iterator d_end;
	boost::filesystem::recursive_directory_iterator dit(current_path);

	for (; dit != d_end; ++dit) {
		if (!is_dot_file(dit->path())) {
			paths.push_back(dit->path().string());
		}

	}
}

int ParseFolder(const string& filename_in, const string& dirname, const vector< string >& filter)
{
	int ret = 0;

	boost::filesystem::ifstream file_in(filename_in, ios_base::binary);

	if (!file_in) {
		cerr << "Parse. `" << filename_in << "` not found!" << endl;
		return -1;
	}

	std::unordered_map<std::string, int> data;

	if (IsV8File16(file_in)) {
		ret = list_files_to_map<Format16>(file_in, &data);
	}
	else {
		ret = list_files_to_map<Format15>(file_in, &data);
	}
	
	//boost::filesystem::path entries_file_path(filename_in);
	//boost::filesystem::path parent_path = entries_file_path.parent_path();

	ret = RecursiveUnpack2(dirname, file_in, filter, true, false);

	if (ret == V8UNPACK_NOT_V8_FILE) {
		cerr << "Parse. `" << filename_in << "` is not V8 file!" << endl;
		return ret;
	}

	boost::filesystem::path root_path(dirname + "\\" + "root");
	

	std::wstring wData = readFile(root_path.string().c_str());

	v8Tree* tt;

	tt = parse_1Ctext(wData);

	std::wstring wroot_guid = L"";
	// находим GUID конфигурации
	if (tt)
		wroot_guid = tt->get_first()->get_first()->get_next()->get_value();


	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	
	std::string root_guid = converter.to_bytes(wroot_guid);

	boost::filesystem::path root_file(dirname + "\\" + root_guid);

	std::wstring wDataRoot = readFile(root_file.string().c_str());

	delete tt;

	tt = parse_1Ctext(wDataRoot);

	v8Tree* lang_tree = tt->get_subnode(L"37f2fa9a-b276-11d4-9435-004095e12fc7");
	
	if (tt)
		delete tt;

	//tt->
	//    first->first->
	//    next->next->next->
	//    first->next->
	//    first->next->
	//    first->next->
	//    first->next->
	//    first->next->
	//    next->value
	/*
	std::wstring ConfigName = L"";
	try
	{
		if (tt)
		ConfigName = tt->get_first()->get_first()->
			get_next()->get_next()->get_next()->
			get_first()->get_next()->
			get_first()->get_next()->
			get_first()->get_next()->
			get_first()->get_next()->
			get_first()->get_next()->
			get_next()->get_value();

	}
	catch (const std::exception&)
	{

	}
	*/


	// tt->
	// first->first->
	// next->next->next->
	// first->next->
	// first->next->
	// first->next->
	// first->next->
	// first->next->
	// next->next->
	// last->value
	std::wstring ConfigNameSynonym = L"";


	v8Tree* ttsyn = nullptr;
	/*
	try
	{
		ttsyn = tt->get_first()->get_first()->
			get_next()->get_next()->get_next()->
			get_first()->get_next()->
			get_first()->get_next()->
			get_first()->get_next()->
			get_first()->get_next()->
			get_first()->get_next()->
			get_next()->get_next()->
			get_last();
		if (ttsyn)
			ConfigNameSynonym = ttsyn->get_value();

	}
	catch (const std::exception&)
	{

	}

	*/

	if (ttsyn)
		delete ttsyn;

	//std::string sConfigName = converter.to_bytes(ConfigName);
	std::string sConfigNameSynonym = converter.to_bytes(ConfigNameSynonym);

	std::wstring wfilename_in = converter.from_bytes(filename_in);


	/*
	uint32_t ElemsNum = 0;
	{
		boost::filesystem::directory_iterator d_end;
		boost::filesystem::directory_iterator dit(dirname);

		for (; dit != d_end; ++dit) {
			if (!is_dot_file(dit->path())) {
				++ElemsNum;
				cout << "Parse `" << dit->path() << "`: ok" << endl << flush;
			}
		}
	}
	*/
	vector<string> paths;

	get_files(paths, dirname);

	for  (auto file : paths)
	{
		cout << "Parse `" << file << "`: ok" << endl << flush;
	}


	boost::filesystem::path rroot_path(dirname + "\\" + "version");


	std::wstring wVersion = readFile(rroot_path.string().c_str());

	std::wstring wVersionNew = L"";

	v8Tree* TreeVersion = parse_1Ctext(wVersion);

	TreeVersion->outtext(wVersionNew);

	Tree<std::string> tree{ "Root" };
	tree.GetRoot()->AppendChild("Left Child");
	tree.GetRoot()->AppendChild("Right Child");
	tree.GetRoot()->GetFirstChild()->AppendChild("First Grandchild of Left Child");
	tree.GetRoot()->GetFirstChild()->AppendChild("Second Grandchild of Left Child");

	ptree::ptree heroTree;

	//heroTree.put("Name", "John");
	//heroTree.put("Exp", 150);
	//heroTree.put("Inventory.Weapon", "Blue Sword");
	//heroTree.put("Inventory.Money", 3000);

	//XML-код для парсинга
	std::string xmlCode =

		"<debug>\
		<filename>debug.log</filename>\
			<modules><module>Finance</module>\
			<module>Admin</module>\
			<module>HR</module>\
		</modules>\
		<level>2</level>\
		</debug>";


		//"<ButtonList>\
	 //        <Button>B1</Button>\
	 //        <Button>B2</Button>\
	 //   </ButtonList>";

	//Создаем поток
	std::stringstream stream(xmlCode);

	try
	{
		boost::property_tree::ptree propertyTree;

		//Читаем XML
		boost::property_tree::read_xml(stream, propertyTree);

		//Читаем значения:
		BOOST_FOREACH(auto & v, propertyTree)
		{
			std::cout << "Button is " << v.second.get<std::string>("") << std::endl;
		}


		//Добавляем пару значений
		propertyTree.put("ButtonList.Button", "B3");
		propertyTree.put("ButtonList.Button", "B4");

		std::stringstream output_stream;

		//Записываем в другой поток
		boost::property_tree::write_xml(output_stream, propertyTree);

		//Получаем XML из потока
		//std::string outputXmlCode = output_stream;

	}
	catch (boost::property_tree::xml_parser_error)
	{

		std::cout << "XML parser error!" << std::endl;

		throw;
	}

	//TreeUtilities::OutputToDotFile(tree, "D:\\work\\cpp\\v8unpack_ms\\ms_v8unpack\\Debug\\tree.txt");

	//v8Tree* Config1 = tt->get_first()->get_first()->get_next()->get_next()->get_next()->get_next()->get_first();
	//Config1->next->first->next->first->next->next->next
	//v8Tree* cfg = Config1->get_next()->get_first()->get_next()->get_first()->get_next()->get_next()->get_next();

	//v8Tree* cfg_subnode1 = cfg->get_subnode(L"061d872a-5787-460e-95ac-ed74ea3a3e84");
	//v8Tree* cfg_subnode1 = Config1->get_subnode(L"061d872a-5787-460e-95ac-ed74ea3a3e84");


	// переключение стандартного потока вывода в формат Юникода
	_setmode(_fileno(stdout), _O_U16TEXT);

	//cout << "Parse `" << root_guid.c_str() << "`: ok" << endl << flush;
	//wcout << "Parse root guid: `" << wroot_guid << "`: ok" << endl << flush;
	//wcout << "Parse `" << ConfigName + L" ( " + ConfigNameSynonym + L" )" << "`: ok" << endl << flush;
	//wcout << "Parse `" << ConfigNameSynonym << "`: ok" << endl << flush;
	wcout << "Parse `" << wfilename_in << "`: ok" << endl << flush;


	delete TreeVersion;
	//delete cfg_subnode1;
	//delete cfg;
	//delete Config1;
	//delete tt;

	return ret;
}


int CV8File::LoadFileFromFolder(const string &dirname)
{
	typedef Format15 format;

    FileHeader.next_page_addr = format::UNDEFINED_VALUE;
    FileHeader.page_size = format::DEFAULT_PAGE_SIZE;
    FileHeader.storage_ver = 0;
    FileHeader.reserved = 0;

    Elems.clear();

    boost::filesystem::directory_iterator d_end;
    boost::filesystem::directory_iterator dit(dirname);

    for (; dit != d_end; ++dit) {
        boost::filesystem::path current_file(dit->path());
        if (current_file.filename().string().at(0) == '.')
            continue;

		CV8Elem elem(current_file.filename().string());

		if (boost::filesystem::is_directory(current_file)) {

			elem.IsV8File = true;

			elem.UnpackedData.LoadFileFromFolder(current_file.string());
			elem.Pack(false);

        } else {
            elem.IsV8File = false;

			elem.data.resize(boost::filesystem::file_size(current_file));

			boost::filesystem::ifstream file_in(current_file, ios_base::binary);
			file_in.read(elem.data.data(), elem.data.size());
        }

        Elems.push_back(elem);
    } // for directory_iterator

	return V8UNPACK_OK;
}

static bool is_dot_file(const boost::filesystem::path &path)
{
	return path.filename().string() == "." || path.filename().string() == "..";
}

template<typename format>
static int recursive_pack(const string &in_dirname, const string &out_filename, bool dont_deflate)
{
	uint32_t ElemsNum = 0;
	{
		boost::filesystem::directory_iterator d_end;
		boost::filesystem::directory_iterator dit(in_dirname);

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
	typename format::elem_addr_t *pTOC;
	pTOC = new typename format::elem_addr_t[ElemsNum];
	cur_block_addr += MAX(format::elem_addr_t::Size() * ElemsNum, format::DEFAULT_PAGE_SIZE);

	boost::filesystem::ofstream file_out(out_filename, ios_base::binary);
	//Открываем выходной файл контейнер на запись
	if (!file_out) {
		delete [] pTOC;
		cout << "SaveFile. Error in creating file!" << endl;
		return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
	}

	file_out << format::placeholder;

	//Резервируем место в начале файла под заголовок и TOC
	for(unsigned i=0; i < cur_block_addr; i++) {
		file_out << '\0';
	}

	uint32_t ElemNum = 0;

	boost::filesystem::directory_iterator d_end;
	boost::filesystem::directory_iterator dit(in_dirname);
	for (; dit != d_end; ++dit) {

		if (is_dot_file(dit->path())) {
			continue;
		}

		boost::filesystem::path current_file(dit->path());
		string name = current_file.filename().string();

		CV8Elem pElem(name);

		pTOC[ElemNum].elem_header_addr = file_out.tellp() - format::BASE_OFFSET;
		SaveBlockData<format>(file_out, pElem.header.data(), pElem.header.size(), pElem.header.size());

		pTOC[ElemNum].elem_data_addr = file_out.tellp() - format::BASE_OFFSET;
		pTOC[ElemNum].fffffff = format::UNDEFINED_VALUE;

		if (boost::filesystem::is_directory(current_file)) {

			pElem.IsV8File = true;

			pElem.UnpackedData.LoadFileFromFolder(current_file.string());
			pElem.Pack(!dont_deflate);

			SaveBlockData<format>(file_out, pElem.data.data(), pElem.data.size());

		} else {

			pElem.IsV8File = false;

			auto DataSize = boost::filesystem::file_size(current_file);

			boost::filesystem::path p_filename(in_dirname);
			p_filename /= name;
			boost::filesystem::ifstream file_in(p_filename, ios_base::binary);

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
					boost::filesystem::path tmp_file_path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();

					Deflate(file_in, tmp_file_path.string());
					SaveBlockData<format>(file_out, tmp_file_path);

					boost::filesystem::remove(tmp_file_path);
				}
			}
		}

		ElemNum++;
	}

	//Записывем заголовок файла
	file_out.seekp(format::BASE_OFFSET, ios_base::beg);
	file_out.write(reinterpret_cast<const char*>(&FileHeader), format::file_header_t::Size());

	//Записываем блок TOC
	SaveBlockData<format>(file_out, (const char*) pTOC, format::elem_addr_t::Size() * ElemsNum);

	delete [] pTOC;

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

	if (!boost::filesystem::exists(in_dirname)) {
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
	for (auto elem : Elems) {
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

}
#pragma clang diagnostic pop