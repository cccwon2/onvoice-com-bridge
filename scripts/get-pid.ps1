# Discord PID 찾기
Get-Process -Name "Discord" | Select-Object Id, ProcessName

# Chrome PID 찾기
Get-Process -Name "chrome" | Select-Object Id, ProcessName