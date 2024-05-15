#pragma once

#include <string>

using namespace std;


// План счетов
const uint32_t CAccntMoxelExtObj          = 0;  // макет объекта ПланСчетов 
const uint32_t CAccntFormHelpExtObj       = 1;  // предопределенные данные объекта ПланСчетов 
const uint32_t CAccntHelpExtObj           = 5;  // справка объекта ПланСчетов 
const uint32_t CAccntPredefinedDataExtObj = 9;  // справка формы объекта ПланСчетов 
const uint32_t CAccntModuleExtObj         = 21; // модуль объекта ПланСчетов 

// БизнесПроцесс
const uint32_t CBPTaskMaketExtObj      = 0; //{ макет объекта БизнесЗадача }
const uint32_t CBPTaskFormHelpExtObj   = 1; //{ модуль объекта БизнесЗадача }
const uint32_t CBPTaskHelpExtObj       = 5; //{ справка объекта БизнесЗадача }
const uint32_t CBPTaskModuleExtObj     = 6; //{ модуль менеджера БизнесЗадача }
const uint32_t CBPTaskMnrgModuleExtObj = 7; //{ справка формы объекта БизнесЗадача }

const uint32_t CModuleExtObj = 6; //{ модуль объекта БизнесЗадача }

const uint32_t CCommonModuleModuleExtObj = 0; //{ модуль Общего модуля }

// Регламентное задание
const uint32_t CPlanedTaskPlanExtObj = 0; //{ расписание объекта Регламентное задание }

// План обмена
const uint32_t CDataExchHelpExtObj       = 0; //{ справка объекта ПланОбмена }
const uint32_t CDataExchContentExtObj    = 1; //{ состав ПланаОбмена }
const uint32_t CDataExchModuleExtObj     = 2; //{ модуль ПланаОбмена }
const uint32_t CDataExchMngrModuleExtObj = 3; //{ модуль менеджера ПланаОбмена }

// Регистр накопления
const uint32_t CAccumRegHelpExtObj          = 0; //{ справочная информация РегистраНакопления }
const uint32_t CAccumRegModuleExtObj        = 1; //{ модуль РегистраНакопления }
const uint32_t CAccumRegManagerModuleExtObj = 2; //{ модуль менеджера РегистраНакопления }
const uint32_t CAccumRegAsmExtObj           = 3; //{ Агрегат РегистраНакопления }

// Константы
const uint32_t CConstModuleExtObj    = 0; //{ модуль Константы }
const uint32_t CConstModuleMgrExtObj = 1; //{ модуль менеджера значения Константы }

// Регистр сведений
const uint32_t CInfoRegHelpExtObj          = 0; //{ справочная информация РегистраСведений }
const uint32_t CInfoRegModuleExtObj        = 1; //{ модуль РегистраСведений }
const uint32_t CInfoRegManagerModuleExtObj = 2; //{ модуль менеджера РегистраСведений }

// Обработки
const uint32_t CProcessingModuleExtObj        = 0; //{ модуль Обработки }
const uint32_t CProcessingHelpExtObj          = 1; //{ справка Обработки }
const uint32_t CProcessingManagerModuleExtObj = 2; //{ модуль менеджера Обработки }


// Справочники
const uint32_t CRefModuleExtObj         = 0;  //{ модуль справочника }
const uint32_t CRefHelpExtObj           = 1;  //{ справка Справочника }
const uint32_t CRefCommandModuleExtObj  = 2;  //{ модуль команды Справочника }
const uint32_t CRefManagerModuleExtObj  = 3;  //{ модуль менеджера справочника }
const uint32_t CRefPredefinedDataExtObj = 29; //{ предопределенные данные Справочника }

// Отчеты
const uint32_t CReportModuleExtObj        = 0; //{ модуль Отчета }
const uint32_t CReportHelpExtObj          = 1; //{ справка Отчета }
const uint32_t CReportManagerModuleExtObj = 2; //{ модуль менеджера Отчета }

// Последовательность
const uint32_t CSequenceModule = 0; //{ модуль Последовательности }

// Документы
const uint32_t CDocModuleExtObj     = 0; //{ модуль Документа }
const uint32_t CDocHelpExtObj       = 1; //{ справка Документа }
const uint32_t CDocMngrModuleExtObj = 2; //{ модуль менеджера Документа }

// Виды расчета
const uint32_t CCalcKindModuleExtObj         = 0; //{ модуль ВидаРасчета }
const uint32_t CCalcKindHelpExtObj           = 1; //{ справка ВидаРасчета }
const uint32_t CCalcKindPredefinedDataExtObj = 2; //{ предопределенные данные ВидаРасчета }

