// FRT.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "FRT.h"
#include <vector>
#include "GetFotosFromCams.h"
#pragma comment(lib,"GetFotosFromCams.lib")
#include "opencv\cv.h"
#include "opencv\highgui.h"

#include "iostream"
#include "fstream"
using namespace std;

#define MAX_LOADSTRING 100
#pragma comment(lib,"opencv_core241d.lib")
#pragma comment(lib,"opencv_highgui241d.lib")
#pragma comment(lib,"opencv_imgproc241d.lib")
#pragma comment(lib,"opencv_imgproc241d.lib")
typedef std::basic_string<TCHAR> tstring;

//OpenGL
#include "GL.h"
#pragma comment(lib,"OPENGL32.lib")
#include "process.h"

#define IDC_MAIN_BUTTON 101
// Global Variables:
HINSTANCE hInst;												// current instance
TCHAR szTitle[MAX_LOADSTRING];									// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING] = L"szWindowClass";			// the main window class name
TCHAR szVideoWindowClass[MAX_LOADSTRING] = L"szVideoWindowClass";				
//vector <HWND> hVWnds;
HWND hVWnds[16];
HDC winDC[16];
vector <HWND> hWndCBs;
/*------- переменные для обеспечения гибкости расчёта расположения окон для видео -------*/
const int videoWidth = 160;
const int videoHeight = 120;
const int hGap = 20;
const int vGap = 40;
const int cameraNumber = 16;
char **picturesFromCams = new char*[cameraNumber];
HWND hWnd;
bool cameraFlagsInit[cameraNumber] = {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true};
bool cameraFlagsCaptureEnabled[cameraNumber] = {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true};
struct ThreadParams
{
	int camID; 
	HWND *hWnd; 
	int width; 
	int height;
};
struct ThreadParams2
{
	int camID;
	char *camImage;
	int width; 
	int height;
	bool *flagInit;
	bool *flagCaptureEnabled;
};

