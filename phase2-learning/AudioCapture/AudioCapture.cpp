#include <iostream>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>

// 링커 설정
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "avrt.lib")

using namespace std;

int main()
{
    cout << "=== OnVoice - 기본 오디오 캡처 테스트 ===" << endl << endl;

    // ===== 1단계: COM 초기화 =====
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        cout << "❌ COM 초기화 실패: 0x" << hex << hr << endl;
        return 1;
    }
    cout << "✅ COM 초기화 완료" << endl;

    // ===== 2단계: 디바이스 열거자 생성 =====
    IMMDeviceEnumerator* enumerator = nullptr;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&enumerator
    );

    if (FAILED(hr)) {
        cout << "❌ 디바이스 열거자 생성 실패: 0x" << hex << hr << endl;
        CoUninitialize();
        return 1;
    }
    cout << "✅ 디바이스 열거자 생성 완료" << endl;

    // ===== 3단계: 기본 재생 디바이스 가져오기 =====
    IMMDevice* device = nullptr;
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);

    if (FAILED(hr)) {
        cout << "❌ 오디오 디바이스 가져오기 실패: 0x" << hex << hr << endl;
        enumerator->Release();
        CoUninitialize();
        return 1;
    }
    cout << "✅ 기본 오디오 디바이스 획득" << endl;

    // ===== 4단계: 오디오 클라이언트 활성화 =====
    IAudioClient* audioClient = nullptr;
    hr = device->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        (void**)&audioClient
    );

    if (FAILED(hr)) {
        cout << "❌ 오디오 클라이언트 활성화 실패: 0x" << hex << hr << endl;
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }
    cout << "✅ 오디오 클라이언트 활성화 완료" << endl;

    // ===== 5단계: 오디오 형식 가져오기 =====
    WAVEFORMATEX* waveFormat = nullptr;
    hr = audioClient->GetMixFormat(&waveFormat);

    if (FAILED(hr)) {
        cout << "❌ 오디오 형식 가져오기 실패: 0x" << hex << hr << endl;
        audioClient->Release();
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    cout << "✅ 오디오 형식 정보:" << endl;
    cout << "   - 샘플링 레이트: " << waveFormat->nSamplesPerSec << " Hz" << endl;
    cout << "   - 채널 수: " << waveFormat->nChannels << endl;
    cout << "   - 비트 깊이: " << waveFormat->wBitsPerSample << " bits" << endl;

    // ===== 6단계: 오디오 클라이언트 초기화 (루프백 모드) =====
    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,           // 공유 모드
        AUDCLNT_STREAMFLAGS_LOOPBACK,       // 루프백 (스피커 출력 캡처)
        10000000,                           // 버퍼 지속 시간 (1초)
        0,                                  // 주기 (공유 모드에서는 0)
        waveFormat,                         // 오디오 형식
        nullptr                             // 세션 GUID
    );

    if (FAILED(hr)) {
        cout << "❌ 오디오 클라이언트 초기화 실패: 0x" << hex << hr << endl;
        CoTaskMemFree(waveFormat);
        audioClient->Release();
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }
    cout << "✅ 오디오 클라이언트 초기화 완료 (루프백 모드)" << endl;

    // ===== 7단계: 캡처 클라이언트 가져오기 =====
    IAudioCaptureClient* captureClient = nullptr;
    hr = audioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&captureClient
    );

    if (FAILED(hr)) {
        cout << "❌ 캡처 클라이언트 가져오기 실패: 0x" << hex << hr << endl;
        CoTaskMemFree(waveFormat);
        audioClient->Release();
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }
    cout << "✅ 캡처 클라이언트 획득 완료" << endl << endl;

    // ===== 8단계: 캡처 시작! =====
    hr = audioClient->Start();
    if (FAILED(hr)) {
        cout << "❌ 캡처 시작 실패: 0x" << hex << hr << endl;
        captureClient->Release();
        CoTaskMemFree(waveFormat);
        audioClient->Release();
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }
    cout << "🎤 오디오 캡처 시작! (5초간 캡처합니다...)" << endl << endl;

    // ===== 9단계: 5초간 오디오 데이터 캡처 =====
    DWORD startTime = GetTickCount();
    int packetCount = 0;
    UINT64 totalFrames = 0;

    while (GetTickCount() - startTime < 5000) {  // 5초
        Sleep(10);  // 10ms 대기

        // 사용 가능한 데이터 확인
        UINT32 packetLength = 0;
        hr = captureClient->GetNextPacketSize(&packetLength);

        if (FAILED(hr)) {
            break;
        }

        while (packetLength > 0) {
            BYTE* pData = nullptr;
            UINT32 numFramesAvailable = 0;
            DWORD flags = 0;

            // 버퍼에서 데이터 가져오기
            hr = captureClient->GetBuffer(
                &pData,
                &numFramesAvailable,
                &flags,
                nullptr,
                nullptr
            );

            if (SUCCEEDED(hr)) {
                packetCount++;
                totalFrames += numFramesAvailable;

                // 데이터 처리 (여기서는 첫 10개 샘플만 출력)
                if (packetCount == 1 && numFramesAvailable > 0) {
                    cout << "📊 첫 번째 패킷 데이터:" << endl;
                    cout << "   프레임 수: " << numFramesAvailable << endl;
                    cout << "   플래그: " << flags << endl;

                    if (pData != nullptr && !(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                        cout << "   첫 10바이트: ";
                        for (int i = 0; i < min(10, (int)(numFramesAvailable * waveFormat->nBlockAlign)); i++) {
                            printf("%02X ", pData[i]);
                        }
                        cout << endl;
                    }
                    cout << endl;
                }

                // 버퍼 해제
                captureClient->ReleaseBuffer(numFramesAvailable);
            }

            // 다음 패킷 확인
            hr = captureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) {
                break;
            }
        }
    }

    // ===== 10단계: 캡처 중지 =====
    audioClient->Stop();
    cout << "✅ 캡처 완료!" << endl;
    cout << "   총 패킷 수: " << packetCount << endl;
    cout << "   총 프레임 수: " << totalFrames << endl;
    cout << "   캡처 시간: 약 5초" << endl << endl;

    // ===== 11단계: 정리 (역순!) =====
    cout << "🧹 리소스 정리 중..." << endl;
    captureClient->Release();
    CoTaskMemFree(waveFormat);
    audioClient->Release();
    device->Release();
    enumerator->Release();
    CoUninitialize();
    cout << "✅ 정리 완료!" << endl << endl;

    cout << "계속하려면 Enter 키를 누르세요..." << endl;
    cin.get();
    return 0;
}