#pragma once

//---------------------------------------------------------------------------
struct v8header_struct {
	__int64 time_create;
	__int64 time_modify;
	int zero;
};

//---------------------------------------------------------------------------
struct fat_item {
	int header_start;
	int data_start;
	int ff; // всегда 7fffffff
};

//---------------------------------------------------------------------------
struct catalog_header {
	int start_empty; // начало первого пустого блока
	int page_size; // размер страницы по умолчанию
	int version; // версия
	int zero; // всегда ноль?
};

//---------------------------------------------------------------------------
enum FileIsCatalog {
	iscatalog_unknown,
	iscatalog_true,
	iscatalog_false
};