// Forward declarations of functions included in this code module:
ATOM				RegisterMainWindowClass(HINSTANCE hInstance);
ATOM				RegisterVideoWindowClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void showCamerasSerial(void *tp);
void readCameraConfig(HWND hwnd, int * ids);
//Инициализация OpenGL
int init_OpenGL(HWND hwnd);
//Дополнительные настройки OpenGL
void Init();
//Изменение размеров окна
void Resize(int width, int height);
//отрисовка картинки с камеры на окне
void drawPictureFromCam(void *tp);
//void readPicturesFromCams(void *tp);
void PutIDsIntoFile(HWND hwnd);
void customFlip(char* oldImg, int width, int height, char* newImg);
wchar_t *convertCharArrayToLPCWSTR(const char* charArray);
void initCams(void *tp);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_FRT, szWindowClass, MAX_LOADSTRING);
	RegisterMainWindowClass(hInstance);
	RegisterVideoWindowClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	/***********************************************/

	// параллельный подход
	//for(int i = 0; i < cameraNumber; i++)
	//{
	//	picturesFromCams[i] = new char[videoWidth*videoHeight*3];
	//	memset(picturesFromCams[i],255,videoWidth*videoHeight*3);
	//	
	//	cameraFlagsInit[i] = false;

	//	ThreadParams2 tp = {i, picturesFromCams[i], videoWidth, videoHeight, &cameraFlagsInit[i], &cameraFlagsCaptureEnabled[i]};
	//	DWORD dwThreadId;
	//	HANDLE hCurThread = CreateThread(
	//		NULL, //Choose default security
	//		0, //Default stack size
	//		(LPTHREAD_START_ROUTINE)&initCams, //Routine to execute
	//		(LPVOID) &tp, //Thread parameter
	//		0, //Immediately run the thread
	//		&dwThreadId //Thread Id
	//	);

	//	while(tp.height != 0) 
	//	{
	//		Sleep(10);
	//	}

	//	Sleep(3000);
	//}



	// this is codeEEEEEEEEEEEEEE!
	for(int i = 0; i < cameraNumber; i++)
	{
		picturesFromCams[i] = new char[videoWidth*videoHeight*3];
		memset(picturesFromCams[i],255,videoWidth*videoHeight*3);
		cameraFlagsInit[i] = false;
	}
	_beginthread(showCamerasSerial, 0, NULL);


	// не хватает проверки на правильность конфига
	int ids[cameraNumber];
	readCameraConfig(hWnd, ids);

	
	int value = 0;
	int cameraIndex = 0;
	for (int col = 0; col < 4; col++)
	{
		for (int row = 0; row < 4; row++)
		{
			/*HWND hVWnd = CreateWindow(szVideoWindowClass, L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 10 + hGap*col + videoWidth*col, 5 + vGap*row + videoHeight*row, videoWidth, videoHeight, hWnd, NULL, hInst, NULL);
			hVWnds.push_back(hVWnd);*/

			hVWnds[cameraIndex] = CreateWindow(szVideoWindowClass, L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 10 + hGap*col + videoWidth*col, 5 + vGap*row + videoHeight*row, videoWidth, videoHeight, hWnd, NULL, hInst, NULL);;
			
			
			/*
			выводит под окном адрес его HWND
			HWND hWndComboBox = CreateWindow(L"static", L"ST_U",
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                              10 + hGap*col + videoWidth*col, 8 + vGap*row + videoHeight*row + videoHeight , 100, 20,
                              hWnd, NULL,
                              hInst, NULL);
			char szBuffer1[15] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
			itoa((int)hVWnds[cameraIndex], szBuffer1, 16);
			SetWindowText(hWndComboBox, convertCharArrayToLPCWSTR(szBuffer1));
			*/

			HWND hWndComboBox = CreateWindow(L"ComboBox", L"", CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 10 + hGap*col + videoWidth*col, 8 + vGap*row + videoHeight*row + videoHeight , 50, 300, hWnd, NULL, hInst, NULL);
			hWndCBs.push_back(hWndComboBox);
			TCHAR cameraIndexItems[16][4] =  
			{
				TEXT("0"), TEXT("1"), TEXT("2"), TEXT("3"), 
				TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), 
				TEXT("8") , TEXT("9"), TEXT("10"), TEXT("11"),
				TEXT("12") , TEXT("13"), TEXT("14"), TEXT("15")
			};
			
			TCHAR cameraIndexItem[4]; 

			memset(&cameraIndexItem, 0, sizeof(cameraIndexItem));       
			for (int k = 0; k < 16; k++)
			{
				wcscpy_s(cameraIndexItem, sizeof(cameraIndexItem)/sizeof(TCHAR),  (TCHAR*)cameraIndexItems[k]);
				
				// Add string to combobox.
				SendMessage(hWndComboBox,(UINT) CB_ADDSTRING, (WPARAM) 0, (LPARAM) cameraIndexItem); 
			}
			SendMessage(hWndComboBox, CB_SETCURSEL, ids[cameraIndex], 0);
			//if (cameraIndex < cameraNumber)
			//{
			
				ThreadParams tp = {cameraIndex, NULL, videoWidth, videoHeight};
				/*
				DWORD dwThreadId;
				HANDLE hCurThread = CreateThread(
					NULL, //Choose default security
					0, //Default stack size
					(LPTHREAD_START_ROUTINE)&drawPictureFromCam, //Routine to execute
					(LPVOID) &tp, //Thread parameter
					0, //Immediately run the thread
					&dwThreadId //Thread Id
					);		
				*/
				
				

				winDC[cameraIndex] = GetDC(hVWnds[cameraIndex]);
				//ReleaseDC(hVWnds[cameraIndex], hdc1);
				//HDC hdc2 = GetDC(hVWnds[cameraIndex]);
				_beginthread(drawPictureFromCam, 0, &tp);

				while(tp.height != 0) 
				{
					Sleep(10);
				}
				
			//}
			cameraIndex++;
		}
	}

	//ShowWindow(hVWnds[cameraIndex], nCmdShow);
	//UpdateWindow(hVWnds[cameraIndex]);

	/*ThreadParams tp2 = {0, 0, videoWidth, videoHeight};
	_beginthread(readPicturesFromCams, 0, &tp2);
	while(tp2.height != 0) 
	{
		Sleep(10);
	}*/
	/***********************************************/

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FRT));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: RegisterMainWindowClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM RegisterMainWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FRT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

