#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <combaseapi.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include "ProcessLoopbackCapture.h"

#pragma comment(lib, "mmdevapi.lib")
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "psapi.lib")

// ========== 전역 변수 ==========
std::vector<unsigned char> g_audioBuffer;  // 캡처된 오디오 저장
size_t g_totalBytes = 0;                   // 총 바이트 수
bool g_isCapturing = false;                // 캡처 중 플래그

// ========== 오디오 콜백 함수 ==========
void AudioCallback(
    const std::vector<unsigned char>::iterator& begin,
    const std::vector<unsigned char>::iterator& end,
    void* pUserData)
{
    // 버퍼에 데이터 추가
    g_audioBuffer.insert(g_audioBuffer.end(), begin, end);

    size_t chunkSize = std::distance(begin, end);
    g_totalBytes += chunkSize;

    // 5초마다 상태 출력 (대략)
    static int callCount = 0;
    if (++callCount % 100 == 0) {
        // RMS 볼륨 계산 (16bit PCM 기준)
        int16_t* samples = (int16_t*)&(*begin);
        size_t numSamples = chunkSize / 2;

        double sum = 0.0;
        for (size_t i = 0; i < numSamples; i++) {
            double sample = samples[i] / 32768.0;  // -1.0 ~ 1.0 정규화
            sum += sample * sample;
        }

        double rms = sqrt(sum / numSamples);
        int volumeBars = (int)(rms * 50);  // 볼륨 막대 (0-50)

        printf("[%6.2f MB] Volume: ", g_totalBytes / 1024.0 / 1024.0);
        for (int i = 0; i < volumeBars; i++) printf("█");
        printf("\n");
    }
}

// ========== WAV 헤더 작성 ==========
void WriteWavHeader(std::ofstream& file, uint32_t dataSize, uint32_t sampleRate, uint16_t channels, uint16_t bitsPerSample) {
    uint32_t byteRate = sampleRate * channels * bitsPerSample / 8;
    uint16_t blockAlign = channels * bitsPerSample / 8;

    // RIFF 헤더
    file.write("RIFF", 4);
    uint32_t chunkSize = 36 + dataSize;
    file.write((char*)&chunkSize, 4);
    file.write("WAVE", 4);

    // fmt 청크
    file.write("fmt ", 4);
    uint32_t subchunk1Size = 16;
    file.write((char*)&subchunk1Size, 4);
    uint16_t audioFormat = 1;  // PCM
    file.write((char*)&audioFormat, 2);
    file.write((char*)&channels, 2);
    file.write((char*)&sampleRate, 4);
    file.write((char*)&byteRate, 4);
    file.write((char*)&blockAlign, 2);
    file.write((char*)&bitsPerSample, 2);

    // data 청크
    file.write("data", 4);
    file.write((char*)&dataSize, 4);
}

// ========== Chrome 메인 프로세스 자동 찾기 ==========

// ========== Windows 내부 구조체 정의 ==========

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
std::wstring GetProcessCommandLine(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) {
        return L"";
    }

    // PEB 주소 가져오기
    PROCESS_BASIC_INFORMATION pbi;
    typedef NTSTATUS(WINAPI* NtQueryInformationProcessPtr)(
        HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    auto NtQueryInformationProcess = (NtQueryInformationProcessPtr)
        GetProcAddress(ntdll, "NtQueryInformationProcess");

    if (!NtQueryInformationProcess) {
        CloseHandle(hProcess);
        return L"";
    }

    NTSTATUS status = NtQueryInformationProcess(
        hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr);

    if (status != 0) {
        CloseHandle(hProcess);
        return L"";
    }

    // PEB 읽기
    PEB peb;
    SIZE_T bytesRead;
    if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), &bytesRead)) {
        CloseHandle(hProcess);
        return L"";
    }

    // RTL_USER_PROCESS_PARAMETERS 읽기
    RTL_USER_PROCESS_PARAMETERS params;
    if (!ReadProcessMemory(hProcess, peb.ProcessParameters, &params, sizeof(params), &bytesRead)) {
        CloseHandle(hProcess);
        return L"";
    }

    // CommandLine 읽기
    std::wstring cmdLine;
    cmdLine.resize(params.CommandLine.Length / sizeof(wchar_t));

    if (ReadProcessMemory(hProcess, params.CommandLine.Buffer,
        &cmdLine[0], params.CommandLine.Length, &bytesRead)) {
        CloseHandle(hProcess);
        return cmdLine;
    }

    CloseHandle(hProcess);
    return L"";
}

