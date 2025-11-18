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
    2014-2022       dmpas       sergey(dot)batanov(at)dmpas(dot)ru
    2019-2020       fishca      fishcaroot(at)gmail(dot)com
 */

/**
 * @file main.cpp
 * @brief Главная точка входа консольного приложения V8Unpack
 *
 * Этот файл содержит основную логику приложения для работы с файлами
 * конфигураций 1C:Предприятие v8. Поддерживает распаковку, упаковку,
 * сжатие и разбор метаданных.
 */

 /**
  * @mainpage V8Unpack - Утилита для работы с файлами конфигураций 1C v8
  *
  * V8Unpack - это кроссплатформенная консольная утилита для работы с
  * файлами конфигураций 1C:Предприятие v8 формата (.cf, .epf, .erf).
  *
  * Основные возможности:
  * - Распаковка контейнеров 1C v8 в файловую структуру
  * - Упаковка файловой структуры обратно в контейнеры 1C v8
  * - Сжатие и распаковка потоков данных (deflate/inflate)
  * - Парсинг и анализ метаданных конфигураций
  * - Пакетная обработка файлов
  *
  * @section usage Использование
  * @code{.bash}
  * # Распаковка конфигурации
  * V8Unpack -unpack config.cf unpacked_dir
  *
  * # Упаковка конфигурации
  * V8Unpack -pack unpacked_dir config_new.cf
  *
  * # Сжатие файла
  * V8Unpack -deflate input.txt output.data
  * @endcode
  *
  * @section authors Авторы
  * - Оригинальный проект: Денис Демидов (disa_da2@mail.ru)
  * - Форк и сопровождение: dmpas (sergey.batanov@dmpas.ru)
  * - Вклад: fishca, pumbaEO
  *
  * @section version Версия
  * Текущая версия: 3.0
  *
  * Изменения в версии 3.0:
  * - Оптимизированная сборка с динамическим созданием файлов
  * - Улучшенное управление памятью для больших конфигураций
  * - Расширенный парсинг метаданных
  */

// main.cpp : Defines the entry point for the console application.
 //

#include "V8File.h"
#include "version.h"
#include "tree.h"

#include "treeparser.hpp"

#include "ExactStructureBuilder.hpp"



#include <iostream>
#include <algorithm>
#include <sstream>
#include <map>
//#include <boost/regex.hpp>
#include <regex>
#include <codecvt>


#include "guids.h"
#include "common.h"
#include "parse_tree.h"
#include "TMetaObjects.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/locale.hpp>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

using namespace std;
using namespace v8unpack;
using namespace std::string_literals;

using mdForm    = mdEntity<EntityType::Form>;
using mdMoxel   = mdEntity<EntityType::Moxel>;
using mdCommand = mdEntity<EntityType::Command>;

namespace fs = boost::filesystem;

typedef int (*handler_t)(vector<string>& argv);
void read_param_file(const char* filename, vector< vector<string> >& list);
handler_t get_run_mode(const vector<string>& args, int& arg_base, bool& allow_listfile);

//const boost::wregex root_config(L"\{\d,[\d]+,(\w{8}-\w{4}-\w{4}-\w{4}-\w{12})\},\"(\S+?)\",\n?");
//std::regex root_config(R"(\{\d,[\d]+,(\w{8}-\w{4}-\w{4}-\w{4}-\w{12})\},\"(\S+?)\",\n?)");

std::string StartRegex = R"(\{)";
std::string EndRegex   = R"([^}]*\})";

//{ 09736b02-9cac-4e3f-b4f7-d3e9576ab948,0 }, // Роли
std::regex rx_roles(StartRegex + std::string(GUID_Roles) + EndRegex);

//{ 0c89c792-16c3-11d5-b96b-0050bae0a95d,0 }, // Общие макеты
std::regex rx_comtempl(StartRegex + std::string(GUID_CommonTemplates) + EndRegex);
 
//{ 0fe48980-252d-11d6-a3c7-0050bae0a776,0 }, // Общие модули
std::regex rx_commod(StartRegex + std::string(GUID_CommonModules) + EndRegex);

//{ 0fffc09c-8f4c-47cc-b41c-8d5c5a221d79,0 }, // http-сервисы
std::regex rx_http(StartRegex + std::string(GUID_HTTPServices) + EndRegex);

