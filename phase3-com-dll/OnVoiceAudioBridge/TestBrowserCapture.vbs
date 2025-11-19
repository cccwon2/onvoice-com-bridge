Option Explicit

WScript.Echo "========================================"
WScript.Echo " 브라우저 오디오 캡처 및 WAV 파일 저장"
WScript.Echo "========================================"
WScript.Echo ""

On Error Resume Next

' =========================================
' 전역 변수: 오디오 데이터 저장
' =========================================
Dim g_audioData
ReDim g_audioData(0)
Dim g_audioDataCount
g_audioDataCount = 0

' =========================================
' 1) COM 객체 생성 (이벤트 Prefix: "OnVoice_")
' =========================================
Dim capture
WScript.Echo "[1] COM 객체 생성 중..."

Set capture = WScript.CreateObject("OnVoiceAudioBridge.OnVoiceCapture", "OnVoice_")

If Err.Number <> 0 Or capture Is Nothing Then
    WScript.Echo "❌ COM 객체 생성 실패"
    WScript.Echo "   Err.Number = " & Hex(Err.Number) & " (" & Err.Number & ")"
    WScript.Echo "   Err.Description = " & Err.Description
    WScript.Quit 1
End If

WScript.Echo "✅ COM 객체 생성 완료"
WScript.Echo ""

' =========================================
' 2) 크롬과 엣지 프로세스 찾기
' =========================================
WScript.Echo "[2] 브라우저 프로세스 검색 중..."
WScript.Echo ""

Dim chromePid, edgePid
chromePid = 0
edgePid = 0

' Chrome 찾기
Err.Clear
chromePid = capture.FindChromeProcess()
If Err.Number <> 0 Then
    WScript.Echo "⚠️  FindChromeProcess 호출 실패"
ElseIf chromePid > 0 Then
    WScript.Echo "✅ Chrome 발견: PID " & chromePid
Else
    WScript.Echo "❌ Chrome이 실행 중이 아닙니다"
End If

' Edge 찾기 (WMI 사용)
edgePid = FindEdgeProcess()
If edgePid > 0 Then
    WScript.Echo "✅ Edge 발견: PID " & edgePid
Else
    WScript.Echo "❌ Edge가 실행 중이 아닙니다"
End If

WScript.Echo ""

' 둘 다 없으면 종료
If chromePid <= 0 And edgePid <= 0 Then
    WScript.Echo "❌ Chrome 또는 Edge를 먼저 실행하고 유튜브 동영상을 재생하세요."
    WScript.Quit 1
End If

' =========================================
' 3) 브라우저 선택
' =========================================
Dim selectedPid, selectedName
selectedPid = 0
selectedName = ""

WScript.Echo "[3] 캡처할 브라우저 선택"
WScript.Echo ""

If chromePid > 0 And edgePid > 0 Then
    ' 둘 다 있으면 선택
    WScript.Echo "1. Chrome (PID: " & chromePid & ")"
    WScript.Echo "2. Edge (PID: " & edgePid & ")"
    WScript.Echo ""
    
    Dim choice
    choice = InputBox("선택 (1 또는 2):", "브라우저 선택", "1")
    
    If choice = "1" Then
        selectedPid = chromePid
        selectedName = "Chrome"
    ElseIf choice = "2" Then
        selectedPid = edgePid
        selectedName = "Edge"
    Else
        WScript.Echo "❌ 잘못된 선택: " & choice
        WScript.Quit 1
    End If
ElseIf chromePid > 0 Then
    ' Chrome만 있음
    selectedPid = chromePid
    selectedName = "Chrome"
    WScript.Echo "Chrome이 선택되었습니다 (PID: " & chromePid & ")"
ElseIf edgePid > 0 Then
    ' Edge만 있음
    selectedPid = edgePid
    selectedName = "Edge"
    WScript.Echo "Edge가 선택되었습니다 (PID: " & edgePid & ")"
End If

WScript.Echo ""
WScript.Echo "✅ 타깃: " & selectedName & " (PID: " & selectedPid & ")"
WScript.Echo ""

' =========================================
' 4) 캡처 시작
' =========================================
WScript.Echo "[4] StartCapture(" & selectedPid & ") 호출..."
Err.Clear
Dim hr
hr = capture.StartCapture(selectedPid)

If Err.Number <> 0 Then
    WScript.Echo "❌ StartCapture 호출 중 스크립트 오류"
    WScript.Echo "   Err.Number = " & Hex(Err.Number) & " (" & Err.Number & ")"
    WScript.Echo "   Err.Description = " & Err.Description
    WScript.Quit 1
End If

