/*----------------------------------------------------------
This Source Code Form is subject to the terms of the
Mozilla Public License, v.2.0. If a copy of the MPL
was not distributed with this file, You can obtain one
at http://mozilla.org/MPL/2.0/.
----------------------------------------------------------*/

#pragma once

#include "NodeTypes.h"

wstring get_node_type_presentation(node_type type)
{
	switch(type)
	{
		case nd_empty:
			return L"Пусто";
		case nd_string:
			return L"Строка";
		case nd_number:
			return L"Число";
		case nd_number_exp:
			return L"Число с показателем степени";
		case nd_guid:
			return L"Уникальный идентификатор";
		case nd_list:
			return L"Список";
		case nd_binary:
			return L"Двоичные данные";
		case nd_binary2:
			return L"Двоичные данные 8.2";
		case nd_binary_d:
			return L"Двоичные данные data";
		case nd_link:
			return L"Ссылка";
		case nd_unknown:
			return L"<Неизвестный тип>";
	}
	return L"<Неизвестный тип>";
}