//{ 11bdaf85-d5ad-4d91-bb24-aa0eee139052,0 }, // Регламентные задания
std::regex rx_shedul(StartRegex + std::string(GUID_ScheduledJobs) + EndRegex);

//{ 15794563-ccec-41f6-a83c-ec5f7b9a5bc1,0 }, // Общие реквизиты
std::regex rx_comattr(StartRegex + std::string(GUID_CommonAttributes) + EndRegex);

//{ 24c43748-c938-45d0-8d14-01424a72b11e,0 }, // Параметры сеанса
std::regex rx_sesspar(StartRegex + std::string(GUID_SessionParameters) + EndRegex);

//{ 30d554db-541e-4f62-8970-a1c6dcfeb2bc,0 }, // Параметры функциональных опций
std::regex rx_funcpar(StartRegex + std::string(GUID_FunctionalOptionsParameters) + EndRegex);

//{ 37f2fa9a-b276-11d4-9435-004095e12fc7,0 }, // Подсистемы
std::regex rx_subs(StartRegex + std::string(GUID_Subsystems) + EndRegex);

//{ 39bddf6a-0c3c-452b-921c-d99cfa1c2f1b,0 }, // Интерфейсы
std::regex rx_intrf(StartRegex + std::string(GUID_Interfaces) + EndRegex);

//{ 3e5404af-6ef8-4c73-ad11-91bd2dfac4c8,0 }, // Стили
std::regex rx_style(StartRegex + std::string(GUID_Styles) + EndRegex);

//{ 3e7bfcc0-067d-11d6-a3c7-0050bae0a776,0 }, // Критерии отбора
std::regex rx_filter(StartRegex + std::string(GUID_FilterCriteria) + EndRegex);

//{ 46b4cd97-fd13-4eaa-aba2-3bddd7699218,0 }, // Хранилища настроек
std::regex rx_settings(StartRegex + std::string(GUID_SettingsStorages) + EndRegex);

//{ 4e828da6-0f44-4b5b-b1c0-a2b3cfe7bdcc,0 }, // Подписки на события
std::regex rx_events(StartRegex + std::string(GUID_EventSubscriptions) + EndRegex);

//{ 58848766-36ea-4076-8800-e91eb49590d7,0 }, // Элементы стиля
std::regex rx_styleitem(StartRegex + std::string(GUID_StyleItems) + EndRegex);

//{ 6e6dc072-b7ac-41e7-8f88-278d25b6da2a,0 },

//{ 7dcd43d9-aca5-4926-b549-1842e6a4e8cf,0 }, // Общие картинки
std::regex rx_pict(StartRegex + std::string(GUID_CommonPictures) + EndRegex);

//{ 857c4a91-e5f4-4fac-86ec-787626f1c108,0 }, // Планы обмена
std::regex rx_exchplans(StartRegex + std::string(GUID_ExchangePlans) + EndRegex);

//{ 8657032e-7740-4e1d-a3ba-5dd6e8afb78f,0 }, // Web-сервисы
std::regex rx_web(StartRegex + std::string(GUID_WebServices) + EndRegex);

//{ 9cd510ce-abfc-11d4-9434-004095e12fc7,1,154c4235 - 35f5 - 42c3 - a0d5 - 07b9ea861f14 }, // Языки
std::regex rx_lang(StartRegex + std::string(GUID_Languages) + EndRegex);

//{ af547940-3268-434f-a3e7-e47d6d2638c3,0 }, // Функциональные опции
std::regex rx_funcopt(StartRegex + std::string(GUID_FunctionalOptions) + EndRegex);

//{ c045099e-13b9-4fb6-9d50-fca00202971e,0 }, // Определяемые типы
std::regex rx_deftype(StartRegex + std::string(GUID_DefinedTypes) + EndRegex);

//{ cc9df798-7c94-4616-97d2-7aa0b7bc515e,0 }, // XDTO - пакеты
std::regex rx_xdto(StartRegex + std::string(GUID_XDTOPackages) + EndRegex);

//{ d26096fb-7a5d-4df9-af63-47d04771fa9b,0 }  // WS - ссылки
std::regex rx_ws(StartRegex + std::string(GUID_WSReferences) + EndRegex);

//{ 0195e80c-b157-11d4-9435-004095e12fc7, 0 }, // Константы
std::regex rx_const(StartRegex + std::string(GUID_Constants) + EndRegex);

//{ 061d872a-5787-460e-95ac-ed74ea3a3e84,0 },  // Документы
std::regex rx_doc(StartRegex + std::string(GUID_Documents) + EndRegex);

