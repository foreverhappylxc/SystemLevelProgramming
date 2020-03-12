#include <windows.h>

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     MessageBox (NULL, TEXT ("Hello, world!"), TEXT ("HelloMsg"), 0) ;

     return 0 ;
}