// Chrome 브라우저 프로세스 찾기 (--type= 플래그 없는 것)
DWORD FindChromeBrowserProcess() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    std::vector<DWORD> chromePids;

    // 모든 chrome.exe 프로세스 수집
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            std::wstring exeName = pe32.szExeFile;
            if (exeName == L"chrome.exe") {
                chromePids.push_back(pe32.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    printf("Chrome 프로세스 %zu개 발견, 분석 중...\n", chromePids.size());

    // 각 프로세스의 명령줄 확인
    for (DWORD pid : chromePids) {
        std::wstring cmdLine = GetProcessCommandLine(pid);

        // --type= 플래그가 없으면 브라우저 프로세스!
        if (!cmdLine.empty() && cmdLine.find(L"--type=") == std::wstring::npos) {
            printf("✅ Chrome 브라우저 프로세스 발견: PID %lu\n", pid);
            return pid;
        }
    }

    // 명령줄 확인 실패 시 fallback: 메모리 가장 큰 것
    printf("⚠️  명령줄 확인 실패, 메모리 기준으로 찾는 중...\n");

    SIZE_T maxMemory = 0;
    DWORD browserPid = 0;

    for (DWORD pid : chromePids) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                if (pmc.WorkingSetSize > maxMemory) {
                    maxMemory = pmc.WorkingSetSize;
                    browserPid = pid;
                }
            }
            CloseHandle(hProcess);
        }
    }

    if (browserPid > 0) {
        printf("✅ Chrome 메인 프로세스 (메모리 기준): PID %lu (%.1f MB)\n",
            browserPid, maxMemory / 1024.0 / 1024.0);
    }

    return browserPid;
}

// Discord 프로세스 찾기
DWORD FindDiscordProcess() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            std::wstring exeName = pe32.szExeFile;
            if (exeName == L"Discord.exe") {
                DWORD pid = pe32.th32ProcessID;
                CloseHandle(hSnapshot);
                printf("✅ Discord 발견: PID %lu\n", pid);
                return pid;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return 0;
}

