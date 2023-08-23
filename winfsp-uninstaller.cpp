#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <Windows.h>
#include <Msi.h>

#undef min

#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment(lib, "msi.lib")

using namespace std::string_literals; // enables s-suffix for std::string literals
void searchAndCopy(std::string flag, std::vector<std::string> args, WCHAR* dest, size_t dest_length);

/*
    Executable to uninstall the WinFSP 1.x driver.

    Parameters:
    -q: Quiet mode, skips confirmation dialog
    -t: (optional) Title used in the confirmation dialog (truncated to at most 60 WHCHARS)
    -m: (optional) Message used in the confirmation dialog (truncated to at most 250 WCHARS)
    
    Return values are:
    0: WinFsp 1.x driver uninstalled successfully
    1: WinFsp 1.x driver not found, nothing happend
    2: WinFsp 1.x driver uninstallation failed
    3: User aborted uninstallation
    4: WinFsp 1.x driver uninstallation successful, restart required
*/
int main(const int argc, char* argv[])
{
    const std::string quiet_flag = "-q"s;

    const std::string title_flag = "-t"s;
    const size_t title_length = 61;
    WCHAR title[title_length] = L"Unistall WinFsp";
 
    const std::string msg_flag = "-m"s;
    const size_t msg_length = 251;
    WCHAR message[msg_length] = L"WinFSP 1.x driver found. Do you want to uninstall it?";


    //search product code with upgrade code
    WCHAR code[39];
    UINT result = MsiEnumRelatedProductsW(L"{82F812D9-4083-4EF1-8BC8-0F1EDA05B46B}", 0, 0, code);
    if (result == ERROR_NO_MORE_ITEMS) {
        //product not found
        return 1;
    }
    else if (result != ERROR_SUCCESS) {
        //error
        return 2;
    }

    //parse arguments and show dialog
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
		args.push_back(argv[i]);
	}
    //check if -q is not present
    if ((std::find(args.begin(), args.end(), quiet_flag) == args.end())) {
        //create alert dialog with OK and cancel button

        //adjust message and title
        searchAndCopy(title_flag, args, title, title_length);
        searchAndCopy(msg_flag, args, message, msg_length);

        int answer = MessageBox(NULL, message, title, MB_OKCANCEL | MB_ICONQUESTION | MB_TASKMODAL);

        if (answer != IDOK) {
            return 3;
        }
    }

    //disable winfsp UI
    MsiSetInternalUI((INSTALLUILEVEL)(INSTALLUILEVEL_NONE | INSTALLUILEVEL_UACONLY), NULL);
    //uninstall product
    result = MsiConfigureProductExW(code, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, L"IGNOREDEPENDENCIES=ALL");
    if (result != ERROR_SUCCESS) {
        return 2;
    }

    //query registry to check if a restart is required
    HKEY key;
    result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager", 0, KEY_READ, &key);
    if (result != ERROR_SUCCESS) {
		return 4; //enforce a restart
	}
    result = RegQueryValueExW(key, L"PendingFileRenameOperations", NULL, NULL, NULL, NULL);
	RegCloseKey(key);
    if ( result == ERROR_FILE_NOT_FOUND ) {
        return 0;
    }
    return 4; //enforce a restart
}

void searchAndCopy(std::string flag, std::vector<std::string> args, WCHAR* dest, size_t dest_length) {
    auto it = std::find(args.begin(), args.end(), flag);
    if (it != args.end()) {
        //get index of flag
        auto index = std::distance(args.begin(), it);
        //check if there is a string after the flag
        if ( index >= 0 && ((size_t) index) + 1 < args.size()) {
            //copy string to destination
            std::string args_content = args[index + 1];
            size_t count = std::min((size_t) std::distance(args_content.begin(), args_content.end()), dest_length-1);
            std::copy_n(args_content.begin(), count, dest);
            dest[count] = '\0';
        }
    }
}