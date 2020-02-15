#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>

#define WM_SHELL_NOTIFY (WM_USER + 100)

static WCHAR s_szDir1[MAX_PATH];    // "%TEMP%\\WatchDir1"
static WCHAR s_szDir2[MAX_PATH];    // "%TEMP%\\WatchDir1\\Dir2"
static WCHAR s_szFile1[MAX_PATH];   // "%TEMP%\\WatchDir1\\File1.txt"
static WCHAR s_szFile2[MAX_PATH];   // "%TEMP%\\WatchDir1\\Dir2\\File2.txt"

static HWND s_hwnd = NULL;
static INT s_argc = 0;
static LPWSTR *s_wargv = NULL;
static LPITEMIDLIST s_pidl = NULL;
static UINT s_uRegID = 0;
static SHChangeNotifyEntry s_entry;

static BOOL
DoInit(HWND hwnd, INT argc, LPWSTR *wargv)
{
    WCHAR szTemp[MAX_PATH], szPath[MAX_PATH];

    if (argc >= 2)
    {
        GetLongPathNameW(wargv[1], szTemp, ARRAYSIZE(szTemp));
        GetFullPathNameW(szTemp, ARRAYSIZE(szPath), szPath, NULL);
    }
    else
    {
        GetTempPathW(ARRAYSIZE(szTemp), szTemp);
        GetLongPathNameW(szTemp, szPath, ARRAYSIZE(szPath));
    }

    lstrcpyW(s_szDir1, szPath);
    PathAddBackslashW(s_szDir1);
    lstrcatW(s_szDir1, L"WatchDir1");
    CreateDirectoryW(s_szDir1, NULL);
    printf("s_szDir1: %S\n", s_szDir1);

    lstrcpyW(s_szDir2, s_szDir1);
    PathAddBackslashW(s_szDir2);
    lstrcatW(s_szDir2, L"Dir2");
    printf("s_szDir2: %S\n", s_szDir2);

    lstrcpyW(s_szFile1, s_szDir1);
    PathAddBackslashW(s_szFile1);
    lstrcatW(s_szFile1, L"File1.txt");
    printf("s_szFile1: %S\n", s_szFile1);

    lstrcpyW(s_szFile2, s_szDir2);
    PathAddBackslashW(s_szFile2);
    lstrcatW(s_szFile2, L"File2.txt");
    printf("s_szFile2: %S\n", s_szFile2);

    s_pidl = ILCreateFromPathW(s_szDir1);

    s_entry.pidl = s_pidl;
    s_entry.fRecursive = TRUE;
    LONG fEvents = SHCNE_ALLEVENTS;
    s_uRegID = SHChangeNotifyRegister(hwnd, SHCNRF_ShellLevel/* | SHCNRF_NewDelivery*/,
                                      fEvents, WM_SHELL_NOTIFY, 1, &s_entry);
    return s_uRegID != 0;
}

static BOOL
OnInitDialog(HWND hwnd)
{
    s_hwnd = hwnd;

    BOOL bOK = DoInit(hwnd, s_argc, s_wargv);
    if (bOK)
    {
        SetDlgItemTextW(hwnd, edt1, s_szDir1);
        ShellExecuteW(hwnd, NULL, s_szDir1, NULL, NULL, SW_SHOWNORMAL);
    }
    else
    {
        MessageBoxW(NULL, L"SHChangeNotifyRegister failed", NULL, MB_ICONERROR);
        DestroyWindow(hwnd);
    }

    return TRUE;
}

static void
OnCommand(HWND hwnd, INT id)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        DestroyWindow(hwnd);
        break;
    }
}

static void
OnDestroy(HWND hwnd)
{
    SHChangeNotifyDeregister(s_uRegID);
    CoTaskMemFree(s_pidl);
    s_hwnd = NULL;
}

