#include <windows.h>
#include <process.h>

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;
void pthread(void* pidthread);

HWND hwnd ;
int  cxClient, cyClient ;
//s判断是左右键点击，右键点击为0，左键点击为1
int s;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     static TCHAR szAppName[] = TEXT ("RndRctMT") ;
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
     
     hwnd = CreateWindow (szAppName, TEXT ("Random Rectangles"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, NULL, hInstance, NULL) ;
     
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
     switch (message)
     {
     case WM_CREATE:
          return 0;
     case WM_RBUTTONDOWN:
		 s = 0;
		 //开始线程
		 _beginthread(pthread, 0, NULL);
		 /*HBRUSH hBrush ;
		 HDC    hdc ;
		 int    xLeft, xRight, yTop, yBottom, iRed, iGreen, iBlue ;
		 
		 while (1)
		 {
			 if (cxClient != 0 || cyClient != 0)
			 {
				 xLeft   = rand () % cxClient ;
				 xRight  = rand () % cxClient ;
				 yTop    = rand () % cyClient ;
				 yBottom = rand () % cyClient ;
				 iRed    = rand () & 255 ;
				 iGreen  = rand () & 255 ;
				 iBlue   = rand () & 255 ;
				 
				 hdc = GetDC (hwnd) ;
				 hBrush = CreateSolidBrush (RGB (iRed, iGreen, iBlue)) ;
				 SelectObject (hdc, hBrush) ;
				 
				 Rectangle (hdc, min (xLeft, xRight), min (yTop, yBottom),
					 max (xLeft, xRight), max (yTop, yBottom)) ;
				 
				 ReleaseDC (hwnd, hdc) ;
				 DeleteObject (hBrush) ;
			 }
		 }*/
		 return 0 ;
	 case WM_LBUTTONDOWN:
		 s = 1;
		 MessageBox(hwnd,"Left button down!","点击左键",MB_OK);
		 return 0;
     case WM_SIZE:
          cxClient = LOWORD (lParam) ;
          cyClient = HIWORD (lParam) ;
          return 0 ;
          
     case WM_DESTROY:
          PostQuitMessage (0) ;
          return 0 ;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}

void pthread(void* pidthread)
{
	HBRUSH hBrush ;
	HDC    hdc ;
	int    xLeft, xRight, yTop, yBottom, iRed, iGreen, iBlue ;
	
	//将true改成了！s左键点击停下，右键点击开始
	while (!s)
	{
		if (cxClient != 0 || cyClient != 0)
		{
			xLeft   = rand () % cxClient ;
			xRight  = rand () % cxClient ;
			yTop    = rand () % cyClient ;
			yBottom = rand () % cyClient ;
			iRed    = rand () & 255 ;
			iGreen  = rand () & 255 ;
			iBlue   = rand () & 255 ;
			
			hdc = GetDC (hwnd) ;
			hBrush = CreateSolidBrush (RGB (iRed, iGreen, iBlue)) ;
			SelectObject (hdc, hBrush) ;
			
			Rectangle (hdc, min (xLeft, xRight), min (yTop, yBottom),
				max (xLeft, xRight), max (yTop, yBottom)) ;
			
			ReleaseDC (hwnd, hdc) ;
			DeleteObject (hBrush) ;
		}
	}
	//结束线程
	_endthread();
	
}