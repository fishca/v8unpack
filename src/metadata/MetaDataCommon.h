//---------------------------------------------------------------------------

#ifndef MetaDataCommonH
#define MetaDataCommonH

#include <vector>
#include <map>
#include <string>

#include "../common.h"

// #include "NodeTypes.h"
// #include "Parse_tree.h"
// #include "Common.h"
// #include "ConfigStorage.h"
// #include "TempStream.h"

//---------------------------------------------------------------------------
//extern TGUID EmptyUID;

//---------------------------------------------------------------------------

struct MetaPropertyLess;
struct GeneratedType;
struct VarValidValue;

class MetaBase;
class MetaValue;
class MetaProperty;
class Class;
class MetaType;
class MetaTypeSet;
class Value1C;
class Value1C_obj;
class Value1C_metaobj;
class MetaContainer;
class ConfigStorage;

class Value1C_obj_ExportThread;

using namespace std;

//---------------------------------------------------------------------------
// Версии контейнера 1С
// значения версий должны обязательно располагаться по возрастанию, чтобы можно было сравнивать версии на >, < и =
enum ContainerVer
{
	cv_2_0   = 1,
	cv_5_0   = 2,
	cv_6_0   = 3,
	cv_106_0 = 4,
	cv_200_0 = 5,
	cv_202_2 = 6,
	cv_216_0 = 7
};

//---------------------------------------------------------------------------
// Версии 1С
// значения версий должны обязательно располагаться по возрастанию, чтобы можно было сравнивать версии на >, < и =
enum Version1C
{
	v1C_min    = 0,
	v1C_8_0    = 1,
	v1C_8_1    = 2,
	v1C_8_2    = 3,
	v1C_8_2_14 = 4,
	v1C_8_3_1  = 5,
	v1C_8_3_2  = 6,
	v1C_8_3_3  = 7,
	v1C_8_3_4  = 8,
	v1C_8_3_5  = 9,
	v1C_8_3_6  = 10
};

//---------------------------------------------------------------------------
// Способы выгрузки
enum ExportType
{
	et_default = 0,
	et_catalog = 1,
	et_file = 2
};

//---------------------------------------------------------------------------
// Базовый класс метаданных 1С
class MetaBase
{
protected:
	String fname;
	String fename;
public:
	MetaBase(){};
	MetaBase(const String& _name, const String& _ename) : fname(_name), fename(_ename){};

    ~MetaBase(){};

    void setname(const String& _name){
        fname = _name;
    };

    void setename(const String& _ename){
        fename = _ename;
    };

    // __property String name = {read = fname};
	// __property String ename = {read = fename};

    String getname(bool english = false)
	{
		if(english)
            return fename;
		else
            return fname;
	};
};


#endif // MetaDataCommonH

