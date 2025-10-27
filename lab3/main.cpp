#include <windows.h>
#include <tchar.h>
#include <chrono>
#include <string>
#include <iostream>
CRITICAL_SECTION cs;

const int countOperation = 10;
std::chrono::high_resolution_clock::time_point start;

void factorial(int n) 
{
	int result = 1;
	for (int i = 1; i <= n; ++i) 
	{
		result *= i;
	}
}
DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	int threadNum = *(int*)lpParam;

	for (int i = 0; i < countOperation; i++)
	{
		Sleep(10);
		factorial(100);
		auto now = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
		std::string currentTime = std::to_string(elapsed) + " ms";
		
		EnterCriticalSection(&cs);
		std::cout << threadNum << "|" << currentTime << std::endl;
		LeaveCriticalSection(&cs);
	}
	ExitThread(0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "RU");
	std::cout << "Нажмите Enter для продолжения: ";
	std::cin.get();
	start = std::chrono::high_resolution_clock::now();
	HANDLE* handles = new HANDLE[2];
	int threadNums[2] = { 1, 2 };
	InitializeCriticalSection(&cs);

	handles[0] = CreateThread(NULL, 0, ThreadProc, &threadNums[0], 0, NULL);
	handles[1] = CreateThread(NULL, 0, ThreadProc, &threadNums[1], 0, NULL);

	WaitForMultipleObjects(2, handles, true, INFINITE);

	return 0;
}
