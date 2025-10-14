#include <windows.h>
#include <tchar.h>

#include <string>
#include <iostream>
CRITICAL_SECTION cs;

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	int threadNum = *(int*) lpParam;
	EnterCriticalSection(&cs);
	std::cout << "Поток № " << lpParam << " выполняет свою работу;" << std::endl;
	LeaveCriticalSection(&cs);
	ExitThread(0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "RU");
	int N;
	std::cout << "Введите количество потоков:";
	std::cin >> N;
	HANDLE* handles = new HANDLE[N];
	InitializeCriticalSection(&cs);

	for (int i = 0; i < N; i++) 
	{
		handles[i] = CreateThread(NULL, 0, ThreadProc, &i, 0, NULL);
	}

	WaitForMultipleObjects(N, handles, true, INFINITE);
	return 0;
}
