/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2020
*
*  TITLE:       MAIN.CPP
*
*  VERSION:     1.01
*
*  DATE:        18 Feb 2020
*
*  Hamakaze main logic and entrypoint.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/

#include "global.h"

KDU_CONTEXT* g_ProvContext;

#pragma data_seg("iris")
volatile LONG g_lApplicationInstances = 0;
#pragma data_seg()
#pragma comment(linker, "/Section:iris,RWS")

#define T_KDUUNSUP   "[!] Unsupported WinNT version"
#define T_KDURUN     "[!] Another instance running, close it before"

#define CMD_PRV         L"-prv"
#define CMD_MAP         L"-map"
#define CMD_PS          L"-ps"
#define CMD_DSE         L"-dse"
#define CMD_LIST        L"-list"
#define CMD_COMPRESS    L"-compress"
#define CMD_TEST        L"-test"

#define T_KDUUSAGE   "[?] No parameters specified or command not recognized, see Usage for help\r\n"\
                     "[?] Usage: kdu Mode [Provider][Command]\r\n\n"\
                     "Parameters: \r\n"\
                     "kdu -prv id       - optional parameter, provider id, default 0\r\n"\
                     "kdu -ps pid       - disable ProtectedProcess for given pid\r\n"\
                     "kdu -map filename - map driver to the kernel and execute it entry point\r\n"\
                     "kdu -dse value    - write user defined value to the system DSE state flags\r\n"\
                     "kdu -list         - list available providers\r\n"                     

#define T_KDUINTRO   "[+] Kernel Driver Utility v1.0.1 started, (c) 2020 KDU Project\r\n[+] Supported x64 OS: Windows 7 and above"
#define T_PRNTDEFAULT   "%s\r\n"

PWCHAR Ansi2Uni(LPCSTR lpMultiByteStr)
{
    int		nUniLen = 0;
    PWCHAR  pUnicode = NULL;
    nUniLen = MultiByteToWideChar(CP_ACP, 0, lpMultiByteStr, -1, NULL, 0);
    pUnicode = (PWCHAR)malloc((nUniLen + 1) * sizeof(WCHAR));
    memset(pUnicode, 0, (nUniLen + 1) * sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, 0, lpMultiByteStr, -1, (LPWSTR)pUnicode, nUniLen);
    return  pUnicode;
}

/*
* KDUProcessCommandLine
*
* Purpose:
*
* Parse command line and do stuff.
*
*/
INT KDUProcessCommandLine(int argc, char* argv[],
    _In_ ULONG HvciEnabled,
    _In_ ULONG NtBuildNumber
)
{
    INT     retVal = -1;
    ULONG   providerId = KDU_PROVIDER_DEFAULT, dseValue = 0;
    CHAR   szParameter[MAX_PATH + 1];

    HINSTANCE hInstance = GetModuleHandle(NULL);

    printf_s("[>] Entering %s\r\n", __FUNCTION__);

    RtlSecureZeroMemory(szParameter, sizeof(szParameter));

    do {

#ifdef _DEBUG

        //
        // Test switch, never used/present in the release build.
        //
        if (supGetCommandLineOption(CMD_TEST,
            FALSE,
            NULL,
            0))
        {
            KDUTest();
            retVal = 0;
            break;
        }

        //
        // Compress switch, never used/present in the release build.
        //
        if (supGetCommandLineOption(CMD_COMPRESS,
            TRUE,
            szParameter,
            sizeof(szParameter) / sizeof(WCHAR)))
        {
            KDUCompressResource(szParameter);
            retVal = 0;
            break;
        }

#endif

        //
        // List providers.
        //
        if (supGetCommandLineOption(argc, argv,
            "-list",
            FALSE,
            NULL,
            0))
        {
            KDUProvList();
            retVal = 0;
            break;
        }

        //
        // Select CVE provider.
        //
        if (supGetCommandLineOption(argc, argv, 
            "-prv",
            TRUE,
            szParameter,
            sizeof(szParameter)))
        {
            providerId = strtoul_a(szParameter);
            if (providerId >= KDU_PROVIDERS_MAX) {
                printf_s("[!] Invalid provider id specified, default will be used\r\n");
                providerId = KDU_PROVIDER_DEFAULT;
            }
        }

        //
        // Check if -dse specified.
        //
        if (supGetCommandLineOption(argc, argv, 
            "-des",
            TRUE,
            szParameter,
            sizeof(szParameter)))
        {
            g_ProvContext = KDUProviderCreate(providerId,
                HvciEnabled,
                NtBuildNumber,
                hInstance,
                ActionTypeDSECorruption);

            if (g_ProvContext) {
                dseValue = strtoul_a(szParameter);
                KDUControlDSE(g_ProvContext, dseValue);
                KDUProviderRelease(g_ProvContext);
            }
        }
        else

            //
            // Check if -map specified.
            //
            if (supGetCommandLineOption(argc, argv, 
                "-map",
                TRUE,
                szParameter,
                sizeof(szParameter)))
            {
                //map driver
                auto temp = Ansi2Uni(szParameter);
                if (RtlDoesFileExists_U(temp)) {

                    g_ProvContext = KDUProviderCreate(providerId,
                        HvciEnabled,
                        NtBuildNumber,
                        hInstance,
                        ActionTypeMapDriver);

                    if (g_ProvContext) {
                        retVal = KDUMapDriver(g_ProvContext, temp);
                        KDUProviderRelease(g_ProvContext);
                    }
                }
                else {
                    printf_s("[!] Input file not found\r\n");
                }
            }

            else

                //
                // Check if -ps specified.
                //
                if (supGetCommandLineOption(argc, argv, 
                    "-ps",
                    TRUE,
                    szParameter,
                    sizeof(szParameter)))
                {
                    g_ProvContext = KDUProviderCreate(providerId,
                        HvciEnabled,
                        NtBuildNumber,
                        hInstance,
                        ActionTypeDKOM);

                    if (g_ProvContext) {

                        if (KDUControlProcess(g_ProvContext, strtou64_a(szParameter)))
                            retVal = 0;

                        KDUProviderRelease(g_ProvContext);
                    }
                }

                else {
                    //
                    // Nothing set, show help.
                    //
                    printf_s(T_PRNTDEFAULT, T_KDUUSAGE);
                }

    } while (FALSE);

    printf_s("[<] Leaving %s\r\n", __FUNCTION__);
    return retVal;
}

