#pragma once

#include <string>
#include <iostream>
#include "NodeTypes.h"

using namespace std;



/*
https://www.math.spbu.ru/user/nlebedin/2-year/theory_atree_.pdf
                    ---
                   | 1 |
  				    ---
  
  
    ---               ---             ---
   | 2 |			 | 3 |			 | 4 |
    ---			      ---			  ---
                
  ---    ---   ---      ---         ---   ---  
 | 5 |	| 6 | | 7 |	   | 8 |	   | 9 | | 10 |	
  ---	 ---   ---	    ---		    ---   ---	
                
	Теперь один указатель у узла указывает на его первого (самого левого) сына, 
	он называется указатель ВНИЗ (down), а другой - на соседнего брата справа (right), и он называется указатель вправо.

                
*/

template<class T>
struct ANode // Arbitrary - поскольку число сыновей произвольно
{
	T data;
	ANode<T> *down, *right;
	ANode(T dd, ANode<T>* d = nullptr, ANode<T>* r = nullptr) : data(dd), down(d), right(r) {}
};

template<class T>
void f_print(ANode<T> *p, int d = 0) // d - смещение
{
	if (p == nullptr) 
		return;
	
	for (int i = 0; i < d; i++)
		cout << ' ';
	
	cout << p->data << endl;
	
	f_print(p->down, d + 3);
	
	f_print(p->right, d);
}

template <class T>
int count(ANode<T> *p)
{
	int c = 0;
	
	if (p == nullptr) 
		return c;
	
	p = p->down;
	
	while (p != nullptr)
	{
		c++;
		p = p->right;
	}
	
	return c;
}

template <class T>
void add_first(ANode<T> *p, T d) // d - данные узла
{
	ANode<T> *t;
	t = new ANode<T>(d, nullptr, p->down);
	p->down = t;
}

template<class T>
void del_first(ANode<T> *p)
{
	ANode<T> *t, *t1, *t2;
	
	t1 = p->down;
	t2 = t1->down;
	t = t2;
	while (t->right != 0)
		t = t->right;
	t->right = t1->right;
	p->down = t2;
	
	delete t1;
}

template<class T>
struct ATree
{
	ANode <T> *root; // корень дерева
	ATree(ANode<T> *p) : root(p) {}
	// конструктор по указателю на узел
	void print() // метод печати
	{
		f_print(root); // вызывает рекурсивную функцию
	}
};


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
	int num_subnode; // количество подчиненных
	v8Tree* parent;   // +1
	v8Tree* next;     // 0
	v8Tree* prev;     // 0
	v8Tree* first;    // -1
	v8Tree* last;     // -1
	unsigned int index;
};

typedef v8Tree* treeptr;

v8Tree* parse_1Ctext(const wstring& text);
wstring outtext(v8Tree* t);