// Регистр расчета
const uint32_t CCalcRegModuleExtObj        = 40000; //{ модуль РегистраРасчета } // значение было 0, может это не правильно
const uint32_t CCalcRegManagerModuleExtObj = 50000; //{ модуль менеджера регистра расчетов }
const uint32_t CCalcRegHelpExtObj          = 0;     //{ справка РегистраРасчета }
const uint32_t CRecalcModuleExtObj         = 0;     //{ модуль Перерасчета }

// Бизнес-процессы
const uint32_t CBPObjMaketExtObj      = 0; //{ макет объекта БизнессПроцесс }
const uint32_t CBPObjHelpObj          = 1; //{ справочная информация объекта БизнессПроцесс }
const uint32_t CBPObjHelpExtObj       = 5; //{ справочная информация объекта БизнессПроцесс }
const uint32_t CBPObjModuleExtObj     = 6; //{ модуль объекта БизнессПроцесс }
const uint32_t CBPObjFlowchartExtObj  = 7; //{ карта БП объекта БизнессПроцесс }
const uint32_t CBPObjMngrModuleExtObj = 8; //{ модуль менеджера объекта БизнессПроцесс }

// Регистр бухгалтерии
const uint32_t CAccntRegManagedFormObj = 0; //{ управляемая форма РегистраБухучета }
const uint32_t CAccntRegHelpExtObj2    = 1; //{ Справка РегистраБухучета }
const uint32_t CAccntRegHelpExtObj     = 5; //{ Справка РегистраБухучета }
const uint32_t CAccntRegModuleExtObj   = 6; //{ модуль РегистраБухучета }

// Журнал
const uint32_t CJournalsHelpExtObj          = 0;     //{ Справка Журнала }
const uint32_t CJournalsFormHelpExtObj      = 1;     //{ Справка ФормыЖурнала }
const uint32_t CJournalsManagerModuleExtObj = 20000; //{ Модуль менеджера журнала }

const uint32_t CCalcPlanModuleExtObj         = 0;     //{ модуль ПланаРасчета }
const uint32_t CCalcPlanHelpExtObj           = 1;     //{ справка ПланаРасчета }
const uint32_t CCalcPlanPredefinedDataExtObj = 2;     //{ предопределенные данные ПланаРасчета }
const uint32_t CCalcPlanManagerModuleExtObj  = 30000; //{ модуль менеджера Плана видов Расчета }

const uint32_t CGlobalFormsHelpExtObj = 1; //{ справка ГлобальнойФормы }
const uint32_t CGlobalFormsFormExtObj = 0; //{ форма ГлобальнойФормы }

const uint32_t CCommonPictData           = 0; //{ данные общей картинки }
const uint32_t CSubsystemHelpExtObj      = 0; //
const uint32_t CSubsystemInterfaceExtObj = 1; //{ справка Подсистемы }
const uint32_t CCommonTableMXLExtObj     = 0; //{ макет объекта ОбщаяТаблица }
const uint32_t CCommonTableHelpExtObj    = 1; //{ справка объекта ОбщаяТаблица }

// План видов характеристик
const uint32_t CChPlanMoxelExtObj          = 0;     //{ макет ПланаВидовХарактеристик }
const uint32_t CChPlanFormHelpExtObj       = 1;     //{ справка объекта ФормаПланаВидовХарактеристик }
const uint32_t CChPlanHelpExtObj           = 5;     //{ справка объекта ПланВидовХарактеристик }
const uint32_t CChPlanPredefinedDataExtObj = 7;     //{ предопределенные данные ПланВидовХарактеристик }
const uint32_t CChPlanModuleExtObj         = 60000; //{ модуль ПланаВидовХарактеристик }
const uint32_t CChPlanManagerModuleExtObj  = 70000; //{ модуль менеджера ПланаВидовХарактеристик }

// Роли
const uint32_t CRoleDataExtObj = 0; //{ данные объекта Роль }

// Стили
const uint32_t CStyleDataExtObj = 0; //{ данные объекта Стиль }

// Интерфейсы
const uint32_t CInterfaceDataExtObj = 0; //{ данные объекта Интерфейс }

// Критерий отбора
const uint32_t CSelCriterionsFormExtObj     = 0;     //{ форма критерия отбора }
const uint32_t CSelCriterionsFormHelpExtObj = 1;     //{ справка ФормыКритерияОтбора }
const uint32_t CSelCriterionsModuleExtObj   = 80000; //{ модуль менеджера критерия отбора }