/*
* KDUMain
*
* Purpose:
*
* KDU main.
*
*/

int KDUMain(int argc, char* argv[])
{
    LONG x = 0;
    INT iResult = -1;
    OSVERSIONINFO osv;

    printf_s("[>] Entering %s\r\n", __FUNCTION__);

    do {

        x = InterlockedIncrement((PLONG)&g_lApplicationInstances);
        if (x > 1) {
            printf_s(T_PRNTDEFAULT, T_KDURUN);
            break;
        }

        RtlSecureZeroMemory(&osv, sizeof(osv));
        osv.dwOSVersionInfoSize = sizeof(osv);
        RtlGetVersion((PRTL_OSVERSIONINFOW)&osv);
        if (osv.dwMajorVersion < 6) {
            printf_s(T_PRNTDEFAULT, T_KDUUNSUP);
            break;
        }

        CHAR szVersion[100];

        StringCchPrintfA(szVersion, 100,
            "[*] Windows version: %u.%u build %u",
            osv.dwMajorVersion,
            osv.dwMinorVersion,
            osv.dwBuildNumber);

        printf_s(T_PRNTDEFAULT, szVersion);

        BOOLEAN secureBoot;

        if (supQuerySecureBootState(&secureBoot)) {
            printf_s("[*] SecureBoot is %s on this machine\r\n", secureBoot ? "enabled" : "disabled");
        }

        BOOLEAN hvciEnabled;
        BOOLEAN hvciStrict;
        BOOLEAN hvciIUM;

        //
        // Providers maybe *not* HVCI compatible.
        //
        if (supQueryHVCIState(&hvciEnabled, &hvciStrict, &hvciIUM)) {

            if (hvciEnabled) {
                printf_s("[*] Windows HVCI mode detected\r\n");
            }

        }

        iResult = KDUProcessCommandLine(argc, argv, hvciEnabled, osv.dwBuildNumber);

    } while (FALSE);

    InterlockedDecrement((PLONG)&g_lApplicationInstances);

    printf_s("[<] Leaving %s\r\n", __FUNCTION__);

    return iResult;
}

/*
* main
*
* Purpose:
*
* Program entry point.
*
*/
int main(int argc, char* argv[])
{
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    printf_s(T_PRNTDEFAULT, T_KDUINTRO);

    int retVal = 0;

    __try {
        retVal = KDUMain(argc, argv);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        printf_s("[!] Unhandled exception 0x%lx\r\n", GetExceptionCode());
    }
    return retVal;
}