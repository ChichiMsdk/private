#Requires AutoHotkey v2.0-a

; #DllLoad "bin\fucksn.dll"
; #DllLoad "bin\test.dll"
; DllCall("ole32\CoInitializeEx", "ptr", 0, "uint", 2) ; 2 = COINIT_APARTMENTTHREADED
; DllCall("LoadLibrary", "Str", "bin\fucksn.dll")

; h := DllCall("LoadLibrary", "Str", "bin\fucksn.dll", "Ptr")

; DllCall("fucksn.dll\bridge_start")
; DllCall("fucksn.dll\bridge_write", "AStr", "echo Hello from AHK!")
; VarSetStrCapacity(buf, 1024)
; len := DllCall("fucksn.dll\bridge_read", "Ptr", &buf, "UInt", 1024, "Int")
; if (len > 0)
;     MsgBox(StrGet(&buf, len, "CP0"))
; DllCall("fucksn.dll\bridge_stop")
; DllCall("FreeLibrary", "Ptr", h, "Int")

DllCall("bin\fucksn\start")
; Exit

; F12::DllCall("bin\fod\launch_game")