// Конфигурация
const uint32_t CConfigLogoExtObj    = 1; //{ Логотип конфигурации }
const uint32_t CConfigSplashExtObj  = 2; //{ Заставка конфигурации }
const uint32_t CConfigHelpExtObj    = 3; //{ Справка конфигурации }
const uint32_t CConfigExtInfoExtObj = 4; //{ Свойства конфигурации }
// Подсистемы
const uint32_t CUserModuleExtObj = 7; //{ Все подсистемы конфигурации }

// Глобальный модуль (обычное приложение)
const uint32_t CGlobalModuleExtObj             = 0; //{ Глобальный модуль конфигурации }
const uint32_t CExternalConnectionModuleExtObj = 5; //{ Модуль внешнего соединения }


const uint32_t CMoxelDataExtObj = 0; //{ Модуль внешнего соединения }


//CExternalConnectionModuleExtObj2 = ".b"; { Модуль внешнего соединения 2} // на самом деле это интерфейс клиентского приложения




const uint32_t CSettStorageFofmExtObj = 0; //{ Справка ХранилищаНастроек }
const uint32_t CSettStorageHelpExtObj = 1; //{ Справка ХранилищаНастроек }

const uint32_t CTaskDescktopIntfExtObj          = 9;     //{ Форма Хранилища Настроек }
const uint32_t CTaskDescktopAreaExtObj          = 8;     //{ Командный интерфейс рабочего стола }
const wstring CConfigAllSubsystemsExtObj        = L".a";  //{ Модуль сеанса пользователя }  // HEX(A) -> Dec(10)
const uint32_t CExternalConnectionModuleExtObj2 = 11;    //{ Модуль внешнего соединения 2 } // HEX(B) -> Dec(11)
const uint32_t CPictureOsnRazdExtObj            = 12;    //{ Картинка основного раздела }  // HEX(C) -> Dec(12)
const uint32_t CSignMobileClient                = 13;    //{ Подпись мобильного клиента }  // HEX(D) -> Dec(13)
const wstring CSignMobileClientExtObj           = L".d";

//CExternalConnectionModuleExtObj3 = 13;   { Неизвестное расширение }      // HEX(D) -> Dec(13)
//CExternalConnectionModuleExtObj4 = ".d"; { Неизвестное расширение }      // HEX(D) -> Dec(13)

const uint32_t CXDTOObj = 0;

// Перечисления
const uint32_t CEnumsForm = 0;   //{ форма Перечисления }
const uint32_t CEnumsModule = 1; //{ Модуль формы Перечисления }

// Команды
const uint32_t CCommandsInfo = 1;   //{ Модуль команды }
const uint32_t CCommandsModule = 2; //{ Модуль команды }

// Внешние источники данных
const uint32_t CExternalDataSourcesExtObj0 = 0;
const uint32_t CExternalDataSourcesExtObj1 = 1;
const uint32_t CExternalDataSourcesExtObj2 = 2;
const uint32_t CExternalDataSourcesExtObj3 = 3;
const uint32_t CExternalDataSourcesExtObj4 = 4;
const uint32_t CExternalDataSourcesExtObj5 = 5;
const uint32_t CExternalDataSourcesExtObj6 = 6;
const uint32_t CExternalDataSourcesExtObj7 = 7;
const uint32_t CExternalDataSourcesExtObj8 = 8;
const uint32_t CExternalDataSourcesExtObj9 = 9;

//{ расширения файлов }
const wstring CBPTaskModuleExt        = L".txt";
const wstring CBPTaskMnrgModuleExt    = L".txt";
const wstring CDocMngrModuleExt       = L".txt";
const wstring CBPObjMngrModuleExt     = L".txt";
const wstring CConfigAllSubsystemsExt = L".txt";
const wstring CSettStorageHelpExt     = L".html";
const wstring CSettStorageFofmExt     = L".frm";
const wstring CMoxelDataExt           = L".mxd";
const wstring CPlanedTaskPlanExt      = L".txt";
const wstring CBPTaskMaketExt         = L".mkt";
const wstring CAccntMoxelExt          = L".mxl";
const wstring CDocModuleExt           = L".txt";
const wstring CDocHelpExt             = L".html";
const wstring CRefModuleExt           = L".txt";
const wstring CRefHelpExt             = L".html";
const wstring CRefPredefinedDataExt   = L".prdf";
const wstring CReportModuleExt        = L".txt";
const wstring CReportHelpExt          = L".html";
const wstring CProcessingModuleExt    = L".txt";
const wstring CProcessingHelpExt      = L".html";
const wstring CInfoRegHelpExt         = L".html";
const wstring CInfoRegModuleExt       = L".txt";
const wstring CCommonModuleModuleExt  = L".txt";
const wstring CAccumRegHelpExt        = L".html";
const wstring CAccumRegModuleExt      = L".txt";
const wstring CAccumRegModuleExt2     = L".mftxt";
const wstring CAccumRegAsmExt         = L".ras";
const wstring CAccntModuleExt         = L".txt";
const wstring CAccntHelpExt           = L".html";
const wstring CAccntPredefinedDataExt = L".prdf";
const wstring CDataExchHelpExt        = L".html";
const wstring CDataExchContentExt     = L".cnt";
const wstring CDataExchModuleExt      = L".txt";
const wstring CDataExchMngrModuleExt  = L".txt";
const wstring CConstModuleExt         = L".txt";
const wstring CConstModuleMngrExt     = L".txt";
const wstring CCalcKindModuleExt      = L".txt";
const wstring CCalcKindHelpExt        = L".html";