//{ 07ee8426-87f1-11d5-b99c-0050bae0a95d,0 },  // Общие формы
std::regex rx_comform(StartRegex + std::string(GUID_CommonForms) + EndRegex);

//{ 13134201-f60b-11d5-a3c7-0050bae0a776,0 },  // Регистры сведений
std::regex rx_infreg(StartRegex + std::string(GUID_InformationRegisters) + EndRegex);

//{ 1c57eabe-7349-44b3-b1de-ebfeab67b47d,0 },  // Группы комманд
std::regex rx_commandgroup(StartRegex + std::string(GUID_CommandGroups) + EndRegex);

//{ 2f1a5187-fb0e-4b05-9489-dc5dd6412348,0 },  // Общие команды
std::regex rx_comcom(StartRegex + std::string(GUID_CommonCommands) + EndRegex);

//{ 36a8e346-9aaa-4af9-bdbd-83be3c177977,0 },  // Нумераторы документов
std::regex rx_num(StartRegex + std::string(GUID_Numerators) + EndRegex);

//{ 4612bd75-71b7-4a5c-8cc5-2b0b65f9fa0d,0 },  // Журналы документов
std::regex rx_jorn(StartRegex + std::string(GUID_JournDocuments) + EndRegex);

//{ 631b75a0-29e2-11d6-a3c7-0050bae0a776,0 },  // Отчеты
std::regex rx_report(StartRegex + std::string(GUID_Reports) + EndRegex);

//{ 82a1b659-b220-4d94-a9bd-14d757b95a48,0 },  // План видов характеристик
std::regex rx_chart(StartRegex + std::string(GUID_ChartOfCharacteristicTypes) + EndRegex);

//{ b64d9a40-1642-11d6-a3c7-0050bae0a776,0 },  // Регистры накопления
std::regex rx_accumreg(StartRegex + std::string(GUID_AccumulationRegisters) + EndRegex);

//{ bc587f20-35d9-11d6-a3c7-0050bae0a776,0 },  // Последовательность
std::regex rx_seq(StartRegex + std::string(GUID_Sequences) + EndRegex);

//{ bf845118-327b-4682-b5c6-285d2a0eb296,0 },  // Обработки
std::regex rx_dataproc(StartRegex + std::string(GUID_DataProcessors) + EndRegex);

//{ cf4abea6-37b2-11d4-940f-008048da11f9,1,36b34b4a-30fc-40f3-be9d-59e7d39f8e95 }, // Справочники
std::regex rx_catalog(StartRegex + std::string(GUID_Catalogs) + EndRegex);	

//{ f6a80749-5ad7-400b-8519-39dc5dff2542,0 }   // Перечисления
std::regex rx_enums(StartRegex + std::string(GUID_Enums) + EndRegex);

// План счетов
std::regex rx_chart_of_acc(StartRegex + std::string(GUID_ChartsOfAccounts) + EndRegex);

// План видов расчета
std::regex rx_chart_of_calc(StartRegex + std::string(GUID_ChartsOfCalculationTypes) + EndRegex);

// Регистр бухгалтерии
std::regex rx_accreg(StartRegex + std::string(GUID_AccountingRegisters) + EndRegex);

// Регистр расчета
std::regex rx_caclreg(StartRegex + std::string(GUID_CalculationRegisters) + EndRegex);

// Бизнес процессы
std::regex rx_business(StartRegex + std::string(GUID_BusinessProcesses) + EndRegex);

// Задачи
std::regex rx_task(StartRegex + std::string(GUID_Tasks) + EndRegex);

// Внешние источники данных
std::regex rx_external(StartRegex + std::string(GUID_ExternalDataSources) + EndRegex);


/////////////////////////////////////////////////////////////////////////////////////////////

