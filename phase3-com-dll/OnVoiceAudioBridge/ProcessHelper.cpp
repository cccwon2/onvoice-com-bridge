// ProcessHelper.cpp
// 프로세스 찾기 유틸리티 함수들 구현부
// 수정사항: Discord 프로세스 찾기 로직 강화 (단순 이름 매칭 -> 메모리/명령줄 분석)

#include "pch.h"
#include "ProcessHelper.h"
#include <vector>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

// 메모리 사용량 확인 함수(GetProcessMemoryInfo)를 위해 필요
#pragma comment(lib, "psapi.lib")

// =============================================================
// 내부 유틸리티 함수
// =============================================================

// 한글 출력을 위한 유틸리티 함수
static void PrintUtf8(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

// 프로세스 명령줄(Command Line) 가져오기
// NTAPI를 직접 호출하여 외부 프로세스의 메모리(PEB)를 읽습니다.
std::wstring GetProcessCommandLine(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess)
    {
        return L"";
    }

    // PEB 주소 가져오기
    PROCESS_BASIC_INFORMATION pbi = {};
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

    if (status != 0) // 0 = STATUS_SUCCESS
    {
        CloseHandle(hProcess);
        return L"";
    }

    if (!pbi.PebBaseAddress)
    {
        CloseHandle(hProcess);
        return L"";
    }

    // PEB 읽기
    PEB peb = {};
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), &bytesRead))
    {
        CloseHandle(hProcess);
        return L"";
    }

    if (!peb.ProcessParameters)
    {
        CloseHandle(hProcess);
        return L"";
    }

    // RTL_USER_PROCESS_PARAMETERS 읽기
    RTL_USER_PROCESS_PARAMETERS params = {};
    if (!ReadProcessMemory(hProcess, peb.ProcessParameters, &params, sizeof(params), &bytesRead))
    {
        CloseHandle(hProcess);
        return L"";
    }

    if (!params.CommandLine.Buffer || params.CommandLine.Length == 0)
    {
        CloseHandle(hProcess);
        return L"";
    }

    // CommandLine 문자열 읽기
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


// =============================================================
// 공개 함수 구현
// =============================================================

// Chrome 브라우저 프로세스 찾기 (--type= 플래그 없는 것)
DWORD FindChromeBrowserProcess()
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        PrintUtf8("[ProcessHelper] CreateToolhelp32Snapshot failed (error=%lu)\n", GetLastError());
        return 0;
    }

    PROCESSENTRY32W pe32 = {};
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    std::vector<DWORD> chromePids;

    // 1. 모든 chrome.exe 프로세스 수집
    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            std::wstring exeName = pe32.szExeFile;
            // 대소문자 구분 없이 비교 (_wcsicmp)
            if (_wcsicmp(exeName.c_str(), L"chrome.exe") == 0)
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

    // 2. 각 프로세스의 명령줄 확인
    for (DWORD pid : chromePids)
    {
        std::wstring cmdLine = GetProcessCommandLine(pid);

        // --type= 플래그가 없으면 메인 브라우저 프로세스일 확률이 높음
        if (!cmdLine.empty() && cmdLine.find(L"--type=") == std::wstring::npos)
        {
            PrintUtf8("[ProcessHelper] Chrome browser process found: PID %lu\n", pid);
            return pid;
        }
    }

    // 3. 명령줄 확인 실패 시 fallback: 메모리 가장 큰 것
    PrintUtf8("[ProcessHelper] CommandLine check failed, finding by memory size...\n");

    SIZE_T maxMemory = 0;
    DWORD browserPid = 0;

    for (DWORD pid : chromePids)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess)
        {
            PROCESS_MEMORY_COUNTERS pmc = {};
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

// Edge 브라우저 프로세스 찾기 (--type= 플래그 없는 것)
DWORD FindEdgeBrowserProcess()
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        PrintUtf8("[ProcessHelper] CreateToolhelp32Snapshot failed (error=%lu)\n", GetLastError());
        return 0;
    }

    PROCESSENTRY32W pe32 = {};
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    std::vector<DWORD> edgePids;

    // 1. 모든 msedge.exe 수집
    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            std::wstring exeName = pe32.szExeFile;
            if (_wcsicmp(exeName.c_str(), L"msedge.exe") == 0)
            {
                edgePids.push_back(pe32.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    if (edgePids.empty())
    {
        PrintUtf8("[ProcessHelper] Edge not found\n");
        return 0;
    }

    PrintUtf8("[ProcessHelper] Edge process count: %zu, analyzing...\n", edgePids.size());

    // 2. 명령줄 확인 (--type= 없는 것 찾기)
    for (DWORD pid : edgePids)
    {
        std::wstring cmdLine = GetProcessCommandLine(pid);

        if (!cmdLine.empty() && cmdLine.find(L"--type=") == std::wstring::npos)
        {
            PrintUtf8("[ProcessHelper] Edge browser process found: PID %lu\n", pid);
            return pid;
        }
    }

    // 3. 실패 시 메모리 가장 큰 것 (Fallback)
    PrintUtf8("[ProcessHelper] CommandLine check failed, finding by memory size...\n");

    SIZE_T maxMemory = 0;
    DWORD browserPid = 0;

    for (DWORD pid : edgePids)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess)
        {
            PROCESS_MEMORY_COUNTERS pmc = {};
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
        PrintUtf8("[ProcessHelper] Edge main process (by memory): PID %lu (%.1f MB)\n",
            browserPid, maxMemory / 1024.0 / 1024.0);
    }
    else
    {
        PrintUtf8("[ProcessHelper] Edge browser process not found\n");
    }

    return browserPid;
}

