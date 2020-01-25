//---------------------------------------------------------------------------

#include "NodeTypes.h"
#include <string>

using namespace std;

//---------------------------------------------------------------------------

string get_node_type_presentation(node_type type)
{
	switch(type)
	{
		case nd_empty:
			return "Пусто";
		case nd_string:
			return "Строка";
		case nd_number:
			return "Число";
		case nd_number_exp:
			return "Число с показателем степени";
		case nd_guid:
			return "Уникальный идентификатор";
		case nd_list:
			return "Список";
		case nd_binary:
			return "Двоичные данные";
		case nd_binary2:
			return "Двоичные данные 8.2";
		case nd_binary_d:
			return "Двоичные данные data";
		case nd_link:
			return "Ссылка";
		case nd_unknown:
			return "<Неизвестный тип>";
	}
	return "<Неизвестный тип>";
}
