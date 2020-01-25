//---------------------------------------------------------------------------

#include "Parse_tree.h"

#include <string>
#include <iostream>
#include <fstream> // подключаем файлы
#include <algorithm>
#include <sstream>
#include <locale>
#include <codecvt>
#include <regex>

using namespace std;

//---------------------------------------------------------------------------



const regex exp_number("^-?\\d+$");
const regex exp_number_exp("^-?\\d+(\\.?\\d*)?((e|E)-?\\d+)?$");

const regex exp_guid("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");

const regex exp_binary("^#base64:[0-9a-zA-Z\\+=\\r\\n\\/]*$");
const regex exp_binary2("^[0-9a-zA-Z\\+=\\r\\n\\/]+$");

const regex exp_link("^[0-9]+:[0-9a-fA-F]{32}$");

const regex exp_binary_d("^#data:[0-9a-zA-Z\\+=\\r\\n\\/]*$");

//---------------------------------------------------------------------------
tree::tree(const string& _value, const node_type _type, tree* _parent)
{
	value = _value;
	type = _type;
	parent = _parent;

	num_subnode = 0;
	index = 0;
	if(parent)
	{
		parent->num_subnode++;
		prev = parent->last;
		if(prev)
		{
			prev->next = this;
			index = prev->index + 1;
		}
		else parent->first = this;
		parent->last = this;
	}
	else prev = NULL;
	next = NULL;
	first = NULL;
	last = NULL;
}

//---------------------------------------------------------------------------
tree::~tree()
{
	while(last) delete last;
	if(prev) prev->next = next;
	if(next) next->prev = prev;
	if(parent)
	{
		if(parent->first == this) parent->first = next;
		if(parent->last == this) parent->last = prev;
		parent->num_subnode--;
	}
}

//---------------------------------------------------------------------------
tree* tree::add_child(const string& _value, const node_type _type)
{
	return new tree(_value, _type, this);
}

//---------------------------------------------------------------------------
tree* tree::add_child()
{
	return new tree("", nd_empty, this);
}

//---------------------------------------------------------------------------
tree* tree::add_node()
{
	return new tree("", nd_empty, this->parent);
}

//---------------------------------------------------------------------------
string& tree::get_value()
{
	return value;
}

//---------------------------------------------------------------------------
node_type tree::get_type()
{
	return type;
}

//---------------------------------------------------------------------------
void tree::set_value(const string& v, const node_type t)
{
	value = v;
	type = t;
}

//---------------------------------------------------------------------------
int tree::get_num_subnode()
{
	return num_subnode;
}

//---------------------------------------------------------------------------
tree* tree::get_subnode(int _index)
{
	if(_index >= num_subnode) return NULL;
	tree* t = first;
	while(_index)
	{
		t = t->next;
		--_index;
	}
	return t;
}

//---------------------------------------------------------------------------
tree* tree::get_subnode(const string& node_name)
{
	tree* t = first;
	while(t)
	{
		if(t->value == node_name) return t;
		t = t->next;
	}
	return NULL;
}

//---------------------------------------------------------------------------
tree* tree::get_next()
{
	return next;
}

//---------------------------------------------------------------------------
tree* tree::get_parent()
{
	return parent;
}

//---------------------------------------------------------------------------
tree* tree::get_first()
{
	return first;
}

//---------------------------------------------------------------------------
tree& tree::operator [](int _index)
{
	if(!this) return *this; //-V704

	tree* ret = first;
	while(_index)
	{
		if(ret) ret = ret->next;
		--_index;
	}
	return *ret;
}