std::string wstringToString(const std::wstring& wstr) {
	// Create a converter for UTF-8 encoding
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

string WStringToString(const wstring& wstr)
{
	string str;
	str.resize(wstr.length() * 4); // UTF-8 can use up to 4 bytes per character
#ifdef _WIN32
	size_t size;
	wcstombs_s(&size, &str[0], str.size() + 1, wstr.c_str(), wstr.size());
#else
	size_t size = wcstombs(&str[0], wstr.c_str(), str.size());
	if (size == static_cast<size_t>(-1)) {
		return ""; // Conversion error
	}
#endif
	str.resize(size);
	return str;
}

std::string removeBOM(const std::string& inputStr)
{
	if (inputStr.length() >= 3 && inputStr[0] == '\xEF' && inputStr[1] == '\xBB' && inputStr[2] == '\xBF')
	{
		return inputStr.substr(3);
	}
	return inputStr;
}

std::wstring removeBOMutf16(const std::wstring& inputStr)
{
	//if (inputStr.length() >= 1 && inputStr[0] == '\xFE')
	//{
	//	return inputStr.substr(1);
	//}
	//return inputStr;
	return inputStr.substr(1);
}



std::string utf8_to_string(const char* utf8str, const locale& loc) {
	// UTF-8 to wstring
	wstring_convert<codecvt_utf8<wchar_t>> wconv;
	wstring wstr = wconv.from_bytes(utf8str);
	// wstring to string
	vector<char> buf(wstr.size());
	use_facet<ctype<wchar_t>>(loc).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());
	return string(buf.data(), buf.size());
}

std::string from_utf8(const std::string& str, const std::locale& loc = std::locale{}) {
	// Simplified version using wchar_t instead of int32_t for better compatibility
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring wstr = converter.from_bytes(str);

	std::string result(wstr.size(), '0');
	std::use_facet<std::ctype<wchar_t>>(loc).narrow(
		wstr.data(),
		wstr.data() + wstr.size(),
		'?', &result[0]);
	return result;
}

std::vector<std::string> find_guids(const std::string& text) {
	std::vector<std::string> guids;
	// Регулярное выражение для поиска GUID
	std::regex guid_pattern(R"([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})");

	//std::regex guid_pattern(R"(\{[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\}|[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})");


	std::sregex_iterator it(text.begin(), text.end(), guid_pattern);
	std::sregex_iterator end;

	while (it != end) {
		guids.push_back(it->str());
		++it;
	}

	return guids;
}



