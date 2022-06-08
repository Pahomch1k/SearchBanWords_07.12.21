#pragma once
#include "header.h"

class SearchW
{
public:
	SearchW(void);
public:
	~SearchW(void);
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	static SearchW* ptr;
	void Cls_OnClose(HWND hwnd);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnBrowse();
	void ReadResult();
	HANDLE hMutex;
};