ATOM RegisterVideoWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_OWNDC ;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FRT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(SS_BLACKFRAME);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szVideoWindowClass;
	wcex.hIconSm		= NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0, 736, 720, NULL, NULL, hInstance, NULL);

	HWND hWndButton = CreateWindow(L"Button", L"Save config", WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON, 310, 650, 100, 20, hWnd, (HMENU)IDC_MAIN_BUTTON, hInst, NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//HWND hVWnd = CreateWindow(szVideoWindowClass, L"WTF", WS_CHILD, 100, 100, 50, 50, hWnd, NULL, hInstance, NULL);

	/*//Дочернее окно для вывода видео
	WNDCLASS wc;
	GetClassInfo(hInst,L"szWindowClass",&wc);
	wc.lpszClassName = L"video";
	wc.hCursor = 0;
	wc.hIcon = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RegisterClass(&wc);*/
	
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDC_MAIN_BUTTON:
			PutIDsIntoFile(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void drawPictureFromCam(void *tp)
{
	try
	{
		ThreadParams * t = (ThreadParams*)tp;
		int camID = t->camID; 
		//HWND *hWnd = t->hWnd; 
		int width = t->width;
		int height = t->height;
		HWND hWnd2 = hVWnds[camID]; 
		t->height = 0;

		//HDC hdc = GetDC(hWnd2);
		/*const POINT ptZero = { 0, 0 };
		HMONITOR hmon = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO mi = { sizeof(mi) };*/

		int init = init_OpenGL(hWnd2);//Результат инициализации OpenGL
		if(init<1)
		{
			MessageBox(NULL, L"OpenGL initialization Error", L"Main window", MB_OK);
			//exit ( 0 ) ;
		}

		Resize(width, height);
		Init();

		char *img1= new char[width*height*3];

		//IplImage *ii = cvCreateImage(cvSize(height,width*2),8,3);
		//char szBuffer1[5] = {NULL,NULL,NULL,NULL,NULL};
		//itoa(camID, szBuffer1, 10);
		
		while(1)
		{
			//img1 = picturesFromCams[camID];
			//memset(img1,255,width*height*3);
			/*
			int result = getFotosFromCams(img1, width, height, camID, false);
			if (result != 1)
			{
				MessageBox(hWnd,  L"Can't getFotosFromCams" , L"Error", MB_OK);
				//return;
			}
			*/

			//getPic(ii);
			/*if(ii)
			{
				memcpy (img1, ii->imageData, width*2*height*3) ;
			}*/
			//cvReleaseImage(&ii);

			customFlip(picturesFromCams[camID], width, height, img1);
			
			
			

			/*IplImage *im = cvCreateImage(cvSize(width,height),8,3);
			memcpy(im->imageData,img1,width*height*3);
			char szBuffer1[15] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
			//itoa(camID, szBuffer1, 10);
			itoa((int)hWnd2, szBuffer1, 16);
			cvShowImage(szBuffer1,im);
			cvWaitKey(33);
			cvReleaseImage(&im);*/
			

			glRasterPos2f(0, 0);
			//cvFlip(ii);
			glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, img1);
			SwapBuffers(winDC[camID]);
			//Sleep(100);
			/*int key = cvWaitKey(33);
			if(key == 27)
			{
				MessageBox(hWnd, L"Esc", L"Error", MB_OK);
				break;
			}*/
			
		}

		//ReleaseDC(hWnd2,hdc);
		//cvReleaseImage(&ii);
		//showVideo = true;
	}
	catch(...)
	{
		MessageBox(NULL, L"Error", L"Error", MB_OK);
	}
}

//
//void readPicturesFromCams(void *tp)
//{
//	ThreadParams * t = (ThreadParams*)tp;
//	int width = t->width;
//	int height = t->height;
//	t->height = 0;
//
//	while(1)
//	{
//		for (int camID = 0; camID < cameraNumber; camID++)
//		{
//			int result = getFotosFromOneCam(picturesFromCams[camID], width, height, camID, false, 2, &cameraFlagsInit[camID], &cameraFlagsCaptureEnabled[camID], true);
//			/*IplImage *im = cvCreateImage(cvSize(width,height),8,3);
//			memcpy(im->imageData,picturesFromCams[camID],width*height*3);
//			cvShowImage("im",im);
//			cvWaitKey(33);
//			cvReleaseImage(&im);*/
//
//			//if (result != 1)
//			//{
//			//	MessageBox(NULL,  L"Can't getFotosFromOneCam" , L"Error", MB_OK);
//			//	//return;
//			//}
//		}
//	}
//
//}

////////////////////////////////////////////////////////////////////////////////
//Инициализация OpenGL														||||
int init_OpenGL(HWND hwnd)
{
 HDC hdc = GetDC(hwnd);
 //Установка параметров контекста воспроизведения OpenGL
 PIXELFORMATDESCRIPTOR pfd;
    pfd.nSize    = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;
    pfd.cRedBits   = 0;
    pfd.cRedShift   = 0;
    pfd.cGreenBits   = 0;
    pfd.cGreenShift   = 0;
    pfd.cBlueBits   = 0;
    pfd.cBlueShift   = 0;
    pfd.cAlphaBits   = 0;
    pfd.cAlphaShift   = 0;
    pfd.cAccumBits   = 0;
    pfd.cAccumRedBits  = 0;
    pfd.cAccumGreenBits  = 0;
    pfd.cAccumBlueBits  = 0;
    pfd.cAccumAlphaBits  = 0;
    pfd.cDepthBits   = 32;
    pfd.cStencilBits  = 0;
    pfd.cAuxBuffers   = 0;
    pfd.iLayerType   = PFD_MAIN_PLANE;
    pfd.bReserved   = 0;
    pfd.dwLayerMask   = 0;
    pfd.dwVisibleMask  = 0;
    pfd.dwDamageMask  = 0;
 //Попытка подобрать подходящий формат пикселя
    int PixelFormat = ChoosePixelFormat( hdc, &pfd);
    if(PixelFormat == 0)
    {
  return 0;
    }
 //Устанавливаем формат пикселя
    if(SetPixelFormat( hdc, PixelFormat, &pfd) == FALSE)
 {
  return 0;
 }
 //Создаем контекст воспроизведения OpenGL
 HGLRC hGLRC = wglCreateContext(hdc);
 if(hGLRC == NULL)
 {
  return 0;
 }
 //Делаем его текущим
 if(wglMakeCurrent(hdc, hGLRC) == false)
 {
  return 0;
 }

 ReleaseDC(hwnd,hdc);

 return 1;
}

//Дополнительные настройки OpenGL											||||
void Init()
{
 //Разрешаем плавное цветовое сглаживание
 glShadeModel(GL_SMOOTH);
 //Устанавливаем цвет отчистки экрана
 glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
 //Разрешаем отчистку буфера глубины
 glClearDepth(1.0f);
}

//Изменение размеров окна													||||
void Resize(int width, int height)
{
 //Устанавливаем область вывода для приложений OpenGL
 glViewport(0, 0, width, height);
 //Выбираем матрицу проекции
 glMatrixMode(GL_PROJECTION);
 //Сброс матрицы проекции
 glLoadIdentity();
 //Установка ортогональной проекции
 glOrtho(0,width,0,height,-1.0,1.0);
 //Выбираем матрицу вида модели
 glMatrixMode(GL_MODELVIEW);
 //Сброс матрицы вида модели
 glLoadIdentity();
}
////////////////////////////////////////////////////////////////////////////////

void PutIDsIntoFile(HWND hwnd)
{
	ofstream out("../camera.conf", ios::out);
	for(int i = 0; i < 16; i++)
	{
		//get selected item index
		int itemIndex = SendMessage(hWndCBs[i], (UINT) CB_GETCURSEL,(WPARAM) 0, (LPARAM) 0);
		if(i > 0)
			out<<endl;
		out<<itemIndex;
	}
	out.close();
	return;
	/*int ItemIndex = SendMessage(hWndCBs[2], (UINT) CB_GETCURSEL,(WPARAM) 0, (LPARAM) 0);
    TCHAR  ListItem[256];
	SendMessage(hWndCBs[2], (UINT) CB_GETLBTEXT, (WPARAM) ItemIndex, (LPARAM) ListItem);
    MessageBox(hwnd, (LPCWSTR) ListItem, TEXT("Item Selected"), MB_OK);*/
	
	HANDLE hFile;

	try
	{
		hFile = CreateFile(_T("../camera.conf"),       // file to open
							GENERIC_WRITE,          // open for writing
							0,						// share for writing
							NULL,					// default security
							CREATE_ALWAYS,			// Opens a file, always. If the specified file exists, the function succeeds and the last-error code is set to ERROR_ALREADY_EXISTS (183). If the specified file does not exist and is a valid path to a writable location, the function creates a file and the last-error code is set to zero.
							FILE_ATTRIBUTE_NORMAL,	// normal file
							NULL);					// no attr. template

		if (hFile == INVALID_HANDLE_VALUE)
			throw exception("CreateFile('camera.conf') failed.");

		/*
		DWORD dwPtr = SetFilePointer( hFile, 0, NULL, FILE_END); //set pointer position to end file

		if (dwPtr == INVALID_SET_FILE_POINTER)
			throw L"SetFilePointer('camera.conf') failed.";
		*/

		_TCHAR writeBuffer[1000] = L"";
		const _TCHAR tchRN[10] = L"\r\n";

		for(int i = 0; i < 16; i++)
		{
			//get selected item index
			int itemIndex = SendMessage(hWndCBs[i], (UINT) CB_GETCURSEL,(WPARAM) 0, (LPARAM) 0);
			if (itemIndex == -1)
				throw L"Select a value in all the dropdowns, please!";

			//convert it into _TCHAR array
			_TCHAR tchItemIndex[10] = L"";
			_stprintf(tchItemIndex, _T("%i"), itemIndex);

			//concatenate it with "\r\n"
			_tcscat_s(tchItemIndex, tchRN);
			
			//and add to the main buffer
			_tcscat_s(writeBuffer, tchItemIndex);
		}

		// Write to File
		bool bErrorFlag = false;
		LPWSTR data = writeBuffer;
		DWORD dwBytesToWrite = lstrlenW(data)*2;
		DWORD dwBytesWritten;
		bErrorFlag = WriteFile( hFile,          // open file handle
								data,			// start of data to write
								dwBytesToWrite, // number of bytes to write
								&dwBytesWritten,			// number of bytes that were written
								NULL);          // no overlapped structure

		if (bErrorFlag == false)
			throw exception("WriteFile('camera.conf') failed.");
	}
	catch(exception &e)
	{
		LPCWSTR errorDetails;
		int errorCode = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorDetails, 0, NULL );
		wstring s1(convertCharArrayToLPCWSTR(e.what()));
		wstring s2(errorDetails);
		wstring s3 = s1 + L" " + s2;
		MessageBox(hwnd, s3.c_str() , L"Error", MB_OK);
	}
	catch(LPCWSTR errorDesc)
	{
		MessageBox(hwnd, errorDesc , L"Error", MB_OK);
	}

	CloseHandle(hFile);
}