void replaceAll(std::string& str, const std::string& from, const std::string& to) 
{
	if (from.empty())
		return;

	size_t start_pos = 0;

	while ((start_pos = str.find(from, start_pos)) != std::string::npos) 
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

//---------------------------------------------------------------------------
void tree::outtext(string& text)
{
	node_type lt = node_type::nd_unknown;

	if(num_subnode)
	{
		if(text.length()) 
			text += "\r\n";
		
		text += '{';
		
		tree* t = first;
		while(t)
		{
			t -> outtext(text);
			lt = t -> type;
			t  = t -> next;
			if(t) 
				text += ',';
		}
		if(lt == nd_list) 
			text += "\r\n";
		
		text += '}';
	}
	else
	{
		switch(type)
		{
			case nd_string:
				text += '\"';
				//text += value.replace(text.begin(), text.end(), "\"", "\"\"");
				replaceAll(value, "\"", "\"\"");
				text += value;
				text += '\"';
				break;
			case nd_number:
			case nd_number_exp:
			case nd_guid:
			case nd_list:
			case nd_binary:
			case nd_binary2:
			case nd_link:
			case nd_binary_d:
				text += value;
				break;
			default:
				break;
		}
	}
}

//---------------------------------------------------------------------------
string tree::path()
{
	string p = "";
	tree* t;

	char buf[80];
	//string buf = ""

	if(!this) 
		return ":??"; //-V704
	
	for(t = this; t->parent; t = t->parent)
	{
		//p = string(":") + t->index + p;
		_itoa(t->index, buf, 10);
		p = string(":") + buf + p;
	}
	return p;
}

//---------------------------------------------------------------------------
node_type classification_value(const string& value)
{
	if(value.length() == 0) return nd_empty;
	if(regex_match(value.c_str(), exp_number)) return nd_number;
	if(regex_match(value.c_str(), exp_number_exp)) return nd_number_exp;
	if(regex_match(value.c_str(), exp_guid)) return nd_guid;
	if(regex_match(value.c_str(), exp_binary)) return nd_binary;
	if(regex_match(value.c_str(), exp_link)) return nd_link;
	if(regex_match(value.c_str(), exp_binary2)) return nd_binary2;
	if(regex_match(value.c_str(), exp_binary_d)) return nd_binary_d;
	return nd_unknown;
}

tree* parse_1Ctext(const string& text, const string& path)
{
	string __curvalue__;

	enum _state{
		s_value, // ожидание начала значения
		s_delimitier, // ожидание разделителя
		s_string, // режим ввода строки
		s_quote_or_endstring, // режим ожидания конца строки или двойной кавычки
		s_nonstring // режим ввода значения не строки
	}state = s_value;

	string curvalue;
	tree* ret;
	tree* t;
	int len = text.length();
	int i;
	char sym;
	node_type nt;

	__curvalue__ = "";

	ret = new tree("", nd_list, NULL);
	t = ret;

	for(i = 1; i <= len; i++)
	{
		sym = text[i];
		if(!sym) break;

		switch(state)
		{
			case s_value:
				switch(sym)
				{
					case ' ': // space
					case '\t':
					case '\r':
					case '\n':
						break;
					case '"':
						
						__curvalue__.clear();
						state = s_string;
						break;
					case '{':
						t = new tree("", nd_list, t);
						break;
					case '}':
						if(t->get_first()) t->add_child("", nd_empty);
						t = t->get_parent();
						if(!t)
						{
							/*
							if(1!=1) err->AddError(L"Ошибка формата потока. Лишняя закрывающая скобка }.",
								L"Позиция", i,
								L"Путь", path);
							*/
							delete ret;
							return NULL;
						}
						state = s_delimitier;
						break;
					case ',':
						t->add_child("", nd_empty);
						break;
					default:
						__curvalue__.clear();
						__curvalue__ += sym;
						state = s_nonstring;
						break;
				}
				break;
			case s_delimitier:
				switch(sym)
				{
					case ' ': // space
					case '\t':
					case '\r':
					case '\n':
						break;
					case ',':
						state = s_value;
						break;
					case '}':
						t = t->get_parent();
						if(!t)
						{
							delete ret;
							return NULL;
						}
						break;
					default:
						delete ret;
						return NULL;
				}
				break;
			case s_string:
				if (sym == '"') {
					state = s_quote_or_endstring;
				}
				else 
					__curvalue__ += sym;
				break;
			case s_quote_or_endstring:
				if(sym == '"')
				{
					__curvalue__ += sym;
					state = s_string;
				}
				else
				{
					t->add_child(__curvalue__, nd_string);
					switch(sym)
					{
						case ' ': // space
						case '\t':
						case '\r':
						case '\n':
							state = s_delimitier;
							break;
						case ',':
							state = s_value;
							break;
						case '}':
							t = t->get_parent();
							if(!t)
							{
								delete ret;
								return NULL;
							}
							state = s_delimitier;
							break;
						default:
							delete ret;
							return NULL;
					}
				}
				break;
			case s_nonstring:
				switch(sym)
				{
					case ',':
						curvalue = __curvalue__;
						nt = classification_value(curvalue);
						if (nt == nd_unknown) {
							/*
							if(err) err->AddError(L"Ошибка формата потока. Неизвестный тип значения.",
							L"Значение", curvalue,
							L"Путь", path);
						*/
						}
						t->add_child(curvalue, nt);
						state = s_value;

						break;

					case '}':
						curvalue = __curvalue__;
						nt = classification_value(curvalue);
						if(nt == nd_unknown) {
							/*
							if(err) err->AddError(L"Ошибка формата потока. Неизвестный тип значения.",
							L"Значение", curvalue,
							L"Путь", path);
						*/
						}
						t->add_child(curvalue, nt);
						t = t->get_parent();
						if(!t)
						{
							delete ret;
							return NULL;
						}
						state = s_delimitier;
						break;
					default:
						__curvalue__ += sym;
						break;
				}
				break;
			default:
				delete ret;
				return NULL;

		}
	}

	if(state == s_nonstring)
	{
		curvalue = __curvalue__;
		nt = classification_value(curvalue);
		if(nt == nd_unknown) {
			/*
			if(err) err->AddError(L"Ошибка формата потока. Неизвестный тип значения.",
			L"Значение", curvalue,
			L"Путь", path);
		*/
		}
		t->add_child(curvalue, nt);
	}
	
	else if(state == s_quote_or_endstring) 
		t -> add_child(__curvalue__, nd_string);
	else if(state != s_delimitier)
	{
		delete ret;
		return NULL;
	}

	if(t != ret)
	{
		delete ret;
		return NULL;
	}

	return ret;

}

// проверка формата потока
bool test_parse_1Ctext(const string& str, const string& path)
{
	string __curvalue__;

	enum _state{
		s_value, // ожидание начала значения
		s_delimitier, // ожидание разделителя
		s_string, // режим ввода строки
		s_quote_or_endstring, // режим ожидания конца строки или двойной кавычки
		s_nonstring // режим ввода значения не строки
	}state = s_value;

	string curvalue;
	int i;
	char sym;
	int _sym;
	node_type nt;
	int level;
	bool ret = true;

	__curvalue__ = "";

	

	//reader = new TStreamReader(str, true);
	level = 0;

	//for(i = 1, _sym = reader->Read(); _sym >= 0; i++, _sym = reader->Read())
	for(i = 1, _sym = str[i]; _sym > 0; i++, _sym = str[i])
	{
		sym = _sym;
		

		switch(state)
		{
			case s_value:
				switch(sym)
				{
					case ' ': // space
					case '\t':
					case '\r':
					case '\n':
						break;
					case '"':
						__curvalue__.clear();
						state = s_string;
						break;
					case '{':
						level++;
						break;
					case '}':
						if(level <= 0)
						{
							ret = false;
						}
						state = s_delimitier;
						level--;
						break;
					default:
						//curvalue = String(sym);
						__curvalue__.clear();
						__curvalue__ += sym;
						state = s_nonstring;
						break;
				}
				break;
			case s_delimitier:
				switch(sym)
				{
					case ' ': // space
					case '\t':
					case '\r':
					case '\n':
						break;
					case L',':
						state = s_value;
						break;
					case L'}':
						if(level <= 0)
						{
							ret = false;
						}
						level--;
						break;
					default:
						return ret;
				}
				break;
			case s_string:
				if(sym == '"'){
					state = s_quote_or_endstring;
				}
				else 
					__curvalue__+= sym;
				break;
			case s_quote_or_endstring:
				if(sym == L'"')
				{
					__curvalue__ += sym;
					state = s_string;
				}
				else
				{
					switch(sym)
					{
						case ' ': // space
						case '\t':
						case '\r':
						case '\n':
							state = s_delimitier;
							break;
						case ',':
							state = s_value;
							break;
						case '}':
							if(level <= 0)
							{
								ret = false;
							}
							level--;
							state = s_delimitier;
							break;
						default:
							return ret;
					}
				}
				break;
			case s_nonstring:
				switch(sym)
				{
					case ',':
						curvalue = __curvalue__;
						nt = classification_value(curvalue);
						if(nt == nd_unknown)
						{
							ret = false;
						}
						state = s_value;
						break;
					case '}':
						curvalue = __curvalue__;
						nt = classification_value(curvalue);
						if(nt == nd_unknown)
						{
							ret = false;
						}
						if(level <= 0)
						{
							ret = false;
						}
						level--;
						state = s_delimitier;
						break;
					default:
						__curvalue__ += sym;
						break;
				}
				break;
			default:
				ret = false;
				break;
		}
	}

	if(state == s_nonstring)
	{
		curvalue = __curvalue__;
		nt = classification_value(curvalue);
		if(nt == nd_unknown)
		{
			ret = false;
		}
	}
	else if(state == s_quote_or_endstring)
	{

	}
	else if(state != s_delimitier)
	{
		ret = false;
	}

	if(level > 0)
	{
		ret = false;
	}

	return ret;

}

string outtext(tree* t)
{
	string text;
	if(t) 
		if(t -> get_first()) 
			t -> get_first() -> outtext(text);
	
	return text;
}

