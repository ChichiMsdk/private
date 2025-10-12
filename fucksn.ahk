#Requires AutoHotkey v2.0-a

; #DllLoad "bin\fucksn.dll"
; #DllLoad "bin\test.dll"
; DllCall("ole32\CoInitializeEx", "ptr", 0, "uint", 2) ; 2 = COINIT_APARTMENTTHREADED
; DllCall("LoadLibrary", "Str", "bin\fucksn.dll")

DllCall("bin\fucksn\start")
Exit

; F12::DllCall("bin\fod\launch_game")
