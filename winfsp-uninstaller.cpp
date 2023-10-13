#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <Windows.h>
#include <Msi.h>

#undef min

#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment(lib, "msi.lib")

using namespace std::string_literals; // enables s-suffix for std::string literals
void searchAndCopy(std::string flag, std::vector<std::string> args, WCHAR* dest, size_t dest_length);
void log(std::ofstream* file, std::string msg);
void logAndClose(std::ofstream* file, std::string msg);


enum UNINSTALL {
    UNINSTALL_SUCCEEDED = 0,
    UNINSTALL_NOT_FOUND = 1,
    UNINSTALL_FAILED = 2,
    UNINSTALL_CANCELED = 3,
    UNINSTALL_REBOOT_REQUIRED = 4,
    UNINSTALL_SKIPPED = 5 
};

/*
    Executable to uninstall the WinFSP 1.x driver.

    Parameters:
    -q: (optional) Quiet mode, skips confirmation dialog
    -t [my title]: (optional) Title used in the confirmation dialog (truncated to at most 60 WHCHARs)
    -m [my message]: (optional) Message used in the confirmation dialog (truncated to at most 250 WCHARs)
    -s: (optional) Skips installer (necessary during uninstall action)
    -l [path]: (optional) Writes debug output to the file specified by path. If the path already exists and is a file, it is overwritten, otherwise a new file is created. The path is truncated to 256 WCHARs.

    Return values are:
    0: WinFsp 1.x driver uninstalled successfully
    1: WinFsp 1.x driver not found, nothing happend
    2: WinFsp 1.x driver uninstallation failed
    3: User aborted uninstallation
    4: WinFsp 1.x driver uninstallation successful, restart required
    5: Uninstallation was skipped
*/
int main(const int argc, char* argv[])
{
    const std::string skip_flag = "-s"s;

    const std::string quiet_flag = "-q"s;

    const std::string title_flag = "-t"s;
    const size_t title_length = 61;
    WCHAR title[title_length] = L"Unistall WinFsp";
 
    const std::string msg_flag = "-m"s;
    const size_t msg_length = 251;
    WCHAR message[msg_length] = L"WinFSP 1.x driver found. Do you want to uninstall it?";

    const std::string log_flag = "-l"s;
    const size_t log_path_length = 256;
    WCHAR log_path[log_path_length];

    //parse arguments and show dialog
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }
    //check if -s is present
    if (std::find(args.begin(), args.end(), skip_flag) != args.end()) {
        return UNINSTALL_SKIPPED;
    }

    std::ofstream log_file;
    //check if -l is present
    if (std::find(args.begin(), args.end(), log_flag) != args.end()) {
        searchAndCopy(log_flag, args, log_path, log_path_length);
        log_file.open(log_path, std::ios::out | std::ios::trunc);
    }

    //search product code with upgrade code
    log(&log_file, "Searching for winfsp 1.x installation"s);
    WCHAR code[39];
    UINT result = MsiEnumRelatedProductsW(L"{82F812D9-4083-4EF1-8BC8-0F1EDA05B46B}", 0, 0, code);
    if (result == ERROR_NO_MORE_ITEMS) {
        logAndClose(&log_file, "Winfsp installation 1.x NOT found."s);
        return UNINSTALL_NOT_FOUND;
    }
    else if (result != ERROR_SUCCESS) {
        logAndClose(&log_file, "Search failed. Return code of MsiEnumRelatedProductsW is: "s + std::to_string(result));
        return UNINSTALL_FAILED;
    }

    //check if -q is not present
    if (std::find(args.begin(), args.end(), quiet_flag) == args.end()) {
        //create alert dialog with OK and cancel button
        log(&log_file, "Showing uninstall confirmation dialog."s);

        //adjust message and title
        searchAndCopy(title_flag, args, title, title_length);
        searchAndCopy(msg_flag, args, message, msg_length);

        int answer = MessageBox(NULL, message, title, MB_OKCANCEL | MB_ICONQUESTION | MB_TASKMODAL);

        if (answer != IDOK) {
            logAndClose(&log_file, "Uninstall canceld by user."s);
            return UNINSTALL_CANCELED;
        }
    }
    else {
        log(&log_file, "Skipping uninstall confirmation dialog."s);
    }

    log(&log_file, "Uninstalling Winfsp 1.x."s);
    //disable winfsp UI
    MsiSetInternalUI((INSTALLUILEVEL)(INSTALLUILEVEL_NONE | INSTALLUILEVEL_UACONLY), NULL);
    //uninstall product
    result = MsiConfigureProductExW(code, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, L"IGNOREDEPENDENCIES=ALL REBOOT=ReallySuppress");
    if (result == ERROR_SUCCESS) {
        logAndClose(&log_file, "Uninstall succeeded."s);
        return UNINSTALL_SUCCEEDED;
    }
    else if (result == ERROR_SUCCESS_REBOOT_REQUIRED) {
        logAndClose(&log_file, "Uninstall succeeded, reboot required."s);
        return UNINSTALL_REBOOT_REQUIRED; //enforce a restart
    }
    else {
        logAndClose(&log_file, "Uninstall failed. Return code of MsiConfigureProductExW is:"s + std::to_string(result));
        return UNINSTALL_FAILED;
    }
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

void log(std::ofstream* file, std::string msg) {
    if ((*file).is_open()) {
        (*file) << msg << "\n";
    }
}

void logAndClose(std::ofstream* file, std::string msg) {
    if ((*file).is_open()) {
        (*file) << msg << "\n";
        (*file).close();
    }
}