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
    2014-2017       dmpas       sergey(dot)batanov(at)dmpas(dot)ru
    2019-2020       fishca      fishcaroot(at)gmail(dot)com
 */

// main.cpp : Defines the entry point for the console application.
//

#include "V8File.h"
#include "V8MetaGuid.h"
#include "Parse_tree.h"
#include "version.h"
//#include "utf8.h"
#include "conv.h"
#include <string>
#include <iostream>
#include <fstream> // подключаем файлы
#include <algorithm>
#include <sstream>
#include <locale>
#include <codecvt>
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;

namespace bfs = boost::filesystem;

typedef int (*handler_t)(vector<string> &argv);
void read_param_file(const char *filename, vector< vector<string> > &list);
handler_t get_run_mode(const vector<string> &args, int &arg_base, bool &allow_listfile);

int usage(vector<string> &argv)
{
	cout << endl;
	cout << "V8Upack Version " << V8P_VERSION
		 << " Copyright (c) " << V8P_RIGHT << endl;

	cout << endl;
	cout << "Unpack, pack, deflate and inflate 1C v8 file (*.cf)" << endl;
	cout << endl;
	cout << "V8UNPACK" << endl;
	cout << "  -U[NPACK]            in_filename.cf     out_dirname [block_name]" << endl;
	cout << "  -U[NPACK]  -L[IST]   listfile" << endl;
	cout << "  -PA[CK]              in_dirname         out_filename.cf" << endl;
	cout << "  -PA[CK]    -L[IST]   listfile" << endl;
	cout << "  -I[NFLATE]           in_filename.data   out_filename" << endl;
	cout << "  -I[NFLATE] -L[IST]   listfile" << endl;
	cout << "  -D[EFLATE]           in_filename        filename.data" << endl;
	cout << "  -D[EFLATE] -L[IST]   listfile" << endl;
	cout << "  -P[ARSE]             in_filename        out_dirname [block_name1 block_name2 ...]" << endl;
	cout << "  -DD[ECOMPILE]        in_filename        out_dirname out_dirname_decompile [block_name1 block_name2 ...]" << endl;
	cout << "  -P[ARSE]   -L[IST]   listfile" << endl;
	cout << "  -B[UILD] [-N[OPACK]] in_dirname         out_filename" << endl;
	cout << "  -B[UILD] [-N[OPACK]] -L[IST] listfile" << endl;
	cout << "  -L[IST]              listfile" << endl;
	
	cout << "  -LISTFILES|-LF       in_filename" << endl;

	cout << "  -E[XAMPLE]" << endl;
	cout << "  -BAT" << endl;
	cout << "  -V[ERSION]" << endl;

	return 0;
}

int version(vector<string> &argv)
{
	cout << V8P_VERSION << endl;
	return 0;
}

int inflate(vector<string> &argv)
{
	int ret = Inflate(argv[0], argv[1]);
	return ret;
}

int deflate(vector<string> &argv)
{
	int ret = Deflate(argv[0], argv[1]);
	return ret;
}

int unpack(vector<string> &argv)
{
	int ret = CV8File::UnpackToFolder(argv[0], argv[1], argv[2], true);
	return ret;
}

int pack(vector<string> &argv)
{
	CV8File V8File;
	int ret = V8File.PackFromFolder(argv[0], argv[1]);
	return ret;
}

int parse(vector<string> &argv)
{
	int ret;
	vector<string> filter;
	for (size_t i = 2; i < argv.size(); i++) {
		if (!argv[i].empty()) {
			filter.push_back(argv[i]);
		}
	}

	boost::filesystem::ifstream src(argv[0], std::ios_base::binary);

	if (CV8File::IsV8File16(src)) {
		src.close();
		ret = CV8File::Parse16(argv[0], argv[1], filter);
	}
	else {
		ret = CV8File::Parse(argv[0], argv[1], filter);
	}

	return ret;
}

