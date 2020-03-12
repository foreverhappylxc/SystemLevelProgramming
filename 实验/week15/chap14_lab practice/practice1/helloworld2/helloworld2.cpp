#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     static TCHAR szAppName[] = TEXT ("HelloWin") ;
     HWND         hwnd ;
     MSG          msg ;
     WNDCLASS     wndclass ;

     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
     wndclass.lpszMenuName  = NULL ;
     wndclass.lpszClassName = szAppName ;

     if (!RegisterClass (&wndclass))
     {
          MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }
     hwnd = CreateWindow (szAppName,                  // window class name
                          TEXT ("The Hello Program"), // window caption
                          WS_OVERLAPPEDWINDOW,        // window style
                          CW_USEDEFAULT,              // initial x position
                          CW_USEDEFAULT,              // initial y position
                          CW_USEDEFAULT,              // initial x size
                          CW_USEDEFAULT,              // initial y size
                          NULL,                       // parent window handle
                          NULL,                       // window menu handle
                          hInstance,                  // program instance handle
                          NULL) ;                     // creation parameters
     
     ShowWindow (hwnd, iCmdShow) ;
     UpdateWindow (hwnd) ;
     
     while (GetMessage (&msg, NULL, 0, 0))
     {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
     }
     return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     HDC         hdc ;
     PAINTSTRUCT ps ;
     RECT        rect ;
     //系统时间
	 SYSTEMTIME  time;
      //存储时间信息
	 char buffer[200];
     
     switch (message)
     {
     case WM_CREATE:
          //PlaySound (TEXT ("hellowin.wav"), NULL, SND_FILENAME | SND_ASYNC) ;
          return 0 ;

     case WM_PAINT:
          hdc = BeginPaint (hwnd, &ps) ;
          
          GetClientRect (hwnd, &rect) ;
          
          DrawText (hdc, TEXT ("Hello, world"), -1, &rect,
                    DT_SINGLELINE | DT_CENTER | DT_VCENTER) ;
          EndPaint (hwnd, &ps) ;
          return 0 ;
          
     case WM_DESTROY:
          PostQuitMessage (0) ;
          return 0 ;
          //左键点击显示时间
   	 case WM_LBUTTONDOWN:
		  hdc = GetDC(hwnd);
		  GetLocalTime(&time);
		  sprintf(buffer,"%d/%d/%d    %02d:%02d:%02d",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond);

		  POINT ipoint;
		  if(GetCursorPos(&ipoint)){
			  ScreenToClient(hwnd,&ipoint);
			  TextOut(hdc,ipoint.x,ipoint.y,buffer,lstrlen(buffer));
		  }
		  ReleaseDC(hwnd,hdc);
		  return 0;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}
