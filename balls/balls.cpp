#include <windows.h>
#include <tlhelp32.h>
#include <thread>
#include <string>
#include <iostream>

DWORD GetProcessIdByName(const wchar_t* processName) {
    DWORD processID = 0;
    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    if (Process32FirstW(hSnapshot, &processEntry)) {
        do {
            if (wcscmp(processEntry.szExeFile, processName) == 0) {
                processID = processEntry.th32ProcessID;
                break;
            }
        } while (Process32NextW(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
    return processID;
}

bool IsWindowVisibleByName(const char* windowName) {
    HWND hwnd = FindWindowA(NULL, windowName);
    return hwnd && IsWindowVisible(hwnd);
}

int main() {
    std::string dllFilePath;
    std::cout << "welcome to skibiditoilet injector | github.com/v-xa/skibiditoilet.injector" << std::endl;
    std::cout << "skibiditoilet injector | path to dll (THE NAME NEEDS TO BE amdxx64.dll): ";
    std::getline(std::cin, dllFilePath);
    
    const wchar_t* targetProcessName = L"RobloxPlayerBeta.exe";
    const char* callBackName = "InitHook";
    int pid = GetProcessIdByName(targetProcessName);

    while (pid == 0 || !IsWindowVisibleByName("Roblox")) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pid = GetProcessIdByName(targetProcessName);
    }

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!processHandle) {
        std::cout << "Failed to open handle" << std::endl;
        return -1;
    }

    HWND hwnd = FindWindowA(NULL, "Roblox");
    DWORD threadID = GetWindowThreadProcessId(hwnd, NULL);

    HMODULE dll = LoadLibraryExA(dllFilePath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (!dll) {
        std::cout << "DLL not found" << std::endl;
        CloseHandle(processHandle);
        return -1;
    }

    FARPROC callbackAddr = GetProcAddress(dll, callBackName);
    if (!callbackAddr) {
        std::cout << "Callback not found" << std::endl;
        CloseHandle(processHandle);
        return -1;
    }

    HHOOK hook = SetWindowsHookExA(WH_GETMESSAGE, (HOOKPROC)callbackAddr, dll, threadID);
    if (!hook) {
        std::cout << "Failed to set hook" << std::endl;
        CloseHandle(processHandle);
        return -1;
    }

    PostThreadMessageA(threadID, WM_NULL, 0, 0);

    while (GetProcessIdByName(targetProcessName) != 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    FreeConsole();
    ExitProcess(0);

    return 0;
}
