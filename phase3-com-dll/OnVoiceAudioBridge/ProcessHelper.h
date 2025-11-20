// ProcessHelper.h
// 프로세스 찾기 유틸리티 함수들
#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>

// Windows 내부 구조체 정의
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PVOID PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PVOID Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    BYTE Reserved4[104];
    PVOID Reserved5[52];
    PVOID PostProcessInitRoutine;
    BYTE Reserved6[128];
    PVOID Reserved7[1];
    ULONG SessionId;
} PEB, * PPEB;

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation = 0
} PROCESSINFOCLASS;

// 프로세스 명령줄 가져오기
std::wstring GetProcessCommandLine(DWORD pid);

// Chrome 브라우저 프로세스 찾기 (--type= 플래그 없는 것)
DWORD FindChromeBrowserProcess();

// Edge 브라우저 프로세스 찾기
DWORD FindEdgeBrowserProcess();

// Discord 프로세스 찾기
DWORD FindDiscordProcess();

