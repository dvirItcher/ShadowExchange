# ShadowExchange
don't readme

"C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" amd64

%windir%\system32\speech_onecore\engines\tts\msttsloc_onecoreenus.dll

cl.exe /O2 /MT /DUNICODE /D_UNICODE main8.cpp /Fe:MSTTSLoc_OneCoreEnUS.dll /link /DLL User32.lib Wtsapi32.lib
