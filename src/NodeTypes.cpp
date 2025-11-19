//---------------------------------------------------------------------------

#include "NodeTypes.h"

//---------------------------------------------------------------------------

String get_node_type_presentation(node_type type)
{
	switch(type)
	{
		case node_type::nd_empty:      return "Пусто";
		case node_type::nd_string:     return "Строка";
		case node_type::nd_number:     return "Число";
		case node_type::nd_number_exp: return "Число с показателем степени";
		case node_type::nd_guid:       return "Уникальный идентификатор";
		case node_type::nd_list:       return "Список";
		case node_type::nd_binary:     return "Двоичные данные";
		case node_type::nd_binary2:    return "Двоичные данные 8.2";
		case node_type::nd_binary_d:   return "Двоичные данные data";
		case node_type::nd_link:       return "Ссылка";
		case node_type::nd_unknown:    return "<Неизвестный тип>";
	}
	return "<Неизвестный тип>";
}
