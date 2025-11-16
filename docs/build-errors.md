# 빌드 에러 해결 기록 🔧

개발 중 발생한 빌드 에러와 해결 방법을 기록합니다.

---

## 목차

- [링커 에러 (LNK)](#링커-에러-lnk)
- [컴파일 에러 (C)](#컴파일-에러-c)
- [COM 에러](#com-에러)
- [ATL 에러](#atl-에러)

---

## 링커 에러 (LNK)

### LNK2019: unresolved external symbol

**예시 에러 메시지**:

```
error LNK2019: unresolved external symbol __imp_CoCreateInstance referenced in function main
```

**원인**: 필요한 `.lib` 파일이 링크되지 않음

**해결 방법**:

```
1. 프로젝트 우클릭 → 속성
2. 구성: 모든 구성
3. 링커 → 입력 → 추가 종속성
4. 필요한 라이브러리 추가 (예: mmdevapi.lib;avrt.lib)
5. 적용 → 확인
```

**관련 라이브러리**:

- WASAPI: `mmdevapi.lib`, `avrt.lib`
- COM: 대부분 Windows SDK에 포함되어 자동 링크

**날짜**: (에러 발생 시 기록)

---

## 컴파일 에러 (C)

(발생 시 추가 예정)

---

## COM 에러

### E_NOTINITIALIZED (0x800401F0)

**증상**: COM 객체 생성 시 에러

**원인**: `CoInitialize()` 호출하지 않음

**해결**:

```cpp
int main() {
    // 제일 먼저!
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("COM 초기화 실패\n");
        return 1;
    }

    // ... COM 작업 ...

    // 제일 마지막!
    CoUninitialize();
    return 0;
}
```

**날짜**: (에러 발생 시 기록)

---

## ATL 에러

(Phase 7 이후 추가 예정)

---

## 🔍 에러 검색 팁

1. **에러 코드 16진수 변환**: `0x80004005` → Google 검색
2. **MSDN 문서**: https://learn.microsoft.com/en-us/windows/win32/api/
3. **Stack Overflow**: 키워드 `[winapi] [atl] [에러코드]`

---

**마지막 업데이트**: 2025-11-16
