#include "SearchW.h"  
using namespace std;

SearchW* SearchW::ptr = NULL;
CRITICAL_SECTION cs; 

wstring FilePath = L"C:\\1\\Ban_words.txt";
wstring FolderPath = L"C:\\1\\*";

vector<wstring>FileList;
vector<wstring>WordList;


int procent;
int sum = 1000;
HWND hEdit[3], hStart[2], hBack[2], hList[2], hProgress, hStatP, hPause, hStatic[7], hAdd, hUpl, hOnMain[2], hEnd, hDel[2], hDialog, hFile[2];
char Text[200];
static HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
 
SearchW::SearchW(void)
{
	ptr = this;
}

SearchW::~SearchW(void)
{
	ReleaseMutex(hMutex);
}

void SearchW::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}
  
//Выбор папки
void SearchW::OnBrowse()
{
	BROWSEINFO binfo = { 0 };
	binfo.hwndOwner = hDialog;
	binfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_BROWSEINCLUDEFILES;
	binfo.lpszTitle = L"Выбор файла или папки...";
	LPITEMIDLIST ptr = SHBrowseForFolder(&binfo);
	TCHAR path[MAX_PATH];
	if (ptr)
	{ 
		SHGetPathFromIDList(ptr, path);
		if (wcslen(path)) 
		{ 
			SetWindowText(hEdit[2], path);
			wstring p(path);
			FolderPath = p; 
		}
	}
}
 
//Если запустил 2 раза программу - ошибка
void MessageAboutError(DWORD dwError)
{
	LPVOID lpMsgBuf = NULL;
	TCHAR szBuf[300]; 
	BOOL fOK = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	if (lpMsgBuf != NULL)
	{
		wsprintf(szBuf, TEXT("Ошибка %d: %s"), dwError, lpMsgBuf);
		MessageBox(0, szBuf, TEXT("Сообщение об ошибке"), MB_OK | MB_ICONSTOP);
		LocalFree(lpMsgBuf);
	}
} 

//Получает все файлы
DWORD WINAPI GetAllFile(LPVOID lp)
{ 
	wchar_t* FPptr = _wcsdup((FolderPath + L"*").c_str());   
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(FPptr, &data);   
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		do {
			if (wcsstr(data.cFileName, L".txt")) FileList.push_back(data.cFileName); 
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	} 
	free(FPptr); 
	return 0;
}
 
//Результат в файл
void ResultInFile(wstring FileName, wstring Word) 
{ 
	ofstream out("C:\\1\\Result.txt", std::ios_base::app); 
	string file(FileName.begin(), FileName.end());
	string word(Word.begin(), Word.end()); 
	out << "file checked:";
	out << file << endl;
	out << "word found: ";
	out << word << endl;
	out << endl; 
	out.close(); 
}

//Результат
DWORD WINAPI Result(LPVOID lp)
{
	for (int i = 0; i < FileList.size(); i++) ResultInFile(FileList[i], L"word");
	return 0;
}
 
//Редактор бан слов
void Add_Edit_File(wstring FileName)
{ 
	vector<wstring>FileWords; 
	wstring buff;
	wstring result = L"C:\\1\\" + FileName;   

	wifstream in(result);
	if (in.is_open())
	{
		while (!in.eof())
		{
			in >> buff;
			FileWords.push_back(buff);
		}
		in.close();
	}
	
	////Замена бан слова
	//for (int i = 0; i < FileWords.size(); i++) 
	//{
	//	for (int j = 0; j < WordList.size(); j++) 
	//	{ 
	//		const wchar_t* fl = FileWords[i].c_str();  
	//		const wchar_t* wl = WordList[i].c_str();    
	//		wchar_t* FLptr = _wcsdup(FileWords[i].c_str());  

	//		if (wcscmp(fl, wl) == 0) wcscpy(FLptr, L"*******"); 

	//		free(FLptr);  
	//		delete[]fl;
	//		delete[]wl;
	//	}
	//}
	  
	//Копи файл бан слов
	wchar_t* FNptr = _wcsdup(FileName.c_str());    
	result += L"-Copy.txt"; 
	ofstream ofs(result, std::ios_base::app);
	for (int i = 0; i < FileWords.size(); i++) 
	{ 
		string word(FileWords[i].begin(), FileWords[i].end()); //convert wstring to string
		ofs << word << " ";
	}
	ofs.close();
	FileWords.clear();
}

