#include <windows.h>
#include <string>
#include <iostream>
#include "tchar.h"
#include <fstream>
#include <vector>

const wchar_t* MUTEX = L"Global\\MyBalanceMutex";
HANDLE hMutex = NULL;

int ReadFromFile()
{
    std::fstream myfile("balance.txt", std::ios_base::in);
    int result = 0;
    if (myfile >> result) {}
    myfile.close();
    return result;
}

void WriteToFile(int data)
{
    std::fstream myfile("balance.txt", std::ios_base::out | std::ios_base::trunc);
    myfile << data << std::endl;
    myfile.close();
}

void Deposit(int money)
{
    WaitForSingleObject(hMutex, INFINITE);

    int balance = ReadFromFile();
    balance += money;
    WriteToFile(balance);

    printf("Balance after deposit: %d\n", balance);

    ReleaseMutex(hMutex);
}

void Withdraw(int money)
{
    WaitForSingleObject(hMutex, INFINITE);

    int balance = ReadFromFile();
    if (balance < money)
    {
        printf("Cannot withdraw money, balance lower than %d\n", money);
        ReleaseMutex(hMutex);
        return;
    }

    Sleep(20);
    balance -= money;
    WriteToFile(balance);

    printf("Balance after withdraw: %d\n", balance);

    ReleaseMutex(hMutex);
}

DWORD WINAPI DoDeposit(LPVOID lpParameter)
{
    Deposit((int)(INT_PTR)lpParameter);
    return 0;
}

DWORD WINAPI DoWithdraw(LPVOID lpParameter)
{
    Withdraw((int)(INT_PTR)lpParameter);
    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
    const int THREAD_COUNT = 50;
    std::vector<HANDLE> handles(THREAD_COUNT);

    bool firstProcess = false;
    hMutex = CreateMutexW(NULL, FALSE, L"Global\\MyBalanceMutex");
    if (!hMutex)
    {
        std::cerr << "CreateMutex failed: " << GetLastError() << std::endl;
        return 1;
    }
    if (GetLastError() != ERROR_ALREADY_EXISTS)
    {
        firstProcess = true;
    }


    if (firstProcess)
    {
        WaitForSingleObject(hMutex, INFINITE);
        WriteToFile(0);
        ReleaseMutex(hMutex);
    }


    SetProcessAffinityMask(GetCurrentProcess(), 1);

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        if (i % 2 == 0)
            handles[i] = CreateThread(NULL, 0, DoDeposit, (LPVOID)(INT_PTR)230, 0, NULL);
        else
            handles[i] = CreateThread(NULL, 0, DoWithdraw, (LPVOID)(INT_PTR)1000, 0, NULL);
    }

    WaitForMultipleObjects(THREAD_COUNT, handles.data(), TRUE, INFINITE);

    for (HANDLE h : handles) if (h) CloseHandle(h);

    printf("Final Balance: %d\n", ReadFromFile());

    CloseHandle(hMutex);

    getchar();
    return 0;
}