int read_ElementsAllocationTable(vector<string>& argv)
{
	//if (argv.size() < 2) {
	//	return V8UNPACK_SHOW_USAGE;
	//}

	//vector<string> filter;

	//int ret = Parse_Test(argv[0], argv[1], filter);

	std::map<std::string, std::wstring> RootTreeMetaData;

	RootTreeMetaData[GUID_Subsystems]                  = md_Subsystems;
	RootTreeMetaData[GUID_CommonModules]               = md_CommonModules;
	RootTreeMetaData[GUID_SessionParameters]           = md_SessionParameters;
	RootTreeMetaData[GUID_Roles]                       = md_Roles;
	RootTreeMetaData[GUID_CommonAttributes]            = md_CommonAttributes;
	RootTreeMetaData[GUID_ExchangePlans]               = md_ExchangePlans;
	RootTreeMetaData[GUID_FilterCriteria]              = md_FilterCriteria;
	RootTreeMetaData[GUID_EventSubscriptions]          = md_EventSubscriptions;
	RootTreeMetaData[GUID_ScheduledJobs]               = md_ScheduledJobs;
	RootTreeMetaData[GUID_FunctionalOptions]           = md_FunctionalOptions;
	RootTreeMetaData[GUID_FunctionalOptionsParameters] = md_FunctionalOptionsParameters;
	RootTreeMetaData[GUID_DefinedTypes]                = md_DefinedTypes;
	RootTreeMetaData[GUID_SettingsStorages]            = md_SettingsStorages;
	RootTreeMetaData[GUID_CommonForms]                 = md_CommonForms;
	RootTreeMetaData[GUID_CommonCommands]              = md_CommonCommands;
	RootTreeMetaData[GUID_CommandGroups]               = md_CommandGroups;
	RootTreeMetaData[GUID_Interfaces]                  = md_Interfaces;
	RootTreeMetaData[GUID_CommonTemplates]             = md_CommonTemplates;
	RootTreeMetaData[GUID_CommonPictures]              = md_CommonPictures;
	RootTreeMetaData[GUID_XDTOPackages]                = md_XDTOPackages;
	RootTreeMetaData[GUID_WebServices]                 = md_WebServices;
	RootTreeMetaData[GUID_HTTPServices]                = md_HTTPServices;
	RootTreeMetaData[GUID_WSReferences]                = md_WSReferences;
	RootTreeMetaData[GUID_StyleItems]                  = md_StyleItems;
	RootTreeMetaData[GUID_Styles]                      = md_Styles;
	RootTreeMetaData[GUID_Languages]                   = md_Languages;
	RootTreeMetaData[GUID_Catalogs]                    = md_Catalogs;
	RootTreeMetaData[GUID_Constants]                   = md_Constants;
	RootTreeMetaData[GUID_Documents]                   = md_Documents;
	RootTreeMetaData[GUID_Numerators]                  = md_DocumentNumerators;
	RootTreeMetaData[GUID_Sequences]                   = L"Последовательности";
	RootTreeMetaData[GUID_JournDocuments]              = md_DocumentJournals;
	RootTreeMetaData[GUID_Enums]                       = md_Enums;
	RootTreeMetaData[GUID_Reports]                     = md_Reports;
	RootTreeMetaData[GUID_DataProcessors]              = md_DataProcessors;
	RootTreeMetaData[GUID_ChartOfCharacteristicTypes]  = md_ChartsOfCharacteristicTypes;
	RootTreeMetaData[GUID_ChartsOfAccounts]            = md_ChartOfAccounts;
	RootTreeMetaData[GUID_ChartsOfCalculationTypes]    = md_ChartOfCalculationTypes;
	RootTreeMetaData[GUID_InformationRegisters]        = md_InformationRegisters;
	RootTreeMetaData[GUID_AccumulationRegisters]       = md_AccumulationRegisters;
	RootTreeMetaData[GUID_AccountingRegisters]         = md_AccountingRegisters;
	RootTreeMetaData[GUID_CalculationRegisters]        = md_CalculationRegisters;
	RootTreeMetaData[GUID_BusinessProcesses]           = md_BusinessProcesses;
	RootTreeMetaData[GUID_Tasks]                       = md_Tasks;
	RootTreeMetaData[GUID_ExternalDataSources]         = md_ExternalDataSources;

	std::cout << argv[0] << std::endl; // имя конфигурации
	std::cout << argv[1] << std::endl; // каталог распаковки

	String DataStrVersion = getDataFromFile1C(argv[0], "version");
	String FileDataVersion = removeBOM(DataStrVersion);
	
	// определяем ГУИД конфигурации
	wstring wRootFile = wgetDataFromFile1C(argv[0], "root", "");
	wstring RootFile = removeBOMutf16(wRootFile);
	wstring RootFileTree = L"";

	wstring ConfigTree = L"";

	std::smatch matches;
	

	//std::cout << matches[1] << std::endl;
	//std::cout << matches[2] << std::endl;


	BracketParser RootParser;
	if (RootParser.parse(RootFile)) {

		wstring RootGUID = RootParser.getRoot()->children[0]->children[1]->value;
		RootFileTree = RootParser.treeToString();

		wstring ConfigStrVersion = wgetDataFromFile1C(argv[0], wstringToString(RootGUID), "");
		wstring ConfigVersion    = removeBOMutf16(ConfigStrVersion);

		string strConfigVersion = WStringToString(ConfigVersion);

		BracketParser ConfigParser;
		if (ConfigParser.parse(ConfigVersion)) {

			ConfigTree = ConfigParser.treeToString();

			//std::regex_search(FileDataRootString, matches, root_config);
			std::regex_search(strConfigVersion, matches, rx_lang);
			std::cout << matches[0] << std::endl;
			std::cout << matches[1] << std::endl;

			std::cout << "OK!" << std::endl;
		}
		else {
			std::cout << "Invalid bracket sequence!" << std::endl;
		}

		std::cout << "OK!" << std::endl;
	}
	else {
		std::cout << "Invalid bracket sequence!" << std::endl;
	}


	return 0;
}

