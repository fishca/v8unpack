#include "v8Tree.h"
#include <string>
#include <algorithm>
#include <boost/regex.hpp>

const boost::wregex exp_number(L"^-?\\d+$");
const boost::wregex exp_number_exp(L"^-?\\d+(\\.?\\d*)?((e|E)-?\\d+)?$");
const boost::wregex exp_guid(L"^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");
const boost::wregex exp_binary(L"^#base64:[0-9a-zA-Z\\+=\\r\\n\\/]*$");
const boost::wregex exp_binary2(L"^[0-9a-zA-Z\\+=\\r\\n\\/]+$");
const boost::wregex exp_link(L"^[0-9]+:[0-9a-fA-F]{32}$");
const boost::wregex exp_binary_d(L"^#data:[0-9a-zA-Z\\+=\\r\\n\\/]*$");

node_type classification_value(const wstring& value)
{
	if (value.size() == 0) return nd_empty;
	if (regex_match(value.c_str(), exp_number))     return nd_number;
	if (regex_match(value.c_str(), exp_number_exp)) return nd_number_exp;
	if (regex_match(value.c_str(), exp_guid))       return nd_guid;
	if (regex_match(value.c_str(), exp_binary))     return nd_binary;
	if (regex_match(value.c_str(), exp_link))       return nd_link;
	if (regex_match(value.c_str(), exp_binary2))    return nd_binary2;
	if (regex_match(value.c_str(), exp_binary_d))   return nd_binary_d;
	return nd_unknown;
}


v8Tree::v8Tree(const wstring _value, const node_type _type, v8Tree* _parent)
{
	value = _value;
	type = _type;
	parent = _parent;

	num_subnode = 0;
	
	index = 0;
	
	if (parent)
	{
		parent->num_subnode++;
	
		prev = parent->last;
		
		if (prev)
		{
			prev->next = this;
			index = prev->index + 1;
		}
		else 
			parent->first = this;
		
		parent->last = this;
	}
	else 
		prev = nullptr;
	
	next = nullptr;
	first = nullptr;
	last = nullptr;

}

v8Tree::~v8Tree()
{
	while (last) 
		delete last;

	if (prev) 
		prev->next = next;

	if (next) 
		next->prev = prev;

	if (parent)
	{
		if (parent->first == this) 
			parent->first = next;
		if (parent->last == this) 
			parent->last = prev;
		
		parent->num_subnode--;
	}

}

v8Tree* v8Tree::add_child(const wstring _value, const node_type _type)
{
	return new v8Tree(_value, _type, this);
}

v8Tree* v8Tree::add_child()
{
	return new v8Tree(L"", nd_empty, this);
}

v8Tree* v8Tree::add_node()
{
	return new v8Tree(L"", nd_empty, this->parent);
}

wstring v8Tree::get_value()
{
	return value;
}

node_type v8Tree::get_type()
{
	return type;
}

int v8Tree::get_num_subnode()
{
	return num_subnode;
}

v8Tree* v8Tree::get_subnode(int _index)
{
	if (_index >= num_subnode) 
		return NULL;
	
	v8Tree* t = first;
	while (_index)
	{
		t = t->next;
		--_index;
	}
	return t;
}

v8Tree* v8Tree::get_subnode(const wstring node_name)
{
	v8Tree* t = first;
	while (t)
	{
		if (t->value == node_name) 
			return t;
		
		t = t->next;
	}
	return nullptr;
}

v8Tree* v8Tree::get_next()
{
	return next;
}

v8Tree* v8Tree::get_parent()
{
	return parent;
}

v8Tree* v8Tree::get_first()
{
	return first;
}

v8Tree* v8Tree::get_last()
{
	return last;
}

v8Tree& v8Tree::operator[](int _index)
{
	if (!this) 
		return *this; //-V704

	v8Tree* ret = first;
	while (_index)
	{
		if (ret) ret = ret->next;
		--_index;
	}
	return *ret;
}

void v8Tree::set_value(const wstring& v, const node_type t)
{
	value = v;
	type = t;
}