If hr <> 0 Then
    WScript.Echo "❌ StartCapture 실패 (HR=" & Hex(hr) & ")"
    WScript.Quit 1
End If

WScript.Echo "✅ StartCapture 성공 (HR=" & Hex(hr) & ")"
WScript.Echo ""

' =========================================
' 5) 10초 동안 캡처
' =========================================
WScript.Echo "[5] 10초 동안 오디오 캡처 중..."
WScript.Echo "⚠️  " & selectedName & "에서 유튜브 동영상이 재생 중인지 확인하세요!"
WScript.Echo ""

Dim i
For i = 10 To 1 Step -1
    WScript.Echo "남은 시간: " & i & "초... (캡처된 데이터: " & GetAudioDataSize() & " bytes)"
    WScript.Sleep 1000
Next

WScript.Echo ""
WScript.Echo "✅ 캡처 완료! 총 " & GetAudioDataSize() & " bytes 수집됨"
WScript.Echo ""

' =========================================
' 6) 캡처 중지
' =========================================
WScript.Echo "[6] StopCapture 호출..."
Err.Clear
hr = capture.StopCapture()

If Err.Number <> 0 Then
    WScript.Echo "❌ StopCapture 호출 중 스크립트 오류"
    WScript.Echo "   Err.Number = " & Hex(Err.Number) & " (" & Err.Number & ")"
    WScript.Echo "   Err.Description = " & Err.Description
Else
    WScript.Echo "✅ StopCapture 완료 (HR=" & Hex(hr) & ")"
End If

WScript.Echo ""

' =========================================
' 7) WAV 파일로 저장
' =========================================
Dim totalSize
totalSize = GetAudioDataSize()

If totalSize > 0 Then
    WScript.Echo "[7] WAV 파일로 저장 중..."
    
    Dim fileName
    fileName = "captured_" & selectedName & "_" & Year(Now) & Right("0" & Month(Now), 2) & Right("0" & Day(Now), 2) & "_" & Right("0" & Hour(Now), 2) & Right("0" & Minute(Now), 2) & Right("0" & Second(Now), 2) & ".wav"
    
    If SaveToWavFile(fileName) Then
        WScript.Echo "✅ 파일 저장 완료: " & fileName
        WScript.Echo "   Windows Media Player나 VLC로 재생해보세요!"
    Else
        WScript.Echo "❌ 파일 저장 실패"
    End If
Else
    WScript.Echo "⚠️  캡처된 데이터가 없습니다."
    WScript.Echo "   " & selectedName & "에서 오디오가 재생되지 않았을 수 있습니다."
End If

WScript.Echo ""
WScript.Echo "테스트 종료"

' =========================================
' 정리
' =========================================
Set capture = Nothing
WScript.Quit 0

' =========================================
' 함수: Edge 프로세스 찾기 (WMI 사용)
' =========================================
Function FindEdgeProcess()
    On Error Resume Next
    
    Dim wmi, processes, process, pid, cmdLine
    FindEdgeProcess = 0
    
    Set wmi = GetObject("winmgmts:\\.\root\cimv2")
    If Err.Number <> 0 Then
        Exit Function
    End If
    
    Set processes = wmi.ExecQuery("SELECT ProcessId, CommandLine FROM Win32_Process WHERE Name = 'msedge.exe'")
    
    Dim maxMemory, bestPid
    maxMemory = 0
    bestPid = 0
    
    For Each process In processes
        pid = process.ProcessId
        cmdLine = ""
        
        On Error Resume Next
        cmdLine = process.CommandLine
        If Err.Number <> 0 Then
            cmdLine = ""
        End If
        
        ' --type= 플래그가 없으면 브라우저 프로세스
        If cmdLine = "" Or InStr(cmdLine, "--type=") = 0 Then
            ' 메모리 사용량 확인 (간단한 방법: 첫 번째로 발견된 것을 사용)
            ' 더 정확하게 하려면 메모리 사용량을 확인해야 하지만, 여기서는 간단하게 처리
            If bestPid = 0 Then
                bestPid = pid
            End If
        End If
    Next
    
    FindEdgeProcess = bestPid
End Function

' =========================================
' 함수: 오디오 데이터 크기 반환
' =========================================
Function GetAudioDataSize()
    On Error Resume Next
    
    Dim size, i
    size = 0
    
    If IsArray(g_audioData) And g_audioDataCount > 0 Then
        For i = 0 To g_audioDataCount - 1
            If IsArray(g_audioData(i)) Then
                size = size + (UBound(g_audioData(i)) - LBound(g_audioData(i)) + 1)
            End If
        Next
    End If
    
    GetAudioDataSize = size
