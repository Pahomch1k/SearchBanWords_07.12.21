#include "SearchW.h" 

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpszCmdLine, int nCmdShow)
{
	SearchW Srch;
	return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, SearchW::DlgProc);
}