void v8Tree::outtext(wstring& text)
{
	node_type lt;
	const wchar_t x = '\"';
	const wchar_t y = '\"\"';

	if (num_subnode)
	{
		if (text.size()) 
			text += L"\r\n";
		
		text += L'{';

		v8Tree* t = first;
		
		while (t)
		{
			t->outtext(text);
			lt = t->type;
			t = t->next;
			if (t) 
				text += L',';
		}
		if (lt == nd_list) 
			text += L"\r\n";
		
		text += L'}';
	}
	else
	{
		switch (type)
		{
		case nd_string:
			text += L'\"';
			std::replace(value.begin(), value.end(), x, y);
			text += L'\"';
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
			//if(msreg) msreg->AddError(L"Ошибка вывода потока. Пустой или неизвестный узел.");
			break;
		}
	}

}

wstring v8Tree::path()
{
	wstring result = L"";
	v8Tree* t;

	if (!this) 
		return L":??"; //-V704
	
	for (t = this; t->parent; t = t->parent)
	{
		//result = wstring(":") + std::to_string(t->index) + result;
		result = L":" + t->index + result;
	}
	return result;
}


v8Tree* parse_1Ctext(const wstring& text)
{
	wstring __curvalue__;

	enum _state {
		s_value,              // ожидание начала значения
		s_delimitier,         // ожидание разделителя
		s_string,             // режим ввода строки
		s_quote_or_endstring, // режим ожидания конца строки или двойной кавычки
		s_nonstring           // режим ввода значения не строки
	}state = s_value;

	wstring curvalue;
	v8Tree* ret;
	v8Tree* t;
	int len = text.size();
	int i;
	wchar_t sym;
	//char sym;
	node_type nt;

	//__curvalue__ = new TStringBuilder;
	__curvalue__ = L' ';

	ret = new v8Tree(L"", nd_list, NULL);

	t = ret;

	for (i = 1; i <= len; i++)
	{
		sym = text[i];
		if (!sym)
			break;

		switch (state)
		{
		case s_value:
			switch (sym)
			{
			case L' ': // space
			case L'\t':
			case L'\r':
			case L'\n':
				break;
			case L'"':
				//__curvalue__->Clear();
				__curvalue__.clear();
				state = s_string;
				break;
			case L'{':
				t = new v8Tree(L"", nd_list, t);
				break;
			case L'}':
				if (t->get_first())
					t->add_child(L"", nd_empty);
				t = t->get_parent();
				if (!t)
				{
					/*
					if (msreg) msreg->AddError(L"Ошибка формата потока. Лишняя закрывающая скобка }.",
						L"Позиция", i,
						L"Путь", path);
					*/
					delete ret;
					return NULL;
				}
				state = s_delimitier;
				break;
			case L',':
				t->add_child(L"", nd_empty);
				break;
			default:
				//curvalue = String(sym);
				//__curvalue__->Clear();
				__curvalue__.clear();
				//__curvalue__->Append(sym);
				//__curvalue__.append(sym);
				__curvalue__ = __curvalue__ + sym;
				state = s_nonstring;
				break;
			}
			break;
		case s_delimitier:
			switch (sym)
			{
			case L' ': // space
			case L'\t':
			case L'\r':
			case L'\n':
				break;
			case L',':
				state = s_value;
				break;
			case L'}':
				t = t->get_parent();
				if (!t)
				{
					/*
					if (msreg) msreg->AddError(L"Ошибка формата потока. Лишняя закрывающая скобка }.",
						L"Позиция", i,
						L"Путь", path);
					*/
					delete ret;
					return NULL;
				}
				//state = s_delimitier;
				break;
			default:
				/*
				if (msreg) msreg->AddError(L"Ошибка формата потока. Ошибочный символ в режиме ожидания разделителя.",
					L"Символ", sym,
					L"Код символа", tohex(sym),
					L"Путь", path);
				*/
				delete ret;
				return NULL;
			}
			break;
		case s_string:
			if (sym == L'"') {
				state = s_quote_or_endstring;
			}
			//else curvalue += String(sym);
			//else __curvalue__->Append(sym);
			else
				__curvalue__ = __curvalue__ + sym;
			break;
		case s_quote_or_endstring:
			if (sym == L'"')
			{
				//curvalue += String(sym);
				//__curvalue__->Append(sym);
				__curvalue__ = __curvalue__ + sym;
				state = s_string;
			}
			else
			{
				//t->add_child(curvalue, nd_string);
				//t->add_child(__curvalue__->ToString(), nd_string);
				t->add_child(__curvalue__, nd_string);
				switch (sym)
				{
				case L' ': // space
				case L'\t':
				case L'\r':
				case L'\n':
					state = s_delimitier;
					break;
				case L',':
					state = s_value;
					break;
				case L'}':
					t = t->get_parent();
					if (!t)
					{
						/*
						if (msreg) msreg->AddError(L"Ошибка формата потока. Лишняя закрывающая скобка }.",
							L"Позиция", i,
							L"Путь", path);

						*/
						delete ret;
						return NULL;
					}
					state = s_delimitier;
					break;
				default:
					/*
					if (msreg) msreg->AddError(L"Ошибка формата потока. Ошибочный символ в режиме ожидания разделителя.",
						L"Символ", sym,
						L"Код символа", tohex(sym),
						L"Путь", path);
					*/
					delete ret;
					return NULL;
				}
			}
			break;
		case s_nonstring:
			switch (sym)
			{
			case ',':
				//curvalue = __curvalue__->ToString();
				curvalue = __curvalue__;
				nt = classification_value(curvalue);
				if (nt == nd_unknown)
					//if (msreg) msreg->AddError(L"Ошибка формата потока. Неизвестный тип значения.", L"Значение", curvalue, L"Путь", path);
					;
				t->add_child(curvalue, nt);
				state = s_value;
				break;
			case '}':
				//curvalue = __curvalue__->ToString();
				curvalue = __curvalue__;
				nt = classification_value(curvalue);
				if (nt == nd_unknown)
					//if (msreg) msreg->AddError(L"Ошибка формата потока. Неизвестный тип значения.", L"Значение", curvalue, L"Путь", path);
					t->add_child(curvalue, nt);
				t = t->get_parent();
				if (!t)
				{
					/*
					if (msreg) msreg->AddError(L"Ошибка формата потока. Лишняя закрывающая скобка }.",
						L"Позиция", i,
						L"Путь", path);
					*/
					delete ret;
					return NULL;
				}
				state = s_delimitier;
				break;
			default:
				//curvalue += String(sym);
				//__curvalue__->Append(sym);
				__curvalue__ = __curvalue__ + sym;
				break;
			}
			break;
		default:
			/*
			if (msreg) msreg->AddError(L"Ошибка формата потока. Неизвестный режим разбора.",
				L"Режим разбора", tohex(state),
				L"Путь", path);
			*/
			delete ret;
			return NULL;

		}
	}

	if (state == s_nonstring)
	{
		//curvalue = __curvalue__->ToString();
		curvalue = __curvalue__;
		nt = classification_value(curvalue);
		if (nt == nd_unknown)
			//if (msreg) msreg->AddError(L"Ошибка формата потока. Неизвестный тип значения.", L"Значение", curvalue, L"Путь", path);
			t->add_child(curvalue, nt);
	}
	//else if(state == s_quote_or_endstring) t->add_child(curvalue, nd_string);
	//else if (state == s_quote_or_endstring) t->add_child(__curvalue__->ToString(), nd_string);
	else if (state == s_quote_or_endstring)
		t->add_child(__curvalue__, nd_string);
	else if (state != s_delimitier)
	{
		/*
		if (msreg) msreg->AddError(L"Ошибка формата потока. Незавершенное значение",
			L"Режим разбора", tohex(state),
			L"Путь", path);
		*/
		delete ret;
		return NULL;
	}

	if (t != ret)
	{
		/*
		if (msreg) msreg->AddError(L"Ошибка формата потока. Не хватает закрывающих скобок } в конце текста разбора.",
			L"Путь", path);
		*/
		delete ret;
		return NULL;
	}

	return ret;
}


wstring outtext(v8Tree* t)
{
	wstring text;
	if (t) 
		if (t->get_first()) 
			t->get_first()->outtext(text);
	return text;
}