End Function

' =========================================
' 함수: WAV 파일로 저장
' =========================================
Function SaveToWavFile(fileName)
    On Error Resume Next
    
    SaveToWavFile = False
    
    ' 오디오 데이터가 없으면 실패
    Dim totalSize
    totalSize = GetAudioDataSize()
    If totalSize <= 0 Then
        Exit Function
    End If
    
    ' ADODB.Stream 생성
    Dim stream
    Set stream = CreateObject("ADODB.Stream")
    If Err.Number <> 0 Then
        WScript.Echo "  ❌ ADODB.Stream 생성 실패"
        Exit Function
    End If
    
    stream.Type = 1 ' adTypeBinary
    stream.Open
    
    ' =========================================
    ' WAV 헤더 작성 (16kHz, mono, 16-bit PCM)
    ' =========================================
    Dim sampleRate, channels, bitsPerSample
    sampleRate = 16000
    channels = 1
    bitsPerSample = 16
    
    Dim byteRate, blockAlign, subchunk1Size, audioFormat
    byteRate = sampleRate * channels * bitsPerSample / 8
    blockAlign = channels * bitsPerSample / 8
    subchunk1Size = 16
    audioFormat = 1 ' PCM
    
    ' RIFF 헤더
    WriteString stream, "RIFF"
    Dim chunkSize
    chunkSize = 36 + totalSize
    WriteLong stream, chunkSize
    WriteString stream, "WAVE"
    
    ' fmt 청크
    WriteString stream, "fmt "
    WriteLong stream, subchunk1Size
    WriteShort stream, audioFormat
    WriteShort stream, channels
    WriteLong stream, sampleRate
    WriteLong stream, byteRate
    WriteShort stream, blockAlign
    WriteShort stream, bitsPerSample
    
    ' data 청크
    WriteString stream, "data"
    WriteLong stream, totalSize
    
    ' =========================================
    ' 오디오 데이터 쓰기
    ' =========================================
    Dim i, j, dataArray
    For i = 0 To g_audioDataCount - 1
        If IsArray(g_audioData(i)) Then
            dataArray = g_audioData(i)
            For j = LBound(dataArray) To UBound(dataArray)
                Dim byteVal
                byteVal = CByte(dataArray(j) And &HFF)
                stream.Write Chr(byteVal)
            Next
        End If
    Next
    
    ' 파일 저장
    On Error Resume Next
    stream.SaveToFile fileName, 2 ' adSaveCreateOverWrite
    If Err.Number <> 0 Then
        WScript.Echo "  ❌ 파일 저장 실패: " & Err.Description
        stream.Close
        Set stream = Nothing
        Exit Function
    End If
    
    stream.Close
    Set stream = Nothing
    
    SaveToWavFile = True
End Function

' =========================================
' 헬퍼 함수: 바이너리 데이터 쓰기
' =========================================
Sub WriteString(stream, str)
    Dim i
    For i = 1 To Len(str)
        stream.Write Chr(Asc(Mid(str, i, 1)))
    Next
End Sub

Sub WriteShort(stream, value)
    Dim b1, b2
    b1 = value And &HFF
    b2 = (value \ 256) And &HFF
    stream.Write Chr(b1) & Chr(b2)
End Sub

Sub WriteLong(stream, value)
    Dim b1, b2, b3, b4
    b1 = value And &HFF
    b2 = (value \ 256) And &HFF
    b3 = (value \ 65536) And &HFF
    b4 = (value \ 16777216) And &HFF
    stream.Write Chr(b1) & Chr(b2) & Chr(b3) & Chr(b4)
End Sub

' =========================================
' 이벤트 핸들러: OnAudioData
' =========================================
Sub OnVoice_OnAudioData(ByVal audioData)
    On Error Resume Next
    
    Dim size, i
    size = 0
    
    If Not IsEmpty(audioData) And IsArray(audioData) Then
        size = UBound(audioData) - LBound(audioData) + 1
        
        ' 배열 크기 확장 (필요시)
        Dim currentSize
        currentSize = UBound(g_audioData) - LBound(g_audioData) + 1
        If g_audioDataCount >= currentSize Then
            ReDim Preserve g_audioData(LBound(g_audioData) To UBound(g_audioData) + 100)
        End If
        
        ' 데이터 복사
        Dim tempArray
        ReDim tempArray(LBound(audioData) To UBound(audioData))
        For i = LBound(audioData) To UBound(audioData)
            tempArray(i) = audioData(i)
        Next
        
        g_audioData(g_audioDataCount) = tempArray
        g_audioDataCount = g_audioDataCount + 1
    End If
End Sub

