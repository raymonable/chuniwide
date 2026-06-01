#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#include <TlHelp32.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <emmintrin.h>

#include <Zydis/Zydis.h>

const uint32_t functionSignatureLength = 25;
const uint8_t functionSignature[] = {0x8D, 0x8F, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x6A, 0x00, 0xC7, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8D, 0x8F};
const uint64_t functionSignatureBitMask = 0b1100000000110100001000001;

uintptr_t ScanForAbsoluteMov(const uint8_t* buffer) {
    uint32_t offset = 0;
    ZydisDisassembledInstruction instruction;

    while (ZYAN_SUCCESS(ZydisDisassembleIntel(
        ZYDIS_MACHINE_MODE_LONG_COMPAT_32,
        0, buffer + offset, 1e5, &instruction
    ))) {
        if (instruction.info.mnemonic == ZYDIS_MNEMONIC_MOV && instruction.info.operand_count == 2) {
            if (instruction.operands[0].mem.base == ZYDIS_REGISTER_NONE)
                return instruction.operands[0].mem.disp.value;
        }
        offset += instruction.info.length;
    }

    return 0;
}

uintptr_t ScanForSignature(uint8_t* buffer, size_t size, const uint8_t* signature, const uint64_t signatureBitMask, const uint32_t signatureLength) {
    __m128i initial = _mm_set1_epi8((char)signature[0]);

    for (size_t index = 0; index < (size - signatureLength); index += 16) {
        __m128i chunk = _mm_loadu_si128((__m128i*)(buffer + index));
        __m128i result = _mm_cmpeq_epi8(chunk, initial);
        int bitMask = _mm_movemask_epi8(result);
        while (bitMask != 0) {
            // access offset of lowest set bit
            unsigned long offset;
            BitScanForward(&offset, bitMask);

            // scan rest of buffer (index = 1 since we don't need to scan initial byte)
            size_t signatureIndex = 1;
            while (signatureLength > signatureIndex) {
                BOOL isWildcard = ((signatureBitMask >> signatureIndex) & 1) == 0;
                if (!isWildcard && buffer[index + offset + signatureIndex] != signature[signatureIndex])
                    break;

                signatureIndex++;

                if (signatureIndex + 1 == signatureLength)
                    return index + offset;
            }

            // clear bit so the loop doesn't get stuck & next bit can be read
            bitMask &= (bitMask - 1);
        }
    }

    return 0;
}

uintptr_t GetModuleBaseAddress(DWORD64 procId, const char* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, (DWORD)procId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!strcmp(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
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
        Sleep(300);
        hwnd = GetForegroundWindow();
        GetWindowTextW(hwnd, title, 500);
    } while (wcscmp(title, L"teaGfx DirectX Release") != 0);

    MODULEINFO moduleInfo;
    GetModuleInformation(
        GetCurrentProcess(),
        GetModuleHandleA(NULL),
        &moduleInfo,
        sizeof(MODULEINFO)
    );

    uintptr_t functionAddr = ScanForSignature(
        moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage,
        functionSignature, functionSignatureBitMask, functionSignatureLength
    );
    DWORD processId = GetCurrentProcessId();
    // we have to subtract image base (0x400000) since it's included in the mov
    uintptr_t addr = GetModuleBaseAddress(processId, "chusanApp.exe") + ScanForAbsoluteMov((uint8_t*)moduleInfo.lpBaseOfDll + functionAddr) - 0x400000;

    BOOL gamePlayFlag = FALSE;

    RECT rect;
    rect.left = 0;
    rect.top = 0;

#ifdef RESIZE_25_INCH
    OutputDebugStringA("Chuniwide: Using 25 inch monitor size\n");
#elif RESIZE_27_INCH
    OutputDebugStringA("Chuniwide: Using 27 inch monitor size\n");
#endif

    while (TRUE) {
        uint32_t gamePlayRead = *(uint32_t*)(addr);

        if (gamePlayRead > 0 && gamePlayFlag == FALSE)
        {
#ifdef RESIZE_25_INCH
            SetWindowPos(hwnd, 0, rect.left - 288, rect.top - 324, 2496, 1404, SWP_SHOWWINDOW);
#elif RESIZE_27_INCH
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