static void
DoShellNotify(HWND hwnd, PIDLIST_ABSOLUTE pidl1, PIDLIST_ABSOLUTE pidl2, LONG lEvent)
{
    CHAR szPath1[MAX_PATH], szPath2[MAX_PATH];

    if (pidl1)
        SHGetPathFromIDListA(pidl1, szPath1);
    else
        szPath1[0] = 0;

    if (pidl2)
        SHGetPathFromIDListA(pidl2, szPath2);
    else
        szPath2[0] = 0;

    switch (lEvent)
    {
    case SHCNE_RENAMEITEM:
        printf("SHCNE_RENAMEITEM('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_CREATE:
        printf("SHCNE_CREATE('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_DELETE:
        printf("SHCNE_DELETE('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_MKDIR:
        printf("SHCNE_MKDIR('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_RMDIR:
        printf("SHCNE_RMDIR('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_MEDIAINSERTED:
        printf("SHCNE_MEDIAINSERTED('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_MEDIAREMOVED:
        printf("SHCNE_MEDIAREMOVED('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_DRIVEREMOVED:
        printf("SHCNE_DRIVEREMOVED('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_DRIVEADD:
        printf("SHCNE_DRIVEADD('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_NETSHARE:
        printf("SHCNE_NETSHARE('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_NETUNSHARE:
        printf("SHCNE_NETUNSHARE('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_ATTRIBUTES:
        printf("SHCNE_ATTRIBUTES('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_UPDATEDIR:
        printf("SHCNE_UPDATEDIR('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_UPDATEITEM:
        printf("SHCNE_UPDATEITEM('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_SERVERDISCONNECT:
        printf("SHCNE_SERVERDISCONNECT('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_UPDATEIMAGE:
        printf("SHCNE_UPDATEIMAGE('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_DRIVEADDGUI:
        printf("SHCNE_DRIVEADDGUI('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_RENAMEFOLDER:
        printf("SHCNE_RENAMEFOLDER('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_FREESPACE:
        printf("SHCNE_FREESPACE('%s', '%s')\n", szPath1, szPath2);
        break;
    case SHCNE_EXTENDED_EVENT:
        printf("SHCNE_EXTENDED_EVENT('%p', '%p')\n", pidl1, pidl2);
        break;
    case SHCNE_ASSOCCHANGED:
        printf("SHCNE_ASSOCCHANGED('%s', '%s')\n", szPath1, szPath2);
        break;
    default:
        printf("(lEvent:%08lX)('%s', '%s')\n", lEvent, szPath1, szPath2);
        break;
    }
    fflush(stdout);
}

static INT_PTR
OnShellNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    LONG lEvent;
    PIDLIST_ABSOLUTE *pidlAbsolute;
    HANDLE hLock = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &pidlAbsolute, &lEvent);
    if (hLock)
    {
        DoShellNotify(hwnd, pidlAbsolute[0], pidlAbsolute[1], lEvent);
        SHChangeNotification_Unlock(hLock);
    }
    else
    {
        pidlAbsolute = (PIDLIST_ABSOLUTE *)wParam;
        DoShellNotify(hwnd, pidlAbsolute[0], pidlAbsolute[1], lParam);
    }
    return TRUE;
}

static INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        return OnInitDialog(hwnd);
    case WM_COMMAND:
        OnCommand(hwnd, LOWORD(wParam));
        break;
    case WM_SHELL_NOTIFY:
        return OnShellNotify(hwnd, wParam, lParam);
    case WM_DESTROY:
        OnDestroy(hwnd);
        break;
    }
    return 0;
}

static BOOL WINAPI
HandlerRoutine(DWORD CtrlType)
{
    DestroyWindow(s_hwnd);
    return FALSE;
}

int main(int argc, char **argv)
{
    printf("Press Ctrl+C to quit\n");
    s_wargv = CommandLineToArgvW(GetCommandLineW(), &s_argc);
    SetConsoleCtrlHandler(HandlerRoutine, TRUE);
    HWND hwnd = CreateDialogW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1), NULL, DialogProc);

    ShowWindow(hwnd, SW_HIDE);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    LocalFree(s_wargv);
    return 0;
}
