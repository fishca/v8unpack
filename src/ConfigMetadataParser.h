#ifndef ConfigMetadataParserH
#define ConfigMetadataParserH

#include <string>
#include <map>
#include <vector>
#include <regex>
#include <iostream>
#include <sstream>
#include <locale>
#include <codecvt>
#include "guids.h"

namespace v8unpack {

/**
 * @brief Структура для хранения информации о метаданных
 */
struct MetadataInfo {
	std::string guid;              // GUID типа метаданных (например, GUID_Catalogs)
	std::wstring type_name;        // Имя типа метаданных (например, "Справочники")
	int count;                     // Количество объектов данного типа
	std::vector<std::string> items; // Список GUID конкретных объектов
};

/**
 * @brief Класс для парсинга структуры конфигурации 1С
 */
class ConfigMetadataParser {
private:
	std::map<std::string, std::wstring> guid_to_name_map;
	std::map<std::string, MetadataInfo> metadata_map;

	/**
	 * @brief Инициализирует маппинг GUID → имя метаданных
	 */
	void initGuidMapping() {
		// Общие метаданные
		guid_to_name_map[GUID_Subsystems]                  = md_Subsystems;
		guid_to_name_map[GUID_CommonModules]               = md_CommonModules;
		guid_to_name_map[GUID_SessionParameters]           = md_SessionParameters;
		guid_to_name_map[GUID_Roles]                       = md_Roles;
		guid_to_name_map[GUID_CommonAttributes]            = md_CommonAttributes;
		guid_to_name_map[GUID_ExchangePlans]               = md_ExchangePlans;
		guid_to_name_map[GUID_FilterCriteria]              = md_FilterCriteria;
		guid_to_name_map[GUID_EventSubscriptions]          = md_EventSubscriptions;
		guid_to_name_map[GUID_ScheduledJobs]               = md_ScheduledJobs;
		guid_to_name_map[GUID_FunctionalOptions]           = md_FunctionalOptions;
		guid_to_name_map[GUID_FunctionalOptionsParameters] = md_FunctionalOptionsParameters;
		guid_to_name_map[GUID_DefinedTypes]                = md_DefinedTypes;
		guid_to_name_map[GUID_SettingsStorages]            = md_SettingsStorages;
		guid_to_name_map[GUID_CommonForms]                 = md_CommonForms;
		guid_to_name_map[GUID_CommonCommands]              = md_CommonCommands;
		guid_to_name_map[GUID_CommandGroups]               = md_CommandGroups;
		guid_to_name_map[GUID_Interfaces]                  = md_Interfaces;
		guid_to_name_map[GUID_CommonTemplates]             = md_CommonTemplates;
		guid_to_name_map[GUID_CommonPictures]              = md_CommonPictures;
		guid_to_name_map[GUID_XDTOPackages]                = md_XDTOPackages;
		guid_to_name_map[GUID_WebServices]                 = md_WebServices;
		guid_to_name_map[GUID_HTTPServices]                = md_HTTPServices;
		guid_to_name_map[GUID_WSReferences]                = md_WSReferences;
		guid_to_name_map[GUID_StyleItems]                  = md_StyleItems;
		guid_to_name_map[GUID_Styles]                      = md_Styles;
		guid_to_name_map[GUID_Languages]                   = md_Languages;

		// Основные объекты конфигурации
		guid_to_name_map[GUID_Catalogs]                    = md_Catalogs;
		guid_to_name_map[GUID_Constants]                   = md_Constants;
		guid_to_name_map[GUID_Documents]                   = md_Documents;
		guid_to_name_map[GUID_Numerators]                  = md_DocumentNumerators;
		guid_to_name_map[GUID_Sequences]                   = L"Последовательности";
		guid_to_name_map[GUID_JournDocuments]              = md_DocumentJournals;
		guid_to_name_map[GUID_Enums]                       = md_Enums;
		guid_to_name_map[GUID_Reports]                     = md_Reports;
		guid_to_name_map[GUID_DataProcessors]              = md_DataProcessors;
		guid_to_name_map[GUID_ChartOfCharacteristicTypes]  = md_ChartsOfCharacteristicTypes;
		guid_to_name_map[GUID_ChartsOfAccounts]            = md_ChartOfAccounts;
		guid_to_name_map[GUID_ChartsOfCalculationTypes]    = md_ChartOfCalculationTypes;
		guid_to_name_map[GUID_InformationRegisters]        = md_InformationRegisters;
		guid_to_name_map[GUID_AccumulationRegisters]       = md_AccumulationRegisters;
		guid_to_name_map[GUID_AccountingRegisters]         = md_AccountingRegisters;
		guid_to_name_map[GUID_CalculationRegisters]        = md_CalculationRegisters;
		guid_to_name_map[GUID_BusinessProcesses]           = md_BusinessProcesses;
		guid_to_name_map[GUID_Tasks]                       = md_Tasks;
		guid_to_name_map[GUID_ExternalDataSources]         = md_ExternalDataSources;
	}