// ========== 메인 함수 ==========
int main()
{
    printf("===========================================\n");
    printf("  OnVoice Audio Capture Test (PoC)\n");
    printf("===========================================\n\n");

    // ========== 1. COM 초기화 ==========
    printf("[1/6] COM 초기화 중...\n");
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        printf("❌ COM 초기화 실패: 0x%X\n", hr);
        return 1;
    }
    printf("✅ COM 초기화 성공\n\n");

    // ========== 2. 프로세스 선택 ==========
    printf("[2/6] 캡처할 프로세스 선택\n");
    printf("1. Discord (자동 찾기)\n");
    printf("2. Chrome (자동 찾기)\n");
    printf("3. 수동 PID 입력\n");
    printf("선택 (1-3): ");

    int choice = 0;
    std::cin >> choice;

    DWORD targetPid = 0;
    std::string processName;

    if (choice == 1) {
        // Discord 자동 찾기
        printf("\nDiscord 프로세스 검색 중...\n");
        targetPid = FindDiscordProcess();
        processName = "Discord";

        if (targetPid == 0) {
            printf("❌ Discord가 실행 중이 아닙니다.\n");
            printf("   Discord를 먼저 실행한 후 다시 시도하세요.\n");
            CoUninitialize();
            return 1;
        }

    }
    else if (choice == 2) {
        // Chrome 자동 찾기
        printf("\nChrome 브라우저 프로세스 검색 중...\n");
        targetPid = FindChromeBrowserProcess();
        processName = "Chrome";

        if (targetPid == 0) {
            printf("❌ Chrome이 실행 중이 아닙니다.\n");
            printf("   Chrome을 먼저 실행한 후 다시 시도하세요.\n");
            CoUninitialize();
            return 1;
        }

    }
    else if (choice == 3) {
        // 수동 입력
        printf("\n힌트: Discord/Chrome을 실행한 후 작업 관리자에서 PID 확인\n");
        printf("      Chrome: Shift+ESC → 브라우저 프로세스 PID\n");
        printf("PID: ");
        std::cin >> targetPid;
        processName = "수동 입력";

        if (targetPid == 0) {
            printf("❌ 잘못된 PID\n");
            CoUninitialize();
            return 1;
        }

    }
    else {
        printf("❌ 잘못된 선택\n");
        CoUninitialize();
        return 1;
    }

    printf("\n✅ 타깃: %s (PID: %lu)\n\n", processName.c_str(), targetPid);

    // ========== 3. 캡처 설정 ==========
    printf("[3/6] 캡처 설정 중...\n");

    ProcessLoopbackCapture capture;

    // 16kHz, 16bit, Mono, PCM
    eCaptureError error = capture.SetCaptureFormat(
        16000,               // Sample Rate
        16,                  // Bits Per Sample
        1,                   // Channels (Mono)
        WAVE_FORMAT_PCM      // PCM 포맷
    );

    if (error != eCaptureError::NONE) {
        printf("❌ 포맷 설정 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(error));
        CoUninitialize();
        return 1;
    }

    // 타깃 프로세스 설정
    error = capture.SetTargetProcess(targetPid);
    if (error != eCaptureError::NONE) {
        printf("❌ 프로세스 설정 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(error));
        CoUninitialize();
        return 1;
    }

    // 콜백 설정
    capture.SetCallback(&AudioCallback, nullptr);

    printf("✅ 설정 완료 (16kHz, 16bit, Mono)\n\n");

    // ========== 4. 캡처 시작 ==========
    printf("[4/6] 캡처 시작 중...\n");
    printf("⚠️  이 단계는 수백 ms 걸릴 수 있습니다 (블로킹)\n");

    error = capture.StartCapture();

    if (error != eCaptureError::NONE) {
        printf("❌ 캡처 시작 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(error));
        printf("HRESULT: 0x%X\n", capture.GetLastErrorResult());

        // 일반적인 에러 원인
        printf("\n가능한 원인:\n");
        printf("1. PID가 잘못됨 (프로세스가 종료됨)\n");
        printf("2. 해당 프로세스가 오디오를 재생하지 않음\n");
        printf("3. 오디오 디바이스 문제\n");

        CoUninitialize();
        return 1;
    }

    g_isCapturing = true;
    printf("✅ 캡처 시작 성공!\n\n");

    // ========== 5. 캡처 진행 ==========
    printf("[5/6] 오디오 캡처 중... (10초)\n");
    printf("Discord/Chrome에서 소리가 나도록 하세요!\n");
    printf("(YouTube 재생, Discord 음성 채팅 등)\n\n");

    // 10초 대기
    for (int i = 10; i > 0; i--) {
        printf("\r남은 시간: %d초  ", i);
        fflush(stdout);
        Sleep(1000);
    }
    printf("\n\n");

    // ========== 6. 캡처 정지 ==========
    printf("[6/6] 캡처 정지 중...\n");

    error = capture.StopCapture();
    if (error != eCaptureError::NONE) {
        printf("⚠️  정지 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(error));
    }
    else {
        printf("✅ 캡처 정지 완료\n\n");
    }

    g_isCapturing = false;

    // ========== 결과 출력 ==========
    printf("===========================================\n");
    printf("  캡처 결과\n");
    printf("===========================================\n");
    printf("총 캡처 데이터: %.2f MB\n", g_totalBytes / 1024.0 / 1024.0);
    printf("총 샘플 수: %zu\n", g_totalBytes / 2);  // 16bit = 2 bytes
    printf("캡처 시간: ~10초\n");
    printf("예상 파일 크기: ~320 KB (16kHz * 2bytes * 10s)\n\n");

    // ========== WAV 파일 저장 ==========
    if (g_totalBytes > 0) {
        printf("WAV 파일로 저장 중...\n");

        std::ofstream wavFile("captured_audio.wav", std::ios::binary);
        if (!wavFile) {
            printf("❌ 파일 생성 실패\n");
        }
        else {
            WriteWavHeader(wavFile, (uint32_t)g_audioBuffer.size(), 16000, 1, 16);
            wavFile.write((char*)g_audioBuffer.data(), g_audioBuffer.size());
            wavFile.close();

            printf("✅ 파일 저장 완료: captured_audio.wav\n");
            printf("   Windows Media Player나 VLC로 재생해보세요!\n");
        }
    }
    else {
        printf("⚠️  캡처된 데이터가 없습니다.\n");
        printf("   해당 프로세스가 오디오를 재생하지 않았을 수 있습니다.\n");
    }

    // ========== 정리 ==========
    CoUninitialize();

    printf("\n아무 키나 눌러 종료...\n");
    std::cin.ignore();
    std::cin.get();

    return 0;
}