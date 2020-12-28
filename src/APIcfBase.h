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
	int ff; // ������ 7fffffff
};

//---------------------------------------------------------------------------
struct catalog_header {
	int start_empty; // ������ ������� ������� �����
	int page_size; // ������ �������� �� ���������
	int version; // ������
	int zero; // ������ ����?
};

//---------------------------------------------------------------------------
enum FileIsCatalog {
	iscatalog_unknown,
	iscatalog_true,
	iscatalog_false
};