DWORD WINAPI CreateCopy(LPVOID lp)
{
	for (int i = 0; i < FileList.size(); i++) Add_Edit_File(FileList[i]);
	return 0;
}
 
//-------------------------------------------------------------read ban words
void ReadBanWords()
{ 
	wstring buff; 
	wifstream in(FilePath);
	if (in.is_open())
	{
		while (!in.eof())
		{
			in >> buff;
			WordList.push_back(buff);
		}
		in.close();
	}
}

DWORD WINAPI ReadBan(LPVOID lp)
{
	ReadBanWords();
	return 0;
}

//Добавление бан слова
void AddBanWords(wstring Word) 
{ 
	ofstream out(FilePath, std::ios_base::app); 
	string word(Word.begin(), Word.end()); 
	out << word << endl;
	out << endl; 
	out.close();
}  
 

BOOL SearchW::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	TCHAR GUID[] = TEXT("text");
	hMutex = CreateMutex(NULL, FALSE, GUID);
	DWORD dwAnswer = WaitForSingleObject(hMutex, 0);
	InitializeCriticalSection(&cs);
	if (dwAnswer == WAIT_TIMEOUT)
	{
		MessageBox(hwnd, TEXT("Нельзя запускать более одной копии приложения!!!"), TEXT("Мьютекс"), MB_OK | MB_ICONINFORMATION);
		EndDialog(hwnd, 0);
	} 

	hProgress = GetDlgItem(hwnd, IDC_PROGRESS1);
	SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 60)); // Установка интервала для индикатора 
	SendMessage(hProgress, PBM_SETSTEP, 6, 0); // Установка шага прикращения  индикатора 
	SendMessage(hProgress, PBM_SETPOS, 0, 0); // Установка текущей позиции индикатора
	SendMessage(hProgress, PBM_SETBKCOLOR, 0, LPARAM(RGB(255, 255, 255))); // Установка цвета фона индикатора
	SendMessage(hProgress, PBM_SETBARCOLOR, 0, LPARAM(RGB(0, 255, 0))); // Установка цвета заполняемых прямоугольников



	hStatP = GetDlgItem(hwnd, IDC_STATICP);
	hPause = GetDlgItem(hwnd, IDC_PAUS);
	hAdd = GetDlgItem(hwnd, IDC_ADD);
	hUpl = GetDlgItem(hwnd, IDC_UPL);
	hEnd = GetDlgItem(hwnd, IDC_END);

	for (int i = 0; i < 7; i++) hStatic[i] = GetDlgItem(hwnd, IDC_STATIC1 + i);
	for (int i = 0; i < 2; i++) hStart[i] = GetDlgItem(hwnd, IDC_START1 + i);
	for (int i = 0; i < 2; i++) hBack[i] = GetDlgItem(hwnd, IDC_BACK1 + i);
	for (int i = 0; i < 3; i++) hEdit[i] = GetDlgItem(hwnd, IDC_EDIT1 + i);
	for (int i = 0; i < 2; i++) hList[i] = GetDlgItem(hwnd, IDC_LIST1 + i);
	for (int i = 0; i < 2; i++) hOnMain[i] = GetDlgItem(hwnd, IDC_BACK3 + i);
	for (int i = 0; i < 2; i++) hDel[i] = GetDlgItem(hwnd, IDC_DEL1 + i);
	for (int i = 0; i < 2; i++) hFile[i] = GetDlgItem(hwnd, IDC_FILE1 + i); 

	ShowWindow(hProgress, SW_HIDE);
	ShowWindow(hStart[1], SW_HIDE); 
	ShowWindow(hStatP, SW_HIDE);
	ShowWindow(hPause, SW_HIDE);
	ShowWindow(hList[1], SW_HIDE); 

	ShowWindow(hEnd, SW_HIDE);
	for (int i = 0; i < 2; i++) ShowWindow(hBack[i], SW_HIDE);
	for (int i = 0; i < 2; i++) ShowWindow(hFile[i], SW_HIDE); 
	for (int i = 0; i < 2; i++) ShowWindow(hOnMain[i], SW_HIDE);
	for (int i = 4; i < 7; i++) ShowWindow(hStatic[i], SW_HIDE);
	for (int i = 1; i < 3; i++) ShowWindow(hEdit[i], SW_HIDE); 

	ShowWindow(hUpl, SW_SHOW);
	
	return TRUE;
}

