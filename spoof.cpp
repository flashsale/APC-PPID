#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <csignal>



DWORD getParentProcessID() {

    // Take a snapshot of currently running processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32 process = { 0 };
    process.dwSize = sizeof(process);

    if (Process32First(snapshot, &process)) {
        do {
            if (!wcscmp(process.szExeFile, L"OneDrive.exe"))
                break;
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);
    return process.th32ProcessID;
}


int main() {

    // Variable decleration
    STARTUPINFOEXA si;
    PROCESS_INFORMATION pi;
    SIZE_T st;
    HANDLE ph;
 
    FreeConsole();

    // Get handle to specific parent process
    ph = OpenProcess(PROCESS_ALL_ACCESS, false, getParentProcessID());

    // Zero out the memory locations of the variables
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    // Find the size needed for later call to initialize our Attribute List
    InitializeProcThreadAttributeList(
        NULL,   // Passing NULL lets us determine the buffer size required to support our number of attributes
        1,      // Attribute Count
        0,      // Flags, 0 req
        &st     // Deref to store returned size
    );


    // Allocate memory in the heap for our attribute list, returns a ptr to the allocated block
    si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, st);

    // Run initialize again, this time with the actual memory block prepared
    InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &st);

    // Update our attribute list to use our specified PPID
    UpdateProcThreadAttribute(
        si.lpAttributeList,                         // ptr to attribute list
        0,                                          // No flags
        PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,       // Following parameter will be a handle to a process to use rather than calling process for proc creation
        &ph,                                        // Handle to process we would like to set as the "parent process"
        sizeof(ph),                                 // Size of prior attribute
        NULL, NULL                                  // Reserved, NULL req
    );

    // Set size of STARTUPINFOEXA into the variable within, not 100% whats up with this tbh
    si.StartupInfo.cb = sizeof(STARTUPINFOEXA);

    // Start the child process 
    CreateProcessA(
        "c:\\Windows\\System32\\notepad.exe",           // Module to be called (Path)
        NULL,                                           // Command line Arguments(?)
        NULL,                                           // Process handle not inheritable
        NULL,                                           // Thread handle not inheritable
        TRUE,                                           // Set handle inheritance to FALSE
        EXTENDED_STARTUPINFO_PRESENT,                   // Creation flag: Tells it the process is created with the extended startup info
        NULL,                                           // Use parent's environment block
        NULL,                                           // Use parent's starting directory 
        reinterpret_cast<LPSTARTUPINFOA>(&si),          // Pointer to STARTUPINFOEXA structure
        &pi
    );

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
