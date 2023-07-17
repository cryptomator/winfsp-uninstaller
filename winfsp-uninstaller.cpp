#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <Windows.h>
#include <Msi.h>

#undef min

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment(lib, "msi.lib")

using namespace std::string_literals; // enables s-suffix for std::string literals
void searchAndCopy(std::string flag, std::vector<std::string> args, WCHAR* dest, UINT dest_length);

/*
    Executable to uninstall the WinFSP 1.x driver.

    Parameters:
    -q: Quiet mode, skips confirmation dialog
    -t: (optional) Title used in the confirmation dialog (truncated to at most 60 WHCHARS)
    -m: (optional) Message used in the confirmation dialog (truncated to at most 250 WCHARS)
    
    Return values are:
    0: WinFSP 1.x driver uninstalled successfully
    1: WinFSP 1.x driver not found, nothing happend
    2: WinFSP 1.x driver uninstallation failed
    3: User aborted uninstallation
*/
int main(const int argc, char* argv[])
{
    bool quiet = false;
    WCHAR title[61] = L"Unistall WinFsp";
    WCHAR message[251] = L"WinFSP 1.x driver found. Do you want to uninstall it?";

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

    //check if arguments contain -q flag
    if(std::find(args.begin(), args.end(), "-q"s) != args.end()) {
        quiet = true;
	}

    if (!quiet) {
        //create alert dialog with OK and cancel button

        //adjust message and title
        searchAndCopy("-t"s, args, title, 60);
        searchAndCopy("-m"s, args, message, 250);

        int answer = MessageBox(NULL, message, title, MB_OKCANCEL | MB_ICONQUESTION | MB_TASKMODAL);

        if (answer != IDOK) {
            return 3;
        }
    }


    //disable UI
    MsiSetInternalUI((INSTALLUILEVEL)(INSTALLUILEVEL_NONE | INSTALLUILEVEL_UACONLY), NULL);
    //uninstall product
    result = MsiConfigureProductExW(code, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, NULL);
    if (result == ERROR_SUCCESS) {
        return 0;
    }
    else {
        return 2;
    }
}

void searchAndCopy(std::string flag, std::vector<std::string> args, WCHAR* dest, UINT dest_length) {
    auto it = std::find(args.begin(), args.end(), flag);
    if (it != args.end()) {
        //get index of flag
        int index = std::distance(args.begin(), it);
        //check if there is a string after the flag
        if (index + 1 < args.size()) {
            //copy string to destination
            std::string args_content = args[index + 1];
            int count = std::min((UINT)std::distance(args_content.begin(), args_content.end()), dest_length);
            std::copy_n(args_content.begin(), count, dest);
            dest[count] = '\0';
        }
    }
}