void SearchW::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	//auto fileName = ""; 
	switch (id)
	{ 
	//Кнопка СТАРТ
	case IDC_START1:
	{
		//Скрывает эл. управ. 1 окно
		for (int i = 0; i < 4; i++) ShowWindow(hStatic[i], SW_HIDE);
		for (int i = 0; i < 2; i++) ShowWindow(hDel[i], SW_HIDE);
		ShowWindow(hStart[0], SW_HIDE);
		ShowWindow(hList[0], SW_HIDE);
		ShowWindow(hAdd, SW_HIDE);
		ShowWindow(hUpl, SW_HIDE);
		ShowWindow(hEdit[0], SW_HIDE);

		//Открывает эл. управ. 2 окно 
		for (int i = 1; i < 3; i++) ShowWindow(hEdit[i], SW_SHOW); 
		for (int i = 0; i < 2; i++) ShowWindow(hFile[i], SW_SHOW);
		ShowWindow(hBack[0], SW_SHOW);
		ShowWindow(hStart[1], SW_SHOW);
		ShowWindow(hStatic[4], SW_SHOW); 
	}
	break;

	//Кнопка Начать после выбора файла
	case IDC_START2:
	{		
		HANDLE hThread;

		SendMessage(hList[1], LB_RESETCONTENT, 0, 0);

		//Скрывает эл. управ. 2 окно 
		for (int i = 1; i < 3; i++) ShowWindow(hEdit[i], SW_HIDE);
		for (int i = 0; i < 2; i++) ShowWindow(hFile[i], SW_HIDE); 
		ShowWindow(hBack[0], SW_HIDE);
		ShowWindow(hStart[1], SW_HIDE);
		ShowWindow(hStatic[4], SW_HIDE);

		//Открывает эл. управ. 3 окно  
		ShowWindow(hPause, SW_SHOW);
		ShowWindow(hBack[1], SW_SHOW);
		ShowWindow(hStatP, SW_SHOW);
		ShowWindow(hProgress, SW_SHOW);
		ShowWindow(hOnMain[0], SW_SHOW);   

		hThread = CreateThread(NULL, 0, GetAllFile, 0, 0, NULL);
		CloseHandle(hThread); 
		hThread = CreateThread(NULL, 0, Result, 0, 0, NULL);
		CloseHandle(hThread); 
		hThread = CreateThread(NULL, 0, CreateCopy, 0, 0, NULL);			 
		CloseHandle(hThread); 

		///*TCHAR name[100];
		//int length = SendMessage(hEdit[2], WM_GETTEXTLENGTH, 0, 0);
		//GetWindowText(hEdit[2], name, length + 1); 
		//vector<wstring>fileFound;
		//int countOfFilesInDirectory = 0;
		//WIN32_FIND_DATA data;
		//HANDLE hFind = FindFirstFile(name, &data);
		//if (hFind != INVALID_HANDLE_VALUE) {
		//	do {
		//		if (wcsstr(data.cFileName, L".txt"))
		//		{
		//			countOfFilesInDirectory++;
		//			fileFound.push_back(data.cFileName);
		//		}
		//	} while (FindNextFile(hFind, &data));
		//	FindClose(hFind);
		//}*/ 
		/*FILE* fp;
		fp = fopen("X.txt", "a");
		TCHAR buff[50];    
		TCHAR buff1[50];
		TCHAR buff2[8] = {'*', '*', '*' , '*' , '*' , '*' , '*' };
		TCHAR* buff4[5]; 

		int x = 0;
	    wifstream in2(TEXT("X2.txt"));
	    if (in2.is_open())
	    { 
	    	while (!in2.eof())
	    	{
	    		in2 >> buff1;
	    		buff4[x] = buff1;
				x++;
	    	}
	    }
	    in2.close();
		
		int i = 0; 
		wifstream in(TEXT("Test.txt"));
		if (in.is_open())
		{
			int y = 0;
			while (!in.eof())
			{ 
				in >> buff;
				for (int i1 = 0; i1 < x; i1++)
				{
					if (*buff == *buff4[i1])
					{
						fwrite(buff2, sizeof(_TCHAR), wcslen(buff2) + 1, fp);
						i++; 
						break;
					}  
					else y++;
				}
				if (y == x) fwrite(buff, sizeof(_TCHAR), wcslen(buff) + 1, fp); 
				y = 0;
			} 
		}
		in.close();  
		fclose(fp); */
		///*TCHAR strnumber[4];
		//wsprintf(strnumber, TEXT("%d"), i, 0);
		//MessageBox(hwnd, strnumber, TEXT("Слова бан"), MB_YESNO | MB_ICONINFORMATION); */

		for (size_t i = 0; i < 10; i++)
		{
			SendMessage(hProgress, PBM_STEPIT, 0, 0);
			Sleep(100);
		}

		ShowWindow(hPause, SW_HIDE);
		ShowWindow(hBack[1], SW_HIDE);
		ShowWindow(hStatP, SW_HIDE);
		ShowWindow(hProgress, SW_HIDE);
		ShowWindow(hOnMain[0], SW_HIDE); 

		ShowWindow(hEnd, SW_SHOW);
		ShowWindow(hList[1], SW_SHOW);
		ShowWindow(hStatic[6], SW_SHOW);
		ShowWindow(hOnMain[1], SW_SHOW);
		  
		TCHAR buff[50];
		wifstream in3(TEXT("C:\\1\\Ban_words.txt"));
		if (in3.is_open())
		{
			int y = 0;
			while (!in3.eof())
			{
				in3 >> buff;
				int index = SendMessage(hList[1], LB_FINDSTRINGEXACT, -1, LPARAM(buff));
				SendMessage(hList[1], LB_ADDSTRING, 0, LPARAM(buff)); 
			}
		}
		in3.close(); 
	}
	break;

	//Кнопка Назад с 2 окна
	case IDC_BACK2:
	{
		//Открывает эл. управ. 2 окно  
		for (int i = 1; i < 3; i++) ShowWindow(hEdit[i], SW_SHOW); 
		for (int i = 0; i < 2; i++) ShowWindow(hFile[i], SW_SHOW); 
		ShowWindow(hBack[0], SW_SHOW);
		ShowWindow(hStart[1], SW_SHOW);
		ShowWindow(hStatic[4], SW_SHOW);

		//Скрывает эл. управ. 3 окно  
		ShowWindow(hPause, SW_HIDE);
		ShowWindow(hBack[1], SW_HIDE);
		ShowWindow(hStatP, SW_HIDE);
		ShowWindow(hProgress, SW_HIDE);
		ShowWindow(hOnMain[0], SW_HIDE);
	}
	break;

	//Кнопка Назад с 3 окна и Кнопки На главную
	case IDC_BACK1: case IDC_BACK3: case IDC_BACK4:
	{
		//Открывает эл. управ. 1 окно  
		for (int i = 0; i < 4; i++) ShowWindow(hStatic[i], SW_SHOW);
		for (int i = 0; i < 2; i++) ShowWindow(hDel[i], SW_SHOW);
		ShowWindow(hStart[0], SW_SHOW);
		ShowWindow(hList[0], SW_SHOW);
		ShowWindow(hAdd, SW_SHOW);
		ShowWindow(hUpl, SW_SHOW);
		ShowWindow(hEdit[0], SW_SHOW);

		//Скрывает эл. управ. 2 окно, 3 окно, 4 окно
		for (int i = 1; i < 3; i++) ShowWindow(hEdit[i], SW_HIDE);
		for (int i = 0; i < 2; i++) ShowWindow(hFile[i], SW_HIDE); 
		ShowWindow(hBack[0], SW_HIDE);
		ShowWindow(hStart[1], SW_HIDE);
		ShowWindow(hStatic[4], SW_HIDE);
		ShowWindow(hPause, SW_HIDE);
		ShowWindow(hBack[1], SW_HIDE);
		ShowWindow(hStatP, SW_HIDE);
		ShowWindow(hProgress, SW_HIDE);
		ShowWindow(hOnMain[0], SW_HIDE);

		ShowWindow(hEnd, SW_HIDE);
		ShowWindow(hList[1], SW_HIDE); 
		ShowWindow(hStatic[6], SW_HIDE);
		ShowWindow(hOnMain[1], SW_HIDE);
	}
	break; 

	//Загружает из файла запрещенные слова
	case IDC_UPL:
	{
		FILE* fp;
		fp = fopen("BadWords.txt", "a");
		TCHAR buff[50];
		SendMessage(hList[0], LB_RESETCONTENT, 0, 0);
		wifstream in(TEXT("Rezume.txt"));
		if (in.is_open())
		{
			while (!in.eof())
			{
				in >> buff;
				SendMessage(hList[0], LB_ADDSTRING, 0, LPARAM(buff));
				fwrite(buff, sizeof(_TCHAR), _tcslen(buff) + 1, fp);
			}
			in.close();
		} 
		fclose(fp);
	}
	break;

	//Добавить запрещенное слово в файл и лист бокс
	case IDC_ADD:
	{
		TCHAR buff[20];
		GetWindowText(hEdit[0], buff, 20);
		wstring word(buff);
		SetWindowTextW(hEdit[0], L" "); 
		AddBanWords(word);

		HANDLE hThread = CreateThread(NULL, 0, ReadBan, 0, 0, NULL);
		CloseHandle(hThread);
		 
		SendMessage(hList[0], LB_RESETCONTENT, 0, 0);
		wifstream in(FilePath);
		if (in.is_open())
		{
			while (!in.eof())
			{
				in >> buff;
				SendMessage(hList[0], LB_ADDSTRING, 0, LPARAM(buff));
			}
			in.close();
		} 
	}
	break;

	//Удалить запрещенное слово
	case IDC_DEL1:
	{
		int index = SendMessage(hList[0], LB_GETCURSEL, 0, 0);
		if (index != LB_ERR)
		{
			int length = SendMessage(hList[0], LB_GETTEXTLEN, index, 0);
			TCHAR* pBuffer = new TCHAR[length + 1];
			SendMessage(hList[0], LB_GETTEXT, index, LPARAM(pBuffer));
			MessageBox(hwnd, pBuffer, TEXT("Удаление слова"), MB_OK | MB_ICONINFORMATION);
			SendMessage(hList[0], LB_DELETESTRING, index, 0);
			delete[] pBuffer;
		}
		else MessageBox(hwnd, TEXT("Слово не выбрано!"), TEXT("Удаление слова"), MB_OK | MB_ICONSTOP);
	}
	break;

	//Очистить список слов 
	case IDC_DEL2: SendMessage(hList[0], LB_RESETCONTENT, 0, 0); 
		break; 

	//Выбор папки
	case IDC_FILE2: OnBrowse();  
		break;

    //Выход
	case IDC_END:  exit(0); 
		break;

	default:
		break;
	}
}

BOOL CALLBACK SearchW::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	case WM_CTLCOLORLISTBOX:
		HDC hdcStatic = (HDC)wParam;
		SetTextColor(hdcStatic, RGB(120, 0, 0));
		return (INT_PTR)hBrush;
	}
	return FALSE;
} 