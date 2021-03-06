﻿//---------------------------------------------------------------------------

#ifndef NodeTypesH
#define NodeTypesH

#include <string>

using namespace std;

//---------------------------------------------------------------------------
enum node_type{
	nd_empty      = 0, // пусто
	nd_string     = 1, // строка
	nd_number     = 2, // число
	nd_number_exp = 3, // число с показателем степени
	nd_guid       = 4, // уникальный идентификатор
	nd_list       = 5, // список
	nd_binary     = 6, // двоичные данные (с префиксом #base64:)
	nd_binary2    = 7, // двоичные данные формата 8.2 (без префикса)
	nd_link       = 8, // ссылка
	nd_binary_d   = 9, // двоичные данные (с префиксом #data:)
	nd_unknown         // неизвестный тип
};

std::string get_node_type_presentation(node_type type);
//---------------------------------------------------------------------------
#endif