	/**
	 * @brief Парсит строку формата {guid,count,item_guid1,item_guid2,...}
	 * @param line Строка для парсинга
	 * @return Структура MetadataInfo с распарсенными данными
	 */
	MetadataInfo parseMetadataLine(const std::string& line) {
		MetadataInfo info;

		// Регулярное выражение для парсинга:
		// {guid,count,item_guid1,item_guid2,...}
		// Примеры:
		// {9cd510ce-abfc-11d4-9434-004095e12fc7,1,154c4235-35f5-42c3-a0d5-07b9ea861f14}
		// {cf4abea6-37b2-11d4-940f-008048da11f9,0}

		std::regex guid_pattern(R"([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})");
		std::regex number_pattern(R"(\d+)");

		auto guid_begin = std::sregex_iterator(line.begin(), line.end(), guid_pattern);
		auto guid_end = std::sregex_iterator();

		std::vector<std::string> guids;
		for (auto i = guid_begin; i != guid_end; ++i) {
			guids.push_back(i->str());
		}

		if (guids.empty()) {
			return info;
		}

		// Первый GUID - это тип метаданных
		info.guid = guids[0];

		// Ищем число (количество объектов)
		std::smatch number_match;
		std::string after_first_guid = line.substr(line.find(guids[0]) + guids[0].length());
		if (std::regex_search(after_first_guid, number_match, number_pattern)) {
			info.count = std::stoi(number_match.str());
		}

		// Остальные GUID - это конкретные объекты
		for (size_t i = 1; i < guids.size(); ++i) {
			info.items.push_back(guids[i]);
		}

		// Получаем имя типа метаданных
		auto it = guid_to_name_map.find(info.guid);
		if (it != guid_to_name_map.end()) {
			info.type_name = it->second;
		}

		return info;
	}

public:
	ConfigMetadataParser() {
		initGuidMapping();
	}

	/**
	 * @brief Парсит содержимое файла конфигурации
	 * @param content Содержимое файла (например, d5e20966-24a3-4184-a1b0-e5aa4773c2b8.txt)
	 */
	void parseConfigFile(const std::string& content) {
		metadata_map.clear();

		// Разбиваем на строки и парсим каждую
		std::istringstream stream(content);
		std::string line;

		while (std::getline(stream, line)) {
			// Ищем строки формата {guid,...}
			if (line.find('{') != std::string::npos &&
			    line.find(',') != std::string::npos) {

				auto info = parseMetadataLine(line);
				if (!info.guid.empty()) {
					metadata_map[info.guid] = info;
				}
			}
		}
	}

	/**
	 * @brief Получает имя типа метаданных по GUID
	 * @param guid GUID типа метаданных
	 * @return Имя типа метаданных (например, "Справочники") или пустая строка
	 */
	std::wstring getMetadataTypeName(const std::string& guid) const {
		auto it = guid_to_name_map.find(guid);
		if (it != guid_to_name_map.end()) {
			return it->second;
		}
		return L"";
	}

	/**
	 * @brief Получает информацию о метаданных по GUID типа
	 * @param guid GUID типа метаданных
	 * @return Структура MetadataInfo или пустая структура
	 */
	MetadataInfo getMetadataInfo(const std::string& guid) const {
		auto it = metadata_map.find(guid);
		if (it != metadata_map.end()) {
			return it->second;
		}
		return MetadataInfo();
	}

	/**
	 * @brief Получает все метаданные из конфигурации
	 * @return Карта GUID → MetadataInfo
	 */
	const std::map<std::string, MetadataInfo>& getAllMetadata() const {
		return metadata_map;
	}

	/**
	 * @brief Генерирует имя файла для объекта метаданных
	 * @param metadata_type_guid GUID типа метаданных (например, GUID_Catalogs)
	 * @param object_guid GUID конкретного объекта
	 * @return Имя файла в формате "ТипМетаданных_ObjectGUID"
	 */
	std::string generateFileName(const std::string& metadata_type_guid,
	                             const std::string& object_guid) const {
		std::wstring type_name = getMetadataTypeName(metadata_type_guid);

		if (type_name.empty()) {
			// Если тип неизвестен, используем просто GUID
			return object_guid;
		}

		// Конвертируем широкую строку в UTF-8
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::string type_name_utf8 = converter.to_bytes(type_name);

		// Формат: ТипМетаданных_ObjectGUID
		return type_name_utf8 + "_" + object_guid;
	}

	/**
	 * @brief Выводит сводку по метаданным в консоль
	 */
	void printSummary() const {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		std::cout << "\n=== Сводка по метаданным конфигурации ===\n" << std::endl;

		for (const auto& pair : metadata_map) {
			const auto& info = pair.second;

			if (info.count > 0) {
				std::string type_name_utf8 = converter.to_bytes(info.type_name);
				std::cout << type_name_utf8 << " (" << info.guid << "): "
				         << info.count << " объект(ов)" << std::endl;

				for (const auto& item_guid : info.items) {
					std::cout << "  - " << item_guid << std::endl;
				}
			}
		}
	}
};

} // namespace v8unpack

#endif // ConfigMetadataParserH