// Discord 프로세스 찾기
// 수정됨: Electron 앱 특성상 여러 프로세스가 뜨므로, 메인 프로세스를 선별하도록 로직 강화
DWORD FindDiscordProcess()
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        PrintUtf8("[ProcessHelper] CreateToolhelp32Snapshot failed (error=%lu)\n", GetLastError());
        return 0;
    }

    PROCESSENTRY32W pe32 = {};
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    std::vector<DWORD> discordPids;

    // 1. 모든 Discord.exe PID 수집
    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            std::wstring exeName = pe32.szExeFile;
            // 대소문자 무관 비교 (보통 "Discord.exe")
            if (_wcsicmp(exeName.c_str(), L"Discord.exe") == 0)
            {
                discordPids.push_back(pe32.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    if (discordPids.empty())
    {
        PrintUtf8("[ProcessHelper] Discord not found\n");
        return 0;
    }

    PrintUtf8("[ProcessHelper] Discord process count: %zu, analyzing...\n", discordPids.size());

    // 2. 명령줄 확인 (--type= 없는 것이 메인 프로세스일 확률 높음)
    for (DWORD pid : discordPids)
    {
        std::wstring cmdLine = GetProcessCommandLine(pid);

        if (!cmdLine.empty() && cmdLine.find(L"--type=") == std::wstring::npos)
        {
            PrintUtf8("[ProcessHelper] Discord Main process found (by cmdline): PID %lu\n", pid);
            return pid;
        }
    }

    // 3. 실패 시 메모리 가장 큰 것 (Fallback)
    // Discord도 브라우저 기반이므로 메인 렌더러나 GPU 프로세스가 메모리를 많이 먹을 수 있음
    PrintUtf8("[ProcessHelper] Discord cmdline check failed, finding by memory size...\n");

    SIZE_T maxMemory = 0;
    DWORD targetPid = 0;

    for (DWORD pid : discordPids)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess)
        {
            PROCESS_MEMORY_COUNTERS pmc = {};
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
            {
                if (pmc.WorkingSetSize > maxMemory)
                {
                    maxMemory = pmc.WorkingSetSize;
                    targetPid = pid;
                }
            }
            CloseHandle(hProcess);
        }
    }

    if (targetPid > 0)
    {
        PrintUtf8("[ProcessHelper] Discord Main process (by memory): PID %lu (%.1f MB)\n",
            targetPid, maxMemory / 1024.0 / 1024.0);
    }
    else
    {
        PrintUtf8("[ProcessHelper] Discord process found but specific target missed.\n");
    }

    return targetPid;
}