void create_tree_metadata(vector<string>& argv)
{
	bfs::path c_path = bfs::current_path();

	bfs::path alls = argv[2];

	bfs::create_directories(alls / Constants);
	bfs::create_directories(alls / Catalogs);
	bfs::create_directories(alls / Documents);
	bfs::create_directories(alls / Documents / Numerators);
	bfs::create_directories(alls / Documents / Sequences);
	bfs::create_directories(alls / JournDocuments);
	bfs::create_directories(alls / Enums);
	bfs::create_directories(alls / Reports);
	bfs::create_directories(alls / DataProcessors);
	bfs::create_directories(alls / ChartOfCharacteristicTypes);
	bfs::create_directories(alls / ChartsOfAccounts);
	bfs::create_directories(alls / ChartsOfCalculationTypes);
	bfs::create_directories(alls / InformationRegisters);
	bfs::create_directories(alls / AccumulationRegisters);
	bfs::create_directories(alls / AccountingRegisters);
	bfs::create_directories(alls / CalculationRegisters);
	bfs::create_directories(alls / BusinessProcesses);
	bfs::create_directories(alls / Tasks);
	bfs::create_directories(alls / ExternalDataSources);
	bfs::create_directories(alls / Config);


	// Ветка "общие"
	bfs::create_directories(alls / Alls);

	bfs::create_directories(alls / Alls / Subsystems);
	bfs::create_directories(alls / Alls / CommonModules);
	bfs::create_directories(alls / Alls / SessionParameters);
	bfs::create_directories(alls / Alls / Roles);
	bfs::create_directories(alls / Alls / CommonAttributes);
	bfs::create_directories(alls / Alls / ExchangePlans);
	bfs::create_directories(alls / Alls / FilterCriteria);
	bfs::create_directories(alls / Alls / EventSubscriptions);
	bfs::create_directories(alls / Alls / ScheduledJobs);
	bfs::create_directories(alls / Alls / FunctionalOptions);
	bfs::create_directories(alls / Alls / FunctionalOptionsParameters);
	bfs::create_directories(alls / Alls / DefinedTypes);
	bfs::create_directories(alls / Alls / SettingsStorages);
	bfs::create_directories(alls / Alls / CommonForms);
	bfs::create_directories(alls / Alls / CommonCommands);
	bfs::create_directories(alls / Alls / CommandGroups);
	bfs::create_directories(alls / Alls / Interfaces);
	bfs::create_directories(alls / Alls / CommonTemplates);
	bfs::create_directories(alls / Alls / CommonPictures);
	bfs::create_directories(alls / Alls / XDTOPackages);
	bfs::create_directories(alls / Alls / WebServices);
	bfs::create_directories(alls / Alls / HTTPServices);
	bfs::create_directories(alls / Alls / WSReferences);
	bfs::create_directories(alls / Alls / StyleItems);
	bfs::create_directories(alls / Alls / Styles);
	bfs::create_directories(alls / Alls / Languages);

}

string UTF8readFile(string filename)
{
	/*
	ifstream fs8(filename);
	
	if (!fs8.is_open()) {
		cout << "Could not open " << filename << endl;
		return 0;
	}

	string line;
	unsigned line_count = 1;

	while (getline(fs8, line)) {
		
		string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
		if (end_it != line.end()) {

			cout << "Invalid UTF-8 encoding detected at line " << line_count << "\n";
			cout << "This part is fine: " << string(line.begin(), end_it) << "\n";
		}

		int length = utf8::distance(line.begin(), end_it);
		line += line;
	}
	fs8.close();

	return line;
	*/
	return "";
}

std::wstring readFile(std::string filename)
{
	std::wifstream wif(filename);
	wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	std::wstringstream wss;
	wss << wif.rdbuf();
	wif.close();

	return wss.str();
}


#pragma region Windows_only 

size_t GetSizeOfFile(const std::wstring& path)
{
	
	struct _stat fileinfo;
	_wstat(path.c_str(), &fileinfo);

	return fileinfo.st_size;

}

std::wstring LoadUtf8FileToWString(const std::wstring& filename)
{
	
	std::wstring buffer;            // stores file contents
	
	FILE* f = _wfopen(filename.c_str(), L"rtS, ccs=UTF-8");

	// Failed to open file
	if (f == NULL)
	{
		// ...handle some error...
		return buffer;
	}

	size_t filesize = GetSizeOfFile(filename);

	// Read entire file contents in to memory
	if (filesize > 0)
	{
		buffer.resize(filesize);
		size_t wchars_read = fread(&(buffer.front()), sizeof(wchar_t), filesize, f);
		buffer.resize(wchars_read);
		buffer.shrink_to_fit();
	}

	fclose(f);

	return buffer;

}
#pragma endregion


