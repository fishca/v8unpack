/*----------------------------------------------------------
This Source Code Form is subject to the terms of the
Mozilla Public License, v.2.0. If a copy of the MPL
was not distributed with this file, You can obtain one
at http://mozilla.org/MPL/2.0/.
----------------------------------------------------------*/

#pragma once

#include <string>
#include <iostream>
#include "NodeTypes.h"

using namespace std;

class v8Tree
{
public:
	v8Tree(const wstring _value, const node_type _type, v8Tree* _parent);
	~v8Tree();
	v8Tree* add_child(const wstring _value, const node_type _type);
	v8Tree* add_child();
	v8Tree* add_node();
	wstring get_value();
	node_type get_type();
	int get_num_subnode();
	v8Tree* get_subnode(int _index);
	v8Tree* get_subnode(const wstring node_name);
	v8Tree* get_next();
	v8Tree* get_parent();
	v8Tree* get_first();
	v8Tree* get_last();
	v8Tree& operator [](int _index);
	void set_value(const wstring& v, const node_type t);
	void outtext(wstring& text);
	wstring path();
private:
	wstring value;
	node_type type;
	int num_subnode;  // количество подчиненных
	v8Tree* parent;   // +1
	v8Tree* next;     // 0
	v8Tree* prev;     // 0
	v8Tree* first;    // -1
	v8Tree* last;     // -1
	unsigned int index;
};

typedef v8Tree* treeptr;
// Основная функция разбора скобочного дерева
//
v8Tree* parse_1Ctext(const wstring& text);
wstring outtext(v8Tree* t);