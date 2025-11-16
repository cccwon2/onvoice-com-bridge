#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <combaseapi.h>
#include "ProcessLoopbackCapture.h"

#pragma comment(lib, "mmdevapi.lib")
#pragma comment(lib, "avrt.lib")

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

    // ========== 2. PID 입력 ==========
    printf("[2/6] 캡처할 프로세스 PID 입력\n");
    printf("힌트: Discord/Chrome을 실행한 후 작업 관리자에서 PID 확인\n");
    printf("PID: ");

    DWORD targetPid = 0;
    std::cin >> targetPid;

    if (targetPid == 0) {
        printf("❌ 잘못된 PID\n");
        CoUninitialize();
        return 1;
    }
    printf("✅ 타깃 PID: %lu\n\n", targetPid);

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