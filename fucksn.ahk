#Requires AutoHotkey v2.0-a

; #DllLoad "bin\fucksn.dll"
; #DllLoad "bin\test.dll"
; DllCall("ole32\CoInitializeEx", "ptr", 0, "uint", 2) ; 2 = COINIT_APARTMENTTHREADED
; DllCall("LoadLibrary", "Str", "bin\fucksn.dll")

; Try DllCall("bin\fucksn\start")
; Catch Error as err
;     MsgBox err.what "`n" err.message

Try DllCall("bin\test\start")
Catch Error as err
    MsgBox err.what "`n" err.message

; F12::DllCall("bin\fod\launch_game")
