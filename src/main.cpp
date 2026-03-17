#include <QApplication>
#include <windows.h>
#include <iostream>
#include <string>
#include "Config.h"
#include "MainWindow.h"
#include "InterceptionWrapper.h"

bool IsRunningAsAdmin() {
    BOOL fIsRunAsAdmin = FALSE;
    PSID pAdministratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
        CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin);
        FreeSid(pAdministratorsGroup);
    }
    return fIsRunAsAdmin;
}

void RelaunchAsAdmin(int argc, char* argv[]) {
    wchar_t szPath[MAX_PATH];
    if (GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath))) {
        std::wstring args = L"";
        for (int i = 1; i < argc; ++i) {
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, argv[i], -1, NULL, 0);
            std::wstring wstrTo(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, argv[i], -1, &wstrTo[0], size_needed);
            args += wstrTo + L" ";
        }

        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.lpVerb = L"runas";
        sei.lpFile = szPath;
        sei.lpParameters = args.c_str();
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;

        ShellExecuteExW(&sei);
    }
}

int main(int argc, char *argv[]) {
    if (!IsRunningAsAdmin()) {
        RelaunchAsAdmin(argc, argv);
        return 0;
    }

    QApplication app(argc, argv);

    // Load configuration
    Config::load();
    
    // Initialize Interception
    if (!intercept::init()) {
        std::cerr << "[ERRO] Falha ao inicializar Interception context!" << std::endl;
    }

    std::cout << "[LOG] Emulador iniciado via C++/Qt." << std::endl;

    MainWindow w;
    w.show();

    int result = app.exec();
    
    intercept::cleanup();
    return result;
}