void customFlip(char* oldImg, int width, int height, char* newImg)
{
	//newImg = new char[width*height*3];

	for (int row = 0; row < height; row++)
	{
		memcpy(&newImg[(height - row - 1) * width * 3], &oldImg[row * width * 3], width * 3);
	}

	//return flippedImage;
}

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString=new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}

void initCams(void *tp)
{
	ThreadParams2 * t1 = (ThreadParams2*)tp;
	ThreadParams2 t2 = *t1; 
	ThreadParams2 *t = &t2;
	t1->height = 0;
	int res = getFotosFromOneCam(t->camImage, t->width, t->height, t->camID, false, 2, t->flagInit, t->flagCaptureEnabled, true);
	int a = 1+1;
}




void showCamerasSerial(void *tp)
{
	while(true)
	{
		for(int i = 0; i < cameraNumber; i++)
		{
			int res = getFotosFromOneCam(picturesFromCams[i], videoWidth, videoHeight, i, false, 1, &cameraFlagsInit[i], &cameraFlagsCaptureEnabled[i], true);

		}
	}
}

void readCameraConfig(HWND hwnd, int * ids)
{
	ifstream myStream("../camera.conf", ios::in);
	if(!myStream.fail())
	{
		int index = 0;
		while(!myStream.eof())
		{
			myStream>>ids[index++];
		}
		//int arrSize = 0;
		//int counter = -1;
		//while(!myStream.eof())
		//{
		//	myStream>>counter;
		//	/*char t[255];
		//	myStream.getline(t, 255);*/
		//	//myStream.getline(t, 255);
		//	arrSize++;
		//}

		//counter = 0;
		//int * arr = new int[arrSize];
		//while(!myStream.eof())
		//{
		//	//myStream.getline();
		//	
		//	myStream>>arr[counter++];
		//}
		//return arr;
	}
	return;
	HANDLE hFile;
	try
	{
		hFile = CreateFile(_T("../camera.conf"),       // file to open
							GENERIC_READ,          // open for writing
							0,						// share for writing
							NULL,					// default security
							OPEN_EXISTING,			// Opens a file or device, only if it exists. If the specified file or device does not exist, the function fails and the last-error code is set to ERROR_FILE_NOT_FOUND (2).
							FILE_ATTRIBUTE_NORMAL,	// normal file
							NULL);					// no attr. template
		if (hFile == INVALID_HANDLE_VALUE)
			throw L"CreateFile('camera.conf') failed.";
		
		DWORD dwPtr = SetFilePointer( hFile, 0, NULL, FILE_BEGIN); //set pointer position to the beginning of the file
		if (dwPtr == INVALID_SET_FILE_POINTER)
			throw L"SetFilePointer('camera.conf') failed.";
		
		bool bErrorFlag = false;
		const _TCHAR tchRN[3] = L"\r\n";
		const int bufferSize = 1000;
		_TCHAR readBuffer[bufferSize] = {0};
		DWORD dwBytesWereRead;
		bErrorFlag = ReadFile(  hFile,          // open file handle
								readBuffer,		// A pointer to the buffer that receives the data read from a file or device.
								bufferSize-1,	// number of bytes to read
								&dwBytesWereRead,// number of bytes that were read
								NULL);          // no overlapped structure
		if (bErrorFlag == false)
			throw L"ReadFile('camera.conf') failed.";

		wstring s(readBuffer);
		wstring delimiter(tchRN);
		wstring token;
		size_t pos = 0;

		int index = 0;
		while ((pos = s.find(delimiter)) != wstring::npos) {
			token = s.substr(0, pos);
			//int id = stoi(token);
			ids[index++] = stoi(token);
			s.erase(0, pos + delimiter.length());
		}
	}
	catch(LPCWSTR errorDesc)
	{
		LPCWSTR errorDetails;
		int errorCode = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorDetails, 0, NULL );
		wstring s1(errorDesc);
		wstring s2(errorDetails);
		wstring s3 = s1 + L" " + s2;
		MessageBox(hwnd, s3.c_str() , L"Error", MB_OK);
	}

	CloseHandle(hFile);
}