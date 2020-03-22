/*----------------------------------------------------------
This Source Code Form is subject to the terms of the
Mozilla Public License, v.2.0. If a copy of the MPL
was not distributed with this file, You can obtain one
at http://mozilla.org/MPL/2.0/.
----------------------------------------------------------*/
/////////////////////////////////////////////////////////////////////////////
//
//
//	Author:			disa_da
//	E-mail:			disa_da2@mail.ru
//
//
/////////////////////////////////////////////////////////////////////////////

/**
    2014-2017       dmpas           sergey(dot)batanov(at)dmpas(dot)ru
    2019-2020       fishca          fishcaroot(at)gmail(dot)com
 */

// V8File.h: interface for the CV8File class.
//
//////////////////////////////////////////////////////////////////////

/*#if !defined(AFX_V8FILE_H__935D5C2B_70FA_45F2_BDF2_A0274A8FD60C__INCLUDED_)
#define AFX_V8FILE_H__935D5C2B_70FA_45F2_BDF2_A0274A8FD60C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000*/

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#include <assert.h>
#include <string>
#include "zlib.h"
#include <fstream>

#include <stdint.h>
#include <vector>
#include <boost/shared_array.hpp>
#include <boost/filesystem.hpp>

typedef uint32_t UINT;
typedef uint32_t DWORD;
typedef uint32_t ULONG;
typedef uint64_t ULONGLONG;

const size_t V8_DEFAULT_PAGE_SIZE = 512;
const uint32_t V8_FF_SIGNATURE = 0x7fffffff;

// Для конфигурации старше 8.3.15, без режима совместимости 
const size_t V8_DEFAULT64_PAGE_SIZE = 0x10000;
const uint64_t V8_FF64_SIGNATURE = 0xffffffffffffffff;

const size_t Offset_816 = 0x1359;  // волшебное смещение, откуда такая цифра неизвестно...

constexpr const char* GUID_ROOT = "30ffe4cc-eef2-4371-8b26-046597e37e22"; // предположительно гуид корневой конфигурации
constexpr const char* GUID_LANG = "ae3cd3da-a1c0-4b8e-8edf-4d25e853a54e"; // предположительно гуид языков конфигурации
constexpr const char* NAME_CONFIG = "BB71D8BD57664101889B95C34FEA2351";
constexpr const char* SNAME_CONFIG = "B b71 d8 b d57664101889 b95 c34 FEA2351";


#define CHUNK 16384
#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#endif


#define V8UNPACK_ERROR -50
#define V8UNPACK_NOT_V8_FILE (V8UNPACK_ERROR-1)
#define V8UNPACK_HEADER_ELEM_NOT_CORRECT (V8UNPACK_ERROR-2)


#define V8UNPACK_INFLATE_ERROR (V8UNPACK_ERROR-20)
#define V8UNPACK_INFLATE_IN_FILE_NOT_FOUND (V8UNPACK_INFLATE_ERROR-1)
#define V8UNPACK_INFLATE_OUT_FILE_NOT_CREATED (V8UNPACK_INFLATE_ERROR-2)
#define V8UNPACK_INFLATE_DATAERROR (V8UNPACK_INFLATE_ERROR-3)

#define V8UNPACK_DEFLATE_ERROR (V8UNPACK_ERROR-30)
#define V8UNPACK_DEFLATE_IN_FILE_NOT_FOUND (V8UNPACK_ERROR-1)
#define V8UNPACK_DEFLATE_OUT_FILE_NOT_CREATED (V8UNPACK_ERROR-2)

#define SHOW_USAGE -22

class CV8Elem;

class CV8File
{
public:

	// Заголовок контейнера
	struct stFileHeader
	{
		DWORD next_page_addr;     // адрес первого свободного блока
		DWORD page_size;          // размер блока по умолчанию
		DWORD storage_ver;        // поле неизвестного назначения, часто совпадает с количеством файлов в контейнере 
		DWORD reserved;           // всегда 0x00000000 ?

		static const UINT Size()
		{
			return 4 + 4 + 4 + 4;
		}
	};

	// Заголовок контейнера 8.3.16
	struct stFileHeader64
	{
		ULONGLONG next_page_addr;  // адрес первого свободного блока - 64 бита стало 
		DWORD page_size;           // размер блока по умолчанию    
		DWORD storage_ver;         // поле неизвестного назначения, часто совпадает с количеством файлов в контейнере 
		DWORD reserved;            // всегда 0x00000000 ?

		static const UINT Size()
		{
			return 8 + 4 + 4 + 4;
		}
	};