int ddecompile(vector<string>& argv)
{
	int ret = 0;

	//typedef std::regex_token_iterator<const char*> Myiter;

	vector<string> filter;

	for (size_t i = 2; i < argv.size(); i++) {
	
		if (!argv[i].empty()) {

			filter.push_back(argv[i]);

		}

	}
		
	/*
	boost::filesystem::ifstream src(argv[0], std::ios_base::binary);

	if (CV8File::IsV8File16(src)) {

		src.close();
		ret = CV8File::Parse16(argv[0], argv[1], filter);

	}
	else {

		ret = CV8File::Parse(argv[0], argv[1], filter);

	}
	*/

	// добавляем раскладывание по каталогам
	// argv[0] - имя конфигурации
	// argv[1] - каталог куда распаковали конфигурацию
	// argv[2] - каталог куда будем раскладывать в соответствии с метаданными
	
	bfs::path pin = argv[1]; // - каталог куда распаковали конфигурацию
	bfs::path pout = argv[2];
	bfs::path froot = pout;
	
	if (ret == 0)
	{
		create_tree_metadata(argv);

		if (bfs::exists(pin / "root"))
		{
			bfs::copy_file(pin / "root", pout / Config / "root");

			froot /= Config;
			froot /= "root";

			static const std::regex regex("[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}");
			
			string s;                          // сюда будем класть считанные строки
			std::ifstream file(froot.c_str()); // файл из которого читаем (для линукс путь будет выглядеть по другому)

			getline(file, s);
			
			std::string sample = s;

			std::string guidConfig = "";
			std::sregex_iterator next(s.begin(), s.end(), regex);
			std::sregex_iterator end;
			while (next != end) {
				std::smatch match = *next;
				guidConfig = match.str();
				next++;
			}

			// читаем файл описания конфигурации
			std::string line;

			bfs::path cfg = argv[1];
			
			// времянка
			guidConfig = "f7a4a6dc-a20e-4f92-a5c5-3b22948f0cbd";

			cfg /= guidConfig;

			std::wstring fileCFG = readFile(cfg.string());
			
#if defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW__)

			std::string sfileCFG = conv::utf8::convert(fileCFG);

#else

			std::string sfileCFG = conv::stdlocal::convert(fileCFG);

#endif

			//sfileCFG = sfileCFG.replace(sfileCFG.begin(), sfileCFG.begin() + 1, "?", "");
			//sfileCFG = sfileCFG.substr(1, sfileCFG.length() - 1);
			vector<string> data_cfg;

			static const std::regex regex_X("\\{\\d,[\\d]+,(\\w{8}-\\w{4}-\\w{4}-\\w{4}-\\w{12})\\},""(\\S+?)"",\\n?");
			std::sregex_iterator next_X(sfileCFG.begin(), sfileCFG.end(), regex_X);
			std::sregex_iterator end_X;

			int count = 1;

			std::smatch match = *next_X;

			data_cfg.push_back(match[1].str()); // здесь располагается GUID конфигурации
			data_cfg.push_back(match[2].str()); // здесь располагается ИМЯ конфигурации

			tree* res = parse_1Ctext(sfileCFG, "");

			// пробуем обратное преобразование
			string testCFG = "";

			testCFG = outtext(res);

			delete res;
		}

		if (bfs::exists(pin / "versions"))
		{
			bfs::copy_file(pin / "versions", pout / Config / "versions");
		}
		if (bfs::exists(pin / "version"))
		{
			bfs::copy_file(pin / "version", pout / Config / "version");
		}

		
		return ret;

	}
	else
		return ret;


	
}

int list_files(vector<string> &argv)
{
	int ret = CV8File::ListFiles(argv[0]);
	return ret;
}

int process_list(vector<string> &argv)
{
	if (argv.size() < 1) {
		return SHOW_USAGE;
	}

	vector< vector<string> > commands;
	read_param_file(argv.at(0).c_str(), commands);

	for (auto command : commands) {

		int arg_base = 0;
		bool allow_listfile = false;

		handler_t handler = get_run_mode(command, arg_base, allow_listfile);

		command.erase(command.begin());
		int ret = handler(command);
		if (ret != 0) {
			// выходим по первой ошибке
			return ret;
		}
	}

	return 0;
}

int bat(vector<string> &argv)
{
	cout << "if %1 == P GOTO PACK" << endl;
	cout << "if %1 == p GOTO PACK" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << ":UNPACK" << endl;
	cout << "V8Unpack.exe -unpack      %2                              %2.unp" << endl;
	cout << "V8Unpack.exe -undeflate   %2.unp\\metadata.data            %2.unp\\metadata.data.und" << endl;
	cout << "V8Unpack.exe -unpack      %2.unp\\metadata.data.und        %2.unp\\metadata.unp" << endl;
	cout << "GOTO END" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << ":PACK" << endl;
	cout << "V8Unpack.exe -pack        %2.unp\\metadata.unp            %2.unp\\metadata_new.data.und" << endl;
	cout << "V8Unpack.exe -deflate     %2.unp\\metadata_new.data.und   %2.unp\\metadata.data" << endl;
	cout << "V8Unpack.exe -pack        %2.unp                         %2.new.cf" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << ":END" << endl;

	return 0;
}

