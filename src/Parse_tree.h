﻿//---------------------------------------------------------------------------

#ifndef Parse_treeH
#define Parse_treeH

#include "NodeTypes.h"
#include <string>
#include <iostream>

using namespace std;


//---------------------------------------------------------------------------
class tree
{
private:
	string value;
	
	node_type type;
	
	int num_subnode; // количество подчиненных
	
	tree* parent; // +1

	tree* next;   // 0
	tree* prev;   // 0

	tree* first;  // -1
	tree* last;   // -1

	unsigned int index;

public:
	
	tree(const string& _value, const node_type _type, tree* _parent);
	~tree();

	tree* add_child(const string& _value, const node_type _type);
	tree* add_child();

	tree* add_node();

	string& get_value();
	
	node_type get_type();
	
	int get_num_subnode();
	
	tree* get_subnode(int _index);
	tree* get_subnode(const string& node_name);
	
	tree* get_next();
	tree* get_parent();
	tree* get_first();

	tree& operator [](int _index);
	
	void set_value(const string& v, const node_type t);
	
	void outtext(string& text);
	
	tree* find_value(const string& val);

	string path();
};

typedef tree* treeptr;

tree* parse_1Ctext(const string& text, const string& path);

bool test_parse_1Ctext(const string& str, const string& path);

void replaceAll(std::string& str, const std::string& from, const std::string& to);

string outtext(tree* t);

#endif