	// запись документа оглавления TOC - Table Of Content
	struct stElemAddr
	{
		DWORD elem_header_addr;      // адрес документа атрибутов файла
		DWORD elem_data_addr;        // адрес документа содержимого файлов
		DWORD fffffff;               // всегда 0x7fffffff ?

		static const UINT Size()
		{
			return 4 + 4 + 4;
		}

	};

	// запись документа оглавления TOC - Table Of Content
	// 8.3.16
	struct stElemAddr64
	{
		// каждый элемент стал 64 бита
		ULONGLONG elem_header_addr;    // адрес документа атрибутов файла
 		ULONGLONG elem_data_addr;      // адрес документа содержимого файлов
		ULONGLONG fffffff;             // всегда 0xffffffffffffffff ?

		static const UINT Size()
		{
			return 8 + 8 + 8;
		}

	};

	// Заголовок блока
	struct stBlockHeader
	{
		char EOL_0D;
		char EOL_0A;
		char data_size_hex[8];
		char space1;
		char page_size_hex[8];
		char space2;
		char next_page_addr_hex[8];
		char space3;
		char EOL2_0D;
		char EOL2_0A;

		stBlockHeader():
			EOL_0D(0xd), EOL_0A(0xa),
			space1(' '), space2(' '), space3(' '),
			EOL2_0D(0xd), EOL2_0A(0xa)
			{}

		static stBlockHeader create(uint32_t block_data_size, uint32_t page_size, uint32_t next_page_addr);

		static const UINT Size()
		{
			return 1 + 1 + 8 + 1 + 8 + 1 + 8 + 1 + 1 + 1;
		};

		bool IsCorrect() const
		{
			return EOL_0D == 0x0d
				&& EOL_0A == 0x0a
				&& space1 == 0x20
				&& space2 == 0x20
				&& space3 == 0x20
				&& EOL2_0D == 0x0d
				&& EOL2_0A == 0x0a;
		}
	};

	// Заголовок блока 8.3.16
	struct stBlockHeader64
	{
		char EOL_0D;
		char EOL_0A;
		
		// Размер всего документа - общая длина документа в байтах. Записана в виде строкового представления hex-числа. 
		char data_size_hex[16]; // 64 бита теперь
		char space1;

		// Размер текущего блока – длина тела блока в байтах. Записана также в виде строкового представления числа INT64 в hex-формате. 
		// Если документ состоит из единственного блока, то размер всего документа либо меньше, 
		// либо совпадает с размером текущего блока (что логично)
		char page_size_hex[16]; // 64 бита теперь
		char space2;

		// Адрес следующего блока – адрес по которому расположен очередной блок документа. 
		// Если адрес следующего блока равен INT64_MAX, то это значит, что следующего блока нет. 
		// Адрес следующего блока также записан в виде строкового представления числа.
		char next_page_addr_hex[16]; // 64 бита теперь
		char space3;
		char EOL2_0D;
		char EOL2_0A;

		stBlockHeader64() :
			EOL_0D(0xd), EOL_0A(0xa),
			space1(' '), space2(' '), space3(' '),
			EOL2_0D(0xd), EOL2_0A(0xa)
		{
			
		}

		static stBlockHeader64 create(ULONGLONG block_data_size, ULONGLONG page_size, ULONGLONG next_page_addr);

		static const UINT Size()
		{
			return 1 + 1 + 16 + 1 + 16 + 1 + 16 + 1 + 1 + 1; // 55 теперь
		};

		bool IsCorrect() const
		{
			return EOL_0D == 0x0d
				&& EOL_0A == 0x0a
				&& space1 == 0x20
				&& space2 == 0x20
				&& space3 == 0x20
				&& EOL2_0D == 0x0d
				&& EOL2_0A == 0x0a;
		}
	};

	int GetData(char **DataBufer, ULONG *DataBuferSize);
	int Pack();
	int LoadFileFromFolder(const std::string &dirname);
	int LoadFileFromFolder64(const std::string& dirname);
	int LoadFile(char *pFileData, ULONG FileData, bool boolInflate = true, bool UnpackWhenNeed = false);
	int SaveFileToFolder(const boost::filesystem::path &directiory) const;

	CV8File();
	CV8File(char *pFileData, bool boolUndeflate = true);
	virtual ~CV8File();

	CV8File(const CV8File &src);

	void Dispose();

	static int PackFromFolder(const std::string &dirname, const std::string &filename);
	static int BuildCfFile(const std::string &dirname, const std::string &filename, bool dont_deflate = false);
	static int BuildCfFile64(const std::string& dirname, boost::filesystem::ofstream& file_out, bool dont_deflate = false);

	static int BuildCf_old(const std::string& dirname, const std::string& filename, bool dont_deflate = false);

