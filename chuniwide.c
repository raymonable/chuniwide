#include <windows.h>
#include <stdint.h>
#include <string.h>
#include <TlHelp32.h>

uintptr_t GetModuleBaseAddress(DWORD64 processId, const char* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                if (!strcmp(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)(modEntry.modBaseAddr);
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

DWORD WINAPI Search(void* ptr) {
    HWND hwnd;
    wchar_t title[500];

    do {
        Sleep(3000);
        hwnd = GetForegroundWindow();
        GetWindowTextW(hwnd, title, 500);
    } while (wcscmp(title, L"teaGfx DirectX Release") != 0);

    BOOL gamePlayFlag = FALSE;
    DWORD processId = GetCurrentProcessId();
    uintptr_t addr = GetModuleBaseAddress(processId, "chusanApp.exe") + 0x185BFA0;

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    while (TRUE) {
        uint32_t gamePlayRead = *(uint32_t*)(addr);

        if (gamePlayRead > 0 && gamePlayFlag == FALSE)
        {
#ifdef RESIZE_25_INCH
            SetWindowPos(hwnd, 0, rect.left - 288, rect.top - 324, 2496, 1404, SWP_SHOWWINDOW);
#else
            SetWindowPos(hwnd, NULL, rect.left - 144, rect.top - 162, 2208, 1242, SWP_SHOWWINDOW);
#endif
            gamePlayFlag = TRUE;
        }
        if (gamePlayRead == 0 && gamePlayFlag == TRUE)
        {
            SetWindowPos(hwnd, NULL, rect.left, rect.top, 1920, 1080, SWP_SHOWWINDOW);
            gamePlayFlag = FALSE;
        }

        Sleep(500);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DWORD tid;
        CreateThread(
            NULL,
            0,
            Search,
            NULL,
            0,
            &tid
        );
    }
    return TRUE;
}