int OutTree()
{
	// Создание корневого узла
	TreeNode* root = new TreeNode(md_Config);

	// Добавление 
	TreeNode* child1 = new TreeNode(md_Common);
	TreeNode* child2 = new TreeNode(md_Constants);
	TreeNode* child3 = new TreeNode(md_Catalogs);
	TreeNode* child4 = new TreeNode(md_Documents);
	TreeNode* child5 = new TreeNode(md_DocumentJournals);
	TreeNode* child6 = new TreeNode(md_Enums);
	TreeNode* child7 = new TreeNode(md_Reports);
	TreeNode* child8 = new TreeNode(md_DataProcessors);
	TreeNode* child9 = new TreeNode(md_ChartsOfCharacteristicTypes);
	TreeNode* child10 = new TreeNode(md_ChartOfAccounts);
	TreeNode* child11 = new TreeNode(md_ChartOfCalculationTypes);
	TreeNode* child12 = new TreeNode(md_InformationRegisters);
	TreeNode* child13 = new TreeNode(md_AccumulationRegisters);
	TreeNode* child14 = new TreeNode(md_AccountingRegisters);
	TreeNode* child15 = new TreeNode(md_CalculationRegisters);
	TreeNode* child16 = new TreeNode(md_BusinessProcesses);
	TreeNode* child17 = new TreeNode(md_Tasks);
	TreeNode* child18 = new TreeNode(md_ExternalDataSources);

	root->addChild(child1);
	root->addChild(child2);
	root->addChild(child3);
	root->addChild(child4);
	root->addChild(child5);
	root->addChild(child6);
	root->addChild(child7);
	root->addChild(child8);
	root->addChild(child9);
	root->addChild(child10);
	root->addChild(child11);
	root->addChild(child12);
	root->addChild(child13);
	root->addChild(child14);
	root->addChild(child15);
	root->addChild(child16);
	root->addChild(child17);
	root->addChild(child18);

	child1->addChild(new TreeNode(md_Subsystems));
	child1->addChild(new TreeNode(md_CommonModules));
	child1->addChild(new TreeNode(md_SessionParameters));
	child1->addChild(new TreeNode(md_Roles));
	child1->addChild(new TreeNode(md_CommonAttributes));
	child1->addChild(new TreeNode(md_ExchangePlans));
	child1->addChild(new TreeNode(md_FilterCriteria));
	child1->addChild(new TreeNode(md_EventSubscriptions));
	child1->addChild(new TreeNode(md_ScheduledJobs));
	child1->addChild(new TreeNode(md_Bots));
	child1->addChild(new TreeNode(md_FunctionalOptions));
	child1->addChild(new TreeNode(md_FunctionalOptionsParameters));
	child1->addChild(new TreeNode(md_DefinedTypes));
	child1->addChild(new TreeNode(md_SettingsStorages));
	child1->addChild(new TreeNode(md_CommonCommands));
	child1->addChild(new TreeNode(md_CommandGroups));
	child1->addChild(new TreeNode(md_CommonForms));
	child1->addChild(new TreeNode(md_CommonTemplates));
	child1->addChild(new TreeNode(md_CommonPictures));
	child1->addChild(new TreeNode(md_XDTOPackages));
	child1->addChild(new TreeNode(md_WebServices));
	child1->addChild(new TreeNode(md_HTTPServices));
	child1->addChild(new TreeNode(md_WSReferences));
	child1->addChild(new TreeNode(md_IntegrationServices));
	child1->addChild(new TreeNode(md_StyleItems));
	child1->addChild(new TreeNode(md_Styles));
	child1->addChild(new TreeNode(md_Languages));


	// Вывод дерева
	root->print();

	delete root;

	return 0;
}

int outtree(vector<string>& argv)
{
	int ret = read_ElementsAllocationTable(argv);
	return ret;
}


/**
 * @brief Вывод справки по использованию программы
 *
 * Отображает информацию о версии программы и всех доступных командах
 * с их параметрами и кратким описанием.
 *
 * @param[in] argv Вектор аргументов командной строки (не используется)
 * @return Код возврата (всегда 0)
 */
int usage(vector<string>& argv)
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
	cout << "  -P[ARSE]   -L[IST]   listfile" << endl;
	cout << "  -S[PARSESTRING]      in_filename" << endl;
	cout << "  -B[UILD] [-N[OPACK]] in_dirname         out_filename" << endl;
	cout << "  -B[UILD] [-N[OPACK]] -L[IST] listfile" << endl;
	cout << "  -L[IST]              listfile" << endl;

	cout << "  -LISTFILES|-LF       in_filename" << endl;

	cout << "  -E[XAMPLE]" << endl;
	cout << "  -BAT" << endl;
	cout << "  -V[ERSION]" << endl;

	//OutTree();

	return 0;
}

/**
 * @brief Показать версию программы
 *
 * Выводит номер текущей версии V8Unpack в стандартный вывод,
 * включая информацию о архитектуре (32-бит или 64-бит).
 *
 * @param[in] argv Вектор аргументов командной строки (не используется)
 * @return Код возврата (всегда 0)
 */
