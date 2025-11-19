// ProcessHelper.cpp
// 프로세스 찾기 유틸리티 함수들 (AudioCaptureTest.cpp 참고)
#include "pch.h"
#include "ProcessHelper.h"
#include <vector>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#pragma comment(lib, "psapi.lib")

// ⭐ 한글 출력을 위한 유틸리티 함수
static void PrintUtf8(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

// 프로세스 명령줄 가져오기
std::wstring GetProcessCommandLine(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess)
    {
        return L"";
    }

    // PEB 주소 가져오기
    PROCESS_BASIC_INFORMATION pbi = {};  // ⭐ 초기화 추가
    typedef NTSTATUS(WINAPI* NtQueryInformationProcessPtr)(
        HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll)
    {
        CloseHandle(hProcess);
        return L"";
    }

    auto NtQueryInformationProcess = (NtQueryInformationProcessPtr)
        GetProcAddress(ntdll, "NtQueryInformationProcess");

    if (!NtQueryInformationProcess)
    {
        CloseHandle(hProcess);
        return L"";
    }

    NTSTATUS status = NtQueryInformationProcess(
        hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr);

    if (status != 0)  // 0 = STATUS_SUCCESS
    {
        CloseHandle(hProcess);
        return L"";
    }

    // ⭐ NULL 포인터 체크 추가
    if (!pbi.PebBaseAddress)
    {
        CloseHandle(hProcess);
        return L"";
    }

    // PEB 읽기
    PEB peb = {};  // ⭐ 초기화 추가
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), &bytesRead))
    {
        CloseHandle(hProcess);
        return L"";
    }

    // ⭐ NULL 포인터 체크 추가
    if (!peb.ProcessParameters)
    {
        CloseHandle(hProcess);
        return L"";
    }

    // RTL_USER_PROCESS_PARAMETERS 읽기
    RTL_USER_PROCESS_PARAMETERS params = {};  // ⭐ 초기화 추가
    if (!ReadProcessMemory(hProcess, peb.ProcessParameters, &params, sizeof(params), &bytesRead))
    {
        CloseHandle(hProcess);
        return L"";
    }

    // ⭐ CommandLine 유효성 체크 추가
    if (!params.CommandLine.Buffer || params.CommandLine.Length == 0)
    {
        CloseHandle(hProcess);
        return L"";
    }

    // CommandLine 읽기
    std::wstring cmdLine;
    cmdLine.resize(params.CommandLine.Length / sizeof(wchar_t));

    if (ReadProcessMemory(hProcess, params.CommandLine.Buffer,
        &cmdLine[0], params.CommandLine.Length, &bytesRead))
    {
        CloseHandle(hProcess);
        return cmdLine;
    }

    CloseHandle(hProcess);
    return L"";
}

// Chrome 브라우저 프로세스 찾기 (--type= 플래그 없는 것)
DWORD FindChromeBrowserProcess()
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        PrintUtf8("[ProcessHelper] CreateToolhelp32Snapshot failed (error=%lu)\n", GetLastError());
        return 0;
    }

    PROCESSENTRY32W pe32 = {};  // ⭐ 초기화 추가
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    std::vector<DWORD> chromePids;

    // 모든 chrome.exe 프로세스 수집
    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            std::wstring exeName = pe32.szExeFile;
            if (exeName == L"chrome.exe")
            {
                chromePids.push_back(pe32.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    if (chromePids.empty())
    {
        PrintUtf8("[ProcessHelper] Chrome not found\n");
        return 0;
    }

    PrintUtf8("[ProcessHelper] Chrome process count: %zu, analyzing...\n", chromePids.size());

    // 각 프로세스의 명령줄 확인
    for (DWORD pid : chromePids)
    {
        std::wstring cmdLine = GetProcessCommandLine(pid);

        // --type= 플래그가 없으면 브라우저 프로세스!
        if (!cmdLine.empty() && cmdLine.find(L"--type=") == std::wstring::npos)
        {
            PrintUtf8("[ProcessHelper] Chrome browser process found: PID %lu\n", pid);
            return pid;
        }
    }

    // 명령줄 확인 실패 시 fallback: 메모리 가장 큰 것
    PrintUtf8("[ProcessHelper] CommandLine check failed, finding by memory size...\n");

    SIZE_T maxMemory = 0;
    DWORD browserPid = 0;

    for (DWORD pid : chromePids)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess)
        {
            PROCESS_MEMORY_COUNTERS pmc = {};  // ⭐ 초기화 추가
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
            {
                if (pmc.WorkingSetSize > maxMemory)
                {
                    maxMemory = pmc.WorkingSetSize;
                    browserPid = pid;
                }
            }
            CloseHandle(hProcess);
        }
    }

    if (browserPid > 0)
    {
        PrintUtf8("[ProcessHelper] Chrome main process (by memory): PID %lu (%.1f MB)\n",
            browserPid, maxMemory / 1024.0 / 1024.0);
    }
    else
    {
        PrintUtf8("[ProcessHelper] Chrome browser process not found\n");
    }

    return browserPid;
}

// Discord 프로세스 찾기
DWORD FindDiscordProcess()
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        PrintUtf8("[ProcessHelper] CreateToolhelp32Snapshot failed (error=%lu)\n", GetLastError());
        return 0;
    }

    PROCESSENTRY32W pe32 = {};  // ⭐ 초기화 추가
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            std::wstring exeName = pe32.szExeFile;
            if (exeName == L"Discord.exe")
            {
                DWORD pid = pe32.th32ProcessID;
                CloseHandle(hSnapshot);
                PrintUtf8("[ProcessHelper] Discord found: PID %lu\n", pid);
                return pid;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    PrintUtf8("[ProcessHelper] Discord not found\n");
    return 0;
}