int example(vector<string> &argv)
{
	cout << "" << endl;
	cout << "" << endl;
	cout << "UNPACK" << endl;
	cout << "V8Unpack.exe -unpack      1Cv8.cf                         1Cv8.unp" << endl;
	cout << "V8Unpack.exe -undeflate   1Cv8.unp\\metadata.data          1Cv8.unp\\metadata.data.und" << endl;
	cout << "V8Unpack.exe -unpack      1Cv8.unp\\metadata.data.und      1Cv8.unp\\metadata.unp" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << "PACK" << endl;
	cout << "V8Unpack.exe -pack        1Cv8.unp\\metadata.unp           1Cv8.unp\\metadata_new.data.und" << endl;
	cout << "V8Unpack.exe -deflate     1Cv8.unp\\metadata_new.data.und  1Cv8.unp\\metadata.data" << endl;
	cout << "V8Unpack.exe -pack        1Cv8.und                        1Cv8_new.cf" << endl;
	cout << "" << endl;
	cout << "" << endl;

	return 0;
}

int build(vector<string> &argv)
{
	const bool dont_pack = false;
	int ret = CV8File::BuildCfFile(argv[0], argv[1], dont_pack);
	return ret;
}

int build_nopack(vector<string> &argv)
{
	const bool dont_pack = true;
	int ret = CV8File::BuildCfFile(argv[0], argv[1], dont_pack);
	return ret;
}

handler_t get_run_mode(const vector<string> &args, int &arg_base, bool &allow_listfile)
{
	if (args.size() - arg_base < 1) {
		allow_listfile = false;
		return usage;
	}

	allow_listfile = true;
	string cur_mode(args[arg_base]);
	transform(cur_mode.begin(), cur_mode.end(), cur_mode.begin(), ::tolower);

	arg_base += 1;
	if (cur_mode == "-version" || cur_mode == "-v") {
		allow_listfile = false;
		return version;
	}

	if (cur_mode == "-inflate" || cur_mode == "-i" || cur_mode == "-und" || cur_mode == "-undeflate") {
		return inflate;
	}

	if (cur_mode == "-deflate" || cur_mode == "-d") {
		return deflate;
	}

	if (cur_mode == "-unpack" || cur_mode == "-u" || cur_mode == "-unp") {
		return unpack;
	}

	if (cur_mode == "-pack" || cur_mode == "-pa") {
		return pack;
	}

	if (cur_mode == "-parse" || cur_mode == "-p") {
		return parse;
	}

	if (cur_mode == "-ddecompile" || cur_mode == "-dd") {
		return ddecompile;
	}


	if (cur_mode == "-build" || cur_mode == "-b") {

		bool dont_pack = false;

		if ((int)args.size() > arg_base) {
			string arg2(args[arg_base]);
			transform(arg2.begin(), arg2.end(), arg2.begin(), ::tolower);
			if (arg2 == "-n" || arg2 == "-nopack") {
				arg_base++;
				dont_pack = true;
			}
		}
		return dont_pack ? build_nopack : build;
	}

	allow_listfile = false;
	if (cur_mode == "-bat") {
		return bat;
	}

	if (cur_mode == "-example" || cur_mode == "-e") {
		return example;
	}

	if (cur_mode == "-list" || cur_mode == "-l") {
		return process_list;
	}

	if (cur_mode == "-listfiles" || cur_mode == "-lf") {
		return list_files;
	}

	return nullptr;
}

void read_param_file(const char *filename, vector< vector<string> > &list)
{
	boost::filesystem::ifstream in(filename);
	string line;
	while (getline(in, line)) {

		vector<string> current_line;

		stringstream ss;
		ss.str(line);

		string item;
		while (getline(ss, item, ';')) {
			current_line.push_back(item);
		}

		while (current_line.size() < 5) {
			// Дополним пустыми строками, чтобы избежать лишних проверок
			current_line.push_back("");
		}

		list.push_back(current_line);
	}
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "rus");

	int arg_base = 1;
	bool allow_listfile = false;
	vector<string> args;
	for (int i = 0; i < argc; i++) {
		args.push_back(argv[i]);
	}
	handler_t handler = get_run_mode(args, arg_base, allow_listfile);

	vector<string> cli_args;

	if (handler == nullptr) {
		usage(cli_args);
		return 1;
	}

	if (allow_listfile && arg_base <= argc) {
		string a_list(argv[arg_base]);
		transform(a_list.begin(), a_list.end(), a_list.begin(), ::tolower);
		if (a_list == "-list" || a_list == "-l") {
			// Передан файл с параметрами
			vector< vector<string> > param_list;
			read_param_file(argv[arg_base + 1], param_list);

			int ret = 0;

			for (auto argv_from_file : param_list) {
				int ret1 = handler(argv_from_file);
				if (ret1 != 0 && ret == 0) {
					ret = ret1;
				}
			}

			return ret;
		}
	}

	for (int i = arg_base; i < argc; i++) {
		cli_args.push_back(string(argv[i]));
	}
	while (cli_args.size() < 3) {
		// Дополним пустыми строками, чтобы избежать лишних проверок
		cli_args.push_back("");
	}

	int ret = handler(cli_args);
	if (ret == SHOW_USAGE) {
		usage(cli_args);
	}
	return ret;
}