int version(vector<string>& argv)
{
	cout << V8P_VERSION;

	// Определяем архитектуру на основе размера указателя
#if defined(__x86_64__) || defined(_M_X64)
	cout << " (x64)" << endl;
#elif defined(__i386__) || defined(_M_IX86) || defined(__WORDSIZE) && __WORDSIZE == 32
	cout << " (x32)" << endl;
#elif defined(__WORDSIZE) && __WORDSIZE == 64
	cout << " (x64)" << endl;
#else
	// Альтернативный способ определения по размеру void*
	// Для демонстрации заменим условие: показать что было бы для 32-бит
	if (sizeof(void*) == 8) {
		cout << " (x64)" << endl;
	} else if (sizeof(void*) == 4) {
		cout << " (x32)" << endl;  // Это условие выполнится в 32-битной системе
	} else {
		cout << " (unk" << (sizeof(void*) * 8) << "bit) " << endl;
	}
#endif

	return 0;
}

int inflate(vector<string>& argv)
{
	int ret = Inflate(argv[0], argv[1]);
	return ret;
}

int deflate(vector<string>& argv)
{
	int ret = Deflate(argv[0], argv[1]);
	return ret;
}

int unpack(vector<string>& argv)
{
	int ret = UnpackToFolder(argv[0], argv[1], argv[2], true);
	return ret;
}

int pack(vector<string>& argv)
{
	int ret = PackFromFolder(argv[0], argv[1]);
	return ret;
}

int parse(vector<string>& argv)
{

	if (argv.size() < 2) {
		return V8UNPACK_SHOW_USAGE;
	}

	vector<string> filter;
	for (size_t i = 2; i < argv.size(); i++) {
		if (!argv[i].empty()) {
			filter.push_back(argv[i]);
		}
	}

	return Parse(argv[0], argv[1], filter);
}

int parsetostring(vector<string>& argv)
{

	if (argv.size() < 1) {
		return V8UNPACK_SHOW_USAGE;
	}

	vector<string> filter;
	for (size_t i = 1; i < argv.size(); i++) {
		if (!argv[i].empty()) {
			filter.push_back(argv[i]);
		}
	}

	string result;
	int ret = ParseToString(argv[0], filter, result);

	if (ret == 0) {
		cout << result << endl;
	}

	return ret;
}

int list_files(vector<string>& argv)
{
	int ret = ListFiles(argv[0]);
	return ret;
}

int process_list(vector<string>& argv)
{
	if (argv.empty()) {
		return V8UNPACK_SHOW_USAGE;
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

int bat(vector<string>& argv)
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

int example(vector<string>& argv)
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

int build(vector<string>& argv)
{
	int ret = BuildCfFile(argv[0], argv[1], false);
	return ret;
}

int build_nopack(vector<string>& argv)
{
	int ret = BuildCfFile(argv[0], argv[1], true);
	return ret;
}

handler_t get_run_mode(const vector<string>& args, int& arg_base, bool& allow_listfile)
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

	if (cur_mode == "-parsestring" || cur_mode == "-s") {
		return parsetostring;
	}

	if (cur_mode == "-build" || cur_mode == "-b") {

		bool dont_pack = false;

		while ((int)args.size() > arg_base) {
			string arg2(args[arg_base]);
			transform(arg2.begin(), arg2.end(), arg2.begin(), ::tolower);
			if (arg2 == "-n" || arg2 == "-nopack") {
				arg_base++;
				dont_pack = true;
			}
			else {
				break;
			}
		}
		return dont_pack ? build_nopack : build;
	}

	allow_listfile = false;
	if (cur_mode == "-test" || cur_mode == "-t") {
		return outtree;

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

void read_param_file(const char* filename, vector< vector<string> >& list)
{
	fs::ifstream in(filename);
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
			current_line.emplace_back("");
		}

		list.push_back(current_line);
	}
}


int main(int argc, char* argv[])
{

	std::locale::global(std::locale(""));
	std::wcout.imbue(std::locale(""));

	int arg_base = 1;
	bool allow_listfile = false;
	vector<string> args;
	for (int i = 0; i < argc; i++) {
		args.emplace_back(argv[i]);
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
		cli_args.emplace_back(string(argv[i]));
	}
	while (cli_args.size() < 3) {
		// Дополним пустыми строками, чтобы избежать лишних проверок
		cli_args.emplace_back("");
	}

	int ret = handler(cli_args);
	if (ret == V8UNPACK_SHOW_USAGE) {
		usage(cli_args);
		
	}
	return ret;

}