const wstring CCalcKindPredefinedDataExt   = L".prdf";
										   
const wstring CCalcRegModuleExt            = L".txt";
const wstring CCalcRegHelpExt              = L".html";
const wstring CRecalcModuleExt             = L".txt";
const wstring CBPObjHelpExt                = L".html";
const wstring CBPObjModuleExt              = L".txt";
const wstring CBPObjFlowchartExt           = L".fch";
const wstring CBPObjMaketExt               = L".mkt";
const wstring CAccntRegModuleExt           = L".txt";
const wstring CAccntRegHelpExt             = L".html";
const wstring CBPTaskHelpExt               = L".html";
const wstring CMoxelExt                    = L".mxl";
const wstring CJournalsModuleExt           = L".txt";
const wstring CJournalsHelpExt             = L".html";
const wstring CAccntFormHelpExt            = L".html";
const wstring CCalcPlanModuleExt           = L".txt";
const wstring CCalcPlanHelpExt             = L".html";
const wstring CCalcPlanPredefinedDataExt   = L".prdf";
const wstring CGlobalFormsHelpExt          = L".html";
const wstring CCommonPictDataExt           = L".pct8";
const wstring CSubsystemHelpExt            = L".html";
const wstring CSubsystemInterfaceExt       = L".intf";
const wstring CCommonTableHelpExt          = L".html";
const wstring CCommonTableMXLExt           = L".mxl";
const wstring CChPlanHelpExt               = L".html";
const wstring CChPlanModuleExt             = L".txt";
const wstring CChPlanManagerModuleExt      = L".txt";
const wstring CChPlanFormHelpExt           = L".html";
const wstring CChPlanPredefinedDataExt     = L".prdf";
const wstring CChPlanMoxelExt              = L".mxl";
const wstring CRoleDataExt                 = L".rol8";
const wstring CStyleDataExt                = L".stl8";
const wstring CInterfaceDataExt            = L".txt";
const wstring CJournalsFormHelpExt         = L".html";
const wstring CSelCriterionsFormHelpExt    = L".html";
const wstring CGlobalModuleExt             = L".txt";
const wstring CConfigLogoExt               = L".pct8";
const wstring CConfigSplashExt             = L".pct8";
const wstring CConfigPictOsnRazdExt        = L".pct8";
const wstring CConfigHelpExt               = L".html";
const wstring CConfigExtInfoExt            = L".dat";
const wstring CExternalConnectionModuleExt = L".txt";
const wstring CInterfaceClientApp          = L".txt";
const wstring CUserModuleExt               = L".txt";
const wstring CTaskDescktopAreaExt         = L".dsk";
const wstring CTaskDescktopIntfExt         = L".dintf";
const wstring CGlobalFormsFormExt          = L".frm";
const wstring CSelCriterionsFormExt        = L".frm";
const wstring CXDTOData                    = L".xml";
//CEnumsExt                  = ".info";
const wstring CEnumsExt                    = L".html";
const wstring CEnumsObjExt                 = L".txt";
const wstring CCommandsModuleExt           = L".txt";
const wstring CCommandsInfoExt             = L".itxt";
const wstring CSignMobileClientExt         = L".sign";

const wstring CExternalDataSourcesExtObj0_ext = L".txt";
const wstring CExternalDataSourcesExtObj1_ext = L".txt";
const wstring CExternalDataSourcesExtObj2_ext = L".txt";
const wstring CExternalDataSourcesExtObj3_ext = L".txt";
const wstring CExternalDataSourcesExtObj4_ext = L".txt";
const wstring CExternalDataSourcesExtObj5_ext = L".txt";
const wstring CExternalDataSourcesExtObj6_ext = L".txt";
const wstring CExternalDataSourcesExtObj7_ext = L".txt";
const wstring CExternalDataSourcesExtObj8_ext = L".txt";
const wstring CExternalDataSourcesExtObj9_ext = L".txt";

uint32_t StringExtToInt(const wstring Ext);