	static int SaveBlockData(std::basic_ostream<char> &file_out, const char *pBlockData, UINT BlockDataSize, UINT PageSize = 512);
	static int SaveBlockData64(std::basic_ostream<char>& file_out, const char* pBlockData, UINT BlockDataSize, UINT PageSize = 0x10000);
	static int SaveBlockData(std::basic_ostream<char> &file_out, std::basic_istream<char> &file_in, UINT BlockDataSize, UINT PageSize = 512);
	static int SaveBlockData64(std::basic_ostream<char>& file_out, std::basic_istream<char>& file_in, UINT BlockDataSize, UINT PageSize = 0x10000);
	static int UnpackToFolder(const std::string &filename, const std::string &dirname, const std::string &block_name, bool print_progress = false);
	
	static int UnpackToDirectoryNoLoad(
		const std::string                &directory,
		      std::basic_istream<char>   &file,
		const std::vector<std::string>   &filter,
		      bool                        boolInflate = true,
		      bool                        UnpackWhenNeed = false
	);

	static int UnpackToDirectoryNoLoad16(
		const std::string                &directory,
		      std::basic_istream<char>   &file,
		const std::vector<std::string>   &filter,
		      bool                        boolInflate = true,
		      bool                        UnpackWhenNeed = false
	);
	
	static int Parse(
		const std::string                &filename,
		const std::string                &dirname,
		const std::vector< std::string > &filter
	);

	static int Parse16(
		const std::string& filename,
		const std::string& dirname,
		const std::vector< std::string >& filter
	);

	static int ListFiles(const std::string &filename);
	static int SaveBlockDataToBuffer(char** Buffer, const char* pBlockData, UINT BlockDataSize, UINT PageSize = 512);
	static bool IsV8File(const char *pFileData, ULONG FileDataSize);
	static bool IsV8File16(const char *pFileData, ULONG FileDataSize);
	static bool IsV8File(std::basic_istream<char> &file);
	static bool IsV8File16(std::basic_istream<char>& file);
	static int ReadBlockData(char *pFileData, stBlockHeader *pBlockHeader, char *&pBlockData, UINT *BlockDataSize = NULL);
	static int ReadBlockData64(char *pFileData, stBlockHeader64 *pBlockHeader, char *&pBlockData, UINT *BlockDataSize = NULL);
	static int ReadBlockData(std::basic_istream<char> &file, stBlockHeader *pBlockHeader, char *&pBlockData, UINT *BlockDataSize = NULL);
	static int ReadBlockData64(std::basic_istream<char> &file, stBlockHeader64 *pBlockHeader, char *&pBlockData, UINT *BlockDataSize = NULL);
	static int ReadBlockData(std::basic_istream<char> &file, stBlockHeader *pBlockHeader, std::basic_ostream<char> &out, UINT *BlockDataSize = NULL);
	static int ReadBlockData64(std::basic_istream<char> &file, stBlockHeader64 *pBlockHeader, std::basic_ostream<char> &out, UINT *BlockDataSize = NULL);

private:
	stFileHeader                FileHeader;
	stFileHeader64              FileHeader64;
	std::vector<stElemAddr>     ElemsAddrs;       // TOC - оглавление собственно
	std::vector<stElemAddr64>   ElemsAddrs64;     // TOC - оглавление собственно для 8.3.16   

	std::vector<CV8Elem>        Elems;
	
	bool                        IsDataPacked;
};

class CV8Elem
{
public:

	struct stElemHeaderBegin
	{
		ULONGLONG date_creation;
		ULONGLONG date_modification;
		DWORD res; // всегда 0x000000?
		//изменяемая длина имени блока
		//после имени DWORD res; // всегда 0x000000?
		static const UINT Size()
		{
			return 8 + 8 + 4;
		};
	};

	CV8Elem(const CV8Elem &src);
	CV8Elem();
	~CV8Elem();

	int           Pack(bool deflate = true);
	int           SetName(const std::string &ElemName);
	std::string   GetName() const;

	void Dispose();

	char               *pHeader;
	UINT                HeaderSize;
	char               *pData;
	UINT                DataSize;
	CV8File             UnpackedData;
	bool                IsV8File;
	bool                NeedUnpack;

};

int Deflate(std::istream &source, std::ostream &dest);
int Inflate(std::istream &source, std::ostream &dest);

int Deflate(const std::string &in_filename, const std::string &out_filename);
int Inflate(const std::string &in_filename, const std::string &out_filename);

int Deflate(const char* in_buf, char** out_buf, ULONG in_len, ULONG* out_len);
int Inflate(const char* in_buf, char** out_buf, ULONG in_len, ULONG* out_len);

DWORD _httoi(const char *value);
ULONGLONG _httoi64(const char *value);

int CreateV8File();

