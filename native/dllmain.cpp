#include <iostream>

#ifdef _WIN32

#include <Windows.h>

#endif

void InitDLL()
{
    // Give time to attach debugger if we need to
#ifdef SLEEP
    std::cout << "[InitDLL] Sleeping for 20 seconds to give you time to attach debugger..." << std::endl;

#ifdef _WIN32
    Sleep(20000);
#else
    std::this_thread::sleep_for(std::chrono::seconds(20));
#endif
#endif
}

#ifdef _WIN32

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
    switch (dwReasonForCall)
    {
        case DLL_PROCESS_ATTACH:
        {
            InitDLL();
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            // 2018-04-25 ExitProcess can result in deadlocks for DLL's according to https://www.experts-exchange.com/questions/22012335/ExitProcess-vs-TerminateProcess.html
            unsigned int exitCode = 0x1337;

            if (!TerminateProcess(GetCurrentProcess(), exitCode))
                ExitProcess(exitCode);
            break;
        }

        default:
            break;
    }

    return TRUE;
}

#endif