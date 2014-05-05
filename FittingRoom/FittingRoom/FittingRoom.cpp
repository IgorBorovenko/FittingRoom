#pragma region Includes
#include "stdafx.h"
#include "FittingRoom.h"
#include "SpinigeController.h"
#include "glut.h"
#include "glut.h"
#pragma endregion

#pragma region Main Window 
// Main Window vars
HDC hDc_Main;
HWND hWnd_Main;
bool mainWindowNeedsRefresh = false;
TCHAR szWindowClass_Main[MAX_LOADSTRING]; // the main window class name
// Main Window funcs
void drawMain(void * args);
BOOL CreateWindow_Main(MONITORINFO monitorInfo, int nCmdShow, bool isShowInWindow);
LRESULT CALLBACK WndProc_Main(HWND, UINT, WPARAM, LPARAM);
void TouchEndHandler_Main();
void openGlDrawText(GLfloat x, GLfloat y, char* text, float size);
#pragma endregion

#pragma region Gallery Window 
// Gallery Window vars
struct FolderProps{wstring name; RECT winRect; bool marked; IplImage* thumbnail;};
HDC hDc_Gallery;
HWND hWnd_Gallery;
HWND hWndGalleryCompareButton;
vector <FolderProps> folders;
bool galleryWindowNeedsRefresh = false;
bool skipFindFolders = false;
TCHAR szWindowClass_Gallery[MAX_LOADSTRING] = L"szWindowClass_Gallery"; // the main window class name
bool showCountdownWarning = false;
// Gallery Window funcs
BOOL CreateWindow_Gallery(MONITORINFO monitorInfo, int nCmdShow, bool isShowInWindow);
LRESULT CALLBACK WndProc_Gallery(HWND, UINT, WPARAM, LPARAM);
void drawGallery(void * args);
void findFoldersWithPictures();
void takeSnapshots(void * args);
void openGlDrawRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, unsigned int rgba);
#pragma endregion

#pragma region Session Window 
// Session Window vars
struct PictureProps{wstring name; IplImage* image;};
HDC hDc_Session;
HWND hWnd_Session;
int sessionCurrentFolderIndex = -1;
int sessionCurrentPictureIndex = -1;
bool sessionWindowNeedsRefresh = false;
bool skipFindPicturesInFolder = false;
vector <PictureProps> picturesInCurrentFolder;
TCHAR szWindowClass_Session[MAX_LOADSTRING] = L"szWindowClass_Session"; // the main window class name
ULONGLONG ticksAtTouchStart = 0;
// Session Window funcs
BOOL CreateWindow_Session(MONITORINFO monitorInfo, int nCmdShow, bool isShowInWindow);
LRESULT CALLBACK WndProc_Session(HWND, UINT, WPARAM, LPARAM);
void TouchEndHandler_Session();
void drawSession(void * args);
void findPicturesInCurrentFolder();
void gallerySelectFolder(int folderIndex);
void galleryUnselectFolder(int folderIndex);
#pragma endregion

#pragma region Comparsion Window
// Comparsion Window vars
HDC hDc_Comparsion;
HWND hWnd_Comparsion;
TCHAR szWindowClass_Comparsion[MAX_LOADSTRING] = L"szWindowClass_Comparsion"; // the main window class name
bool comparsionWindowNeedsRefresh = false;
int comparsionCurreentPictureIndex = -1;
bool skipFindPicturesInSelectedFolder = false;
// Comparsion Window funcs
BOOL CreateWindow_Comparsion(MONITORINFO monitorInfo, int nCmdShow, bool isShowInWindow);
LRESULT CALLBACK WndProc_Comparsion(HWND, UINT, WPARAM, LPARAM);
void drawComparsion(void * args);
void TouchEndHandler_Comparsion();
int findPicturesInSelectedFolders();
vector<PictureProps> picsToCompare[4];
#pragma endregion

// my global variables
HINSTANCE hInst; // current instance
bool finishAllThreads = false;
TCHAR szTitle[MAX_LOADSTRING];	// The title bar text
int CurrentTouchesCount = 0;
vector <TOUCHINPUT> CurrentTouches;
CamerasController Cameras;
int snapshotWarningStatus = 0;
SpinigeController spinige;
// my declarations
BOOL InitInstance(HINSTANCE, int);
ATOM RegisterWindowClass(HINSTANCE hInstance, WNDPROC wndProc, LPCWSTR windowName);
int GetTouchesResult();
void TouchHandler(TOUCHINPUT touch);
int InitializeOpenGL(HWND _hWnd, int monitorWidth, int monitorHeight);
wchar_t *convertCharArrayToLPCWSTR(const char* charArray);
void calculateScaledImageSize(int placeholderW, int placeholderH, int originalW, int originalH, int* newW, int* newH);
IplImage *rotateImage(IplImage *src);
void loadPicturesFromFolderIntoVector(int folderIndex, vector<PictureProps> *pics, int placeholderW, int placeholderH);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FITTINGROOM, szWindowClass_Main, MAX_LOADSTRING);
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;
	// initialize CamerasController
	int errorCode = Cameras.InitializeCameras();
	if(errorCode != 0)
	{
		// initialization error handling
		switch(errorCode)
		{
			case -1:
				MessageBoxA(hWnd_Main, "Cannot read config file", "Error", MB_OK);
				break;
			case -2:
				MessageBoxA(hWnd_Main, "Cannot find active cameras", "Error", MB_OK);
				break;
		}
	}
	else
	{
		Cameras.BeginShow();
		_beginthread(drawMain, 0, NULL);
	}

	//start gallery display thread
	_beginthread(drawGallery, 0, NULL);
	
	//start session display thread
	_beginthread(drawSession, 0, NULL);

	//start session display thread
	_beginthread(drawComparsion, 0, NULL);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FITTINGROOM));
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

//   FUNCTION: InitInstance(HINSTANCE, int)
//   PURPOSE: Saves instance handle and creates main window
//   COMMENTS:
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// Store instance handle in our global variable
	hInst = hInstance; 
	// get monitor settings
	MONITORINFO monitorInfo = { sizeof(monitorInfo) };
	POINT point = { 0, 0 };
	HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);
	if(!GetMonitorInfo(monitor, &monitorInfo))
		return FALSE;

	bool isShowInWindow = false;
	bool mainWindowOk = CreateWindow_Main(monitorInfo, nCmdShow, isShowInWindow);
	bool galleryWindowOk = CreateWindow_Gallery(monitorInfo, nCmdShow, isShowInWindow);
	bool sessionWindowOk = CreateWindow_Session(monitorInfo, nCmdShow, isShowInWindow);
	bool comparsionWindowOk = CreateWindow_Comparsion(monitorInfo, nCmdShow, isShowInWindow);
	if (mainWindowOk && galleryWindowOk && sessionWindowOk && comparsionWindowOk)
		return true;
	else
		return false;
}
/*=================================================================================================================================*/
ATOM RegisterWindowClass(HINSTANCE hInstance, WNDPROC wndProc, LPCWSTR windowName)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_OWNDC; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= wndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FITTINGROOM));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0; //MAKEINTRESOURCE(IDR_MAINFRAME);
	wcex.lpszClassName	= windowName;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassEx(&wcex);
}
/*=================================================================================================================================*/
BOOL CreateWindow_Main(MONITORINFO monitorInfo, int nCmdShow, bool isShowInWindow)
{
	RegisterWindowClass(hInst, WndProc_Main, szWindowClass_Main);
	if(isShowInWindow)
	{
		hWnd_Main = CreateWindow(szWindowClass_Main, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);
	}
	else
	{
		hWnd_Main = CreateWindow(szWindowClass_Main, 
								 L"Fitting Room",
								 WS_POPUP | WS_VISIBLE, 
								 monitorInfo.rcMonitor.left,
								 monitorInfo.rcMonitor.top, 
								 monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, 
								 monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
								 NULL, NULL, hInst, 0);
	}
	if (!hWnd_Main)
	{
		LPCWSTR errorDetails;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorDetails, 0, NULL);
		MessageBox(NULL, errorDetails, L"Main window init error", MB_OK);
		return FALSE;
	}
	hDc_Main = GetDC(hWnd_Main);
	ShowWindow(hWnd_Main, nCmdShow);
	UpdateWindow(hWnd_Main);
	RegisterTouchWindow(hWnd_Main, 0); 
	return TRUE;
}
/*=================================================================================================================================*/
BOOL CreateWindow_Gallery(MONITORINFO monitorInfo, int nCmdShow, bool isShowInWindow)
{
	// create the gallery window
	RegisterWindowClass(hInst, WndProc_Gallery, szWindowClass_Gallery);
	if(isShowInWindow)
	{
		hWnd_Gallery = CreateWindow(szWindowClass_Gallery, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);
	}
	else
	{
		hWnd_Gallery = CreateWindow(szWindowClass_Gallery,
									L"Gallery",
									WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
									monitorInfo.rcMonitor.left, 
									monitorInfo.rcMonitor.top,
									monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
									monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
									NULL, NULL,  hInst, 0);
	}
	if (!hWnd_Gallery)
	{
		LPCWSTR errorDetails;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorDetails, 0, NULL);
		MessageBox(NULL, errorDetails, L"Gallery init error", MB_OK);
		return FALSE;
	}
	// draw a button "Back"  
	HWND hWndGalleryBackButton = CreateWindow(TEXT("button"), 
											  TEXT("<-- Back"),
											  WS_VISIBLE | WS_CHILD /*| BS_DEFPUSHBUTTON*/,
											  monitorInfo.rcMonitor.left, 
											  monitorInfo.rcMonitor.top,
											  (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left) / 2 - 5,
											  100,
											  hWnd_Gallery,
											  (HMENU)GALLERY_BACK_BUTTON,
											  hInst,
											  NULL);

	hWndGalleryCompareButton = CreateWindow(TEXT("button"), 
											  TEXT("Compare"),
											  WS_VISIBLE | WS_CHILD /*| BS_DEFPUSHBUTTON*/,
											  (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left) / 2 + 5, 
											  monitorInfo.rcMonitor.top,
											  (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left) / 2 - 5,
											  100,
											  hWnd_Gallery,
											  (HMENU)GALLERY_COMPARE_BUTTON,
											  hInst,
											  NULL);
	ShowWindow(hWndGalleryCompareButton, SW_HIDE);

	hDc_Gallery = GetDC(hWnd_Gallery);
	ShowWindow(hWnd_Gallery, 0);
	RegisterTouchWindow(hWnd_Gallery, 0);
	return TRUE;
}
/*=================================================================================================================================*/
BOOL CreateWindow_Session(MONITORINFO monitorInfo, int nCmdShow, bool isShowInWindow)
{
	// create the session window
	RegisterWindowClass(hInst, WndProc_Session, szWindowClass_Session);
	if(isShowInWindow)
	{
		hWnd_Session = CreateWindow(szWindowClass_Session, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);
	}
	else
	{
		hWnd_Session = CreateWindow(szWindowClass_Session,
									L"Session",
									WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
									monitorInfo.rcMonitor.left, 
									monitorInfo.rcMonitor.top,
									monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
									monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
									NULL, NULL,  hInst, 0);
	}
	if (!hWnd_Session)
	{
		LPCWSTR errorDetails;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorDetails, 0, NULL);
		MessageBox(NULL, errorDetails, L"Session init error", MB_OK);
		return FALSE;
	}
	// draw a button "Back" 
	HWND hWndSessionBackButton = CreateWindow(TEXT("button"), 
											  TEXT("<-- Back"),
											  WS_VISIBLE | WS_CHILD /*| BS_DEFPUSHBUTTON*/,
											  monitorInfo.rcMonitor.left, 
											  monitorInfo.rcMonitor.top,
											  monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
											  100,
											  hWnd_Session,
											  (HMENU)SESSION_BACK_BUTTON,
											  hInst,
											  NULL);
	hDc_Session = GetDC(hWnd_Session);
	ShowWindow(hWnd_Session, 0);
	RegisterTouchWindow(hWnd_Session, 0);
	return TRUE;
}

BOOL CreateWindow_Comparsion(MONITORINFO monitorInfo, int nCmdShow, bool isShowInWindow)
{
	// create the session window
	RegisterWindowClass(hInst, WndProc_Comparsion, szWindowClass_Comparsion);
	if(isShowInWindow)
	{
		hWnd_Comparsion = CreateWindow(szWindowClass_Comparsion, L"Comparsion", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);
	}
	else
	{
		hWnd_Comparsion = CreateWindow(szWindowClass_Comparsion,
									L"Comparsion",
									WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
									monitorInfo.rcMonitor.left, 
									monitorInfo.rcMonitor.top,
									monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
									monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
									NULL, NULL,  hInst, 0);
	}
	if (!hWnd_Comparsion)
	{
		LPCWSTR errorDetails;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorDetails, 0, NULL);
		MessageBox(NULL, errorDetails, L"Comparsion init error", MB_OK);
		return FALSE;
	}
	// draw a button "Back" 
	HWND hWndComparsionBackButton = CreateWindow(TEXT("button"), 
											  TEXT("<-- Back"),
											  WS_VISIBLE | WS_CHILD /*| BS_DEFPUSHBUTTON*/,
											  monitorInfo.rcMonitor.left, 
											  monitorInfo.rcMonitor.top,
											  monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
											  100,
											  hWnd_Comparsion,
											  (HMENU)COMPARSION_BACK_BUTTON,
											  hInst,
											  NULL);
	hDc_Comparsion = GetDC(hWnd_Comparsion);
	ShowWindow(hWnd_Comparsion, 0);
	RegisterTouchWindow(hWnd_Comparsion, 0);
	return TRUE;
}

/*=================================================================================================================================*/
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//  PURPOSE:  Processes messages for the main window.
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
		case WM_MOUSEMOVE:
			if (wParam & MK_LBUTTON) 
            {
				POINTS points = MAKEPOINTS(lParam); 	
				TOUCHINPUT touch = { points.x * 100, points.y * 100, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
				TouchHandler(touch);
			}
			break;
		case WM_LBUTTONUP:
			TouchEndHandler_Main();
			break;
		case WM_TOUCH:
			{
				// Number of actual per-contact messages
				unsigned int inputsCount = (unsigned int) wParam;
				// Allocate the storage for the parameters of the per-contact messages
				TOUCHINPUT * touches = new TOUCHINPUT[inputsCount];
				if (touches != NULL)
				{
					// Unpack message parameters into the array of TOUCHINPUT structures, each representing a message for one single contact.
					if (GetTouchInputInfo((HTOUCHINPUT)lParam, inputsCount, touches, sizeof(TOUCHINPUT)))
					{
						// For each contact, dispatch the message to the appropriate message handler.
						for (unsigned int i = 0; i < inputsCount; i++)
						{
							if (touches[i].dwFlags & TOUCHEVENTF_DOWN)
							{
								CurrentTouchesCount++;
								TouchHandler(touches[i]);
							}
							else if (touches[i].dwFlags & TOUCHEVENTF_MOVE)
								TouchHandler(touches[i]);
							else if (touches[i].dwFlags & TOUCHEVENTF_UP)
							{
								CurrentTouchesCount--;
								TouchHandler(touches[i]);
								if (CurrentTouchesCount == 0)
									TouchEndHandler_Main();
							}
						}
					}
					delete [] touches;
					touches = NULL;
				}
				CloseTouchInputHandle((HTOUCHINPUT)lParam);
				break;
			}
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		//case WM_PAINT:
		//	hdc = BeginPaint(hWnd, &ps);
		//	// TODO: Add any drawing code here...
		//	EndPaint(hWnd, &ps);
		//	break;
		case WM_DESTROY:
			finishAllThreads = true;
			PostQuitMessage(0);
			break;
		default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK WndProc_Gallery(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rcClient;                 // client area rectangle 
	POINT ptClientUL;              // client upper left corner 
    POINT ptClientLR;              // client lower right corner 
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	DWORD dwThreadId;
	DWORD a = 0;

	switch (message)
	{
		case WM_LBUTTONDOWN:
            {
				//remember ticks just user touched the gallery
				if (ticksAtTouchStart == 0)
					ticksAtTouchStart = GetTickCount64();
			}
			break;
		case WM_MOUSEMOVE:
			if (wParam & MK_LBUTTON) 
            {
				//MessageBox(hWnd_Gallery, L"mouse move in the gallery", L"Error", MB_OK);
				//InvalidateRect(hWndGallery, NULL, false);
			}
			break;
		case WM_LBUTTONUP:
			{
				//get current ticks
				ULONGLONG ticksAtTouchEnd = GetTickCount64();
				//calculate how many ticks passed since a touch started
				ULONGLONG ticksPassed = ticksAtTouchEnd - ticksAtTouchStart;
				//reset variable for a next use
				ticksAtTouchStart = 0;
				bool longTouch = (ticksPassed > 700);

				POINTS pts = MAKEPOINTS(lParam);
				POINT pt = {pts.x, pts.y};
				for (int folderIndex = 0; folderIndex < folders.size(); folderIndex++)
				{
					//check if the click was made inside the rect of some picture
					if (PtInRect(&folders[folderIndex].winRect, pt))
					{
						/*wchar_t szBuffer[10];
						_stprintf(szBuffer, _T("%i"), folderIndex);
						MessageBox(hWnd_Gallery,  szBuffer , L"info", MB_OK);*/

						if (longTouch == true)
						{
							//count currently selected folders
							int markedFoldersCount = 0;
							for (int i = 0; i < folders.size(); i++)
								if (folders[i].marked == true)
									markedFoldersCount++;

							//select or unselect current folder depending on the current selection
							if (folders[folderIndex].marked == true)
								galleryUnselectFolder(folderIndex);
							else 
								if (markedFoldersCount < 4)
									gallerySelectFolder(folderIndex);

							//refresh "Compare" button visibility
							markedFoldersCount = 0;
							for (int i = 0; i < folders.size(); i++)
								if (folders[i].marked == true)
									markedFoldersCount++;
							if (markedFoldersCount > 1)
								ShowWindow(hWndGalleryCompareButton, SW_SHOW);
							else
								ShowWindow(hWndGalleryCompareButton, SW_HIDE);
						}
						else
						{
							//show photo session
							ShowWindow(hWnd_Session, SW_SHOW);
							UpdateWindow(hWnd_Session);
							sessionCurrentFolderIndex = folderIndex;
							sessionCurrentPictureIndex = 0;
							skipFindPicturesInFolder = false;
							sessionWindowNeedsRefresh = true;
						}

						break;
					}
				}


				break;
			}
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case GALLERY_BACK_BUTTON:
					Cameras.BeginShow();
					ShowWindow(hWnd_Gallery, 0);
					ShowWindow(hWnd_Main, 1);
					UpdateWindow(hWnd_Main);
					break;
				case GALLERY_COMPARE_BUTTON:
					//ShowWindow(hWnd_Gallery, SW_HIDE);

					comparsionCurreentPictureIndex = 0;
					skipFindPicturesInSelectedFolder = false;
					ShowWindow(hWnd_Comparsion, SW_SHOW);
					UpdateWindow(hWnd_Comparsion);
					comparsionWindowNeedsRefresh = true;
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		//case WM_PAINT:
		//	hDc_Gallery = BeginPaint(hWnd, &ps);
		//	// TODO: Add any drawing code here...
		//	EndPaint(hWnd, &ps);
		//	break;
		case WM_CLOSE:
			ShowWindow(hWnd_Gallery, 0);
			ShowWindow(hWnd_Main, 1);
			UpdateWindow(hWnd_Main);
			break;
		case WM_DESTROY:
			//ShowWindow(hWndGallery, 0);
			DestroyWindow(hWnd_Gallery);
			//PostQuitMessage(0);
			break;
	
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK WndProc_Session(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rcClient;                 // client area rectangle 
	POINT ptClientUL;              // client upper left corner 
    POINT ptClientLR;              // client lower right corner 
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	//FLICK_DATA fd;
	DWORD dwThreadId;
	DWORD a = 0;

	switch (message)
	{
		case WM_MOUSEMOVE:
			if (wParam & MK_LBUTTON) 
            {
				POINTS pts = MAKEPOINTS(lParam); 
				
				TOUCHINPUT ti = {
					pts.x * 100,
					pts.y * 100,
					NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
				};

				TouchHandler(ti);
			}
			break;

		case WM_LBUTTONUP:
			{
				TouchEndHandler_Session();
				/*int direction = GetTouchesResult();
				sessionWindowDirectionHandler(direction);*/

			
				break;
			}
        case WM_TOUCH:
            {
                // WM_TOUCH message can contain several messages from different contacts
                // packed together.
                // Message parameters need to be decoded:
                unsigned int numInputs = (unsigned int) wParam; // Number of actual per-contact messages
                TOUCHINPUT* ti = new TOUCHINPUT[numInputs]; // Allocate the storage for the parameters of the per-contact messages
                if (ti == NULL)
                {
                    break;
                }
                // Unpack message parameters into the array of TOUCHINPUT structures, each
                // representing a message for one single contact.
                if (GetTouchInputInfo((HTOUCHINPUT)lParam, numInputs, ti, sizeof(TOUCHINPUT)))
                {
                    // For each contact, dispatch the message to the appropriate message
                    // handler.
                    for (int i = 0; i < numInputs; ++i)
                    {
                        if (ti[i].dwFlags & TOUCHEVENTF_DOWN)
                        {
							CurrentTouchesCount++;
                            TouchHandler(ti[i]);
                        }
                        else if (ti[i].dwFlags & TOUCHEVENTF_MOVE)
                        {
                            TouchHandler(ti[i]);
                        }
                        else if (ti[i].dwFlags & TOUCHEVENTF_UP)
                        {
							CurrentTouchesCount--;
                            TouchHandler(ti[i]);
							if (CurrentTouchesCount == 0)
							{
								TouchEndHandler_Session();
							}
                        }
                    }
                }
                CloseTouchInputHandle((HTOUCHINPUT)lParam);
                delete [] ti;
            }
            break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case SESSION_BACK_BUTTON:
				{
					sessionCurrentPictureIndex = -1;
					ShowWindow(hWnd_Session, SW_HIDE);
					ShowWindow(hWnd_Gallery, SW_SHOW);
					UpdateWindow(hWnd_Gallery);
					break;
				}
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		//case WM_PAINT:
			//DefWindowProc(hWnd, message, wParam, lParam);
			//hDc_Session = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			//EndPaint(hWnd, &ps);
			//break;
		case WM_CLOSE:
			ShowWindow(hWnd_Session, 0);
			ShowWindow(hWnd_Gallery, 1);
			UpdateWindow(hWnd_Gallery);
			break;
		case WM_DESTROY:
			DestroyWindow(hWnd_Session);
			//PostQuitMessage(0);
			break;
	
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK WndProc_Comparsion(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rcClient;                 // client area rectangle 
	POINT ptClientUL;              // client upper left corner 
    POINT ptClientLR;              // client lower right corner 
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	//FLICK_DATA fd;
	DWORD dwThreadId;
	DWORD a = 0;

	switch (message)
	{
		case WM_MOUSEMOVE:
			if (wParam & MK_LBUTTON) 
            {
				POINTS pts = MAKEPOINTS(lParam); 
				
				TOUCHINPUT ti = {
					pts.x * 100,
					pts.y * 100,
					NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
				};

				TouchHandler(ti);
			}
			break;

		case WM_LBUTTONUP:
			{
				TouchEndHandler_Comparsion();
				break;
			}
        case WM_TOUCH:
            {
                // WM_TOUCH message can contain several messages from different contacts
                // packed together.
                // Message parameters need to be decoded:
                unsigned int numInputs = (unsigned int) wParam; // Number of actual per-contact messages
                TOUCHINPUT* ti = new TOUCHINPUT[numInputs]; // Allocate the storage for the parameters of the per-contact messages
                if (ti == NULL)
                {
                    break;
                }
                // Unpack message parameters into the array of TOUCHINPUT structures, each
                // representing a message for one single contact.
                if (GetTouchInputInfo((HTOUCHINPUT)lParam, numInputs, ti, sizeof(TOUCHINPUT)))
                {
                    // For each contact, dispatch the message to the appropriate message
                    // handler.
                    for (int i = 0; i < numInputs; ++i)
                    {
                        if (ti[i].dwFlags & TOUCHEVENTF_DOWN)
                        {
							CurrentTouchesCount++;
                            TouchHandler(ti[i]);
                        }
                        else if (ti[i].dwFlags & TOUCHEVENTF_MOVE)
                        {
                            TouchHandler(ti[i]);
                        }
                        else if (ti[i].dwFlags & TOUCHEVENTF_UP)
                        {
							CurrentTouchesCount--;
                            TouchHandler(ti[i]);
							if (CurrentTouchesCount == 0)
							{
								TouchEndHandler_Comparsion();
							}
                        }
                    }
                }
                CloseTouchInputHandle((HTOUCHINPUT)lParam);
                delete [] ti;
            }
            break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case COMPARSION_BACK_BUTTON:
				{
					comparsionCurreentPictureIndex = -1;
					ShowWindow(hWnd_Comparsion, SW_HIDE);
					ShowWindow(hWnd_Gallery, SW_SHOW);
					UpdateWindow(hWnd_Gallery);
					break;
				}
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_CLOSE:
			ShowWindow(hWnd_Comparsion, SW_HIDE);
			ShowWindow(hWnd_Gallery, SW_SHOWNORMAL);
			UpdateWindow(hWnd_Gallery);
			break;
		case WM_DESTROY:
			DestroyWindow(hWnd_Session);
			break;
	
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#pragma region Processing touches
/*=================================================================================================================================*/
int GetTouchesResult()
{
	int result = -1;
	if (CurrentTouches.size() > 1)
	{	
		POINT start = { 0, 0 }, end = { 0, 0 };
		// get middle index in of array touches
		int middleIndex = CurrentTouches.size() / 2;
		if(CurrentTouches.size() % 2 == 0)
			middleIndex -= 1;
		// get start point
		int counter = 0;
		for(int i = 0; i <= middleIndex; i++)
		{
			start.x += CurrentTouches[i].x;
			start.y += CurrentTouches[i].y;
			counter++;
		}
		start.x = start.x / counter;
		start.y = start.y / counter;
		// get end point
		counter = 0;
		for(unsigned int i = middleIndex + 1; i < CurrentTouches.size(); i++)
		{
			end.x += CurrentTouches[i].x;
			end.y += CurrentTouches[i].y;
			counter++;
		}
		end.x = end.x / counter;
		end.y = end.y / counter;
		// get angle
		double angle = atan2l(-(end.y - start.y), end.x - start.x) * 180 / 3.1415926535;
		// choose action
		if(angle > 45 && angle < 135)
			result = TOUCH_MOVEMENT_UP;
		else if(angle <= 45 && angle >= -45)
			result = TOUCH_MOVEMENT_RIGHT;
		else if(angle < -45 && angle > -135)
			result = TOUCH_MOVEMENT_DOWN;
		else 
			result = TOUCH_MOVEMENT_LEFT;
	}
	CurrentTouches.clear();
	return result;
}
/*=================================================================================================================================*/
void TouchHandler(TOUCHINPUT touch)
{
	CurrentTouches.push_back(touch);
}
/*=================================================================================================================================*/
void TouchEndHandler_Main()
{	
	int direction = GetTouchesResult();
	
	
	if (!showCountdownWarning)
	{
		//if countdown and taking pictures process IS NOT active while registered new touches - handle touches as usually
		switch(direction)
		{
			case TOUCH_MOVEMENT_UP:
			{
				//DestroyWindow(hWnd_Main);
				_beginthread(takeSnapshots, 0, NULL);
				break;
			}
			case TOUCH_MOVEMENT_DOWN:
				Cameras.EndShow();
				ShowWindow(hWnd_Main, 0);
				ShowWindow(hWnd_Gallery, 1);
				UpdateWindow(hWnd_Gallery);
				galleryWindowNeedsRefresh = true;
				break;
			case TOUCH_MOVEMENT_LEFT:
				Cameras.SetNextRightCamera();
				break;
			case TOUCH_MOVEMENT_RIGHT:
				Cameras.SetNextLeftCamera();
				break;
		}
	}
	else
	{
		//if countdown and taking pictures process IS active while registered new touches - cancel countdown and taking pictures
		showCountdownWarning = false;
	}
}

/*=================================================================================================================================*/
void TouchEndHandler_Session()
{
	int direction = GetTouchesResult();
	switch(direction)
	{
	case TOUCH_MOVEMENT_DOWN:
		{
			MessageBox(hWnd_Main, L"down", L"Error", MB_OK);

			//open the gallery window
			//ShowWindow(hWndGallery, 1);
			//galleryWindowNeedsRefresh = true;
			break;
		}
	case TOUCH_MOVEMENT_UP:
		{
			/*MessageBox(hWndMain, L"up (close application)", L"Error", MB_OK);
			PostQuitMessage(0);*/
			spinige.createSpinigeStorage(folders[sessionCurrentFolderIndex].name);
			break;
		}
	case TOUCH_MOVEMENT_LEFT:
		{
			sessionCurrentPictureIndex++;
			if (sessionCurrentPictureIndex == picturesInCurrentFolder.size())
				sessionCurrentPictureIndex = 0;

			sessionWindowNeedsRefresh = true;
			break;
		}
	case TOUCH_MOVEMENT_RIGHT:
		{
			sessionCurrentPictureIndex--;
			if (sessionCurrentPictureIndex == -1)
				sessionCurrentPictureIndex = picturesInCurrentFolder.size() - 1;

			sessionWindowNeedsRefresh = true;
			break;
		}
	case -1:
		//not enough points made to define a direction
		break;
	default:
		MessageBox(NULL, L"invalid direction", L"Error", MB_OK);
	}
}
/*=================================================================================================================================*/
void TouchEndHandler_Comparsion()
{
	int direction = GetTouchesResult();
	switch(direction)
	{
	case TOUCH_MOVEMENT_DOWN:
		{
			MessageBox(hWnd_Comparsion, L"down", L"Error", MB_OK);

			//open the gallery window
			//ShowWindow(hWndGallery, 1);
			//galleryWindowNeedsRefresh = true;
			break;
		}
	case TOUCH_MOVEMENT_UP:
		{
			MessageBox(hWnd_Comparsion, L"up", L"Error", MB_OK);
			/*PostQuitMessage(0);
			spinige.createSpinigeStorage(folders[sessionCurrentFolderIndex].name);*/
			break;
		}
	case TOUCH_MOVEMENT_LEFT:
		{
			//MessageBox(hWnd_Comparsion, L"left", L"Error", MB_OK);
			
			comparsionCurreentPictureIndex++;
			if (comparsionCurreentPictureIndex == picsToCompare[0].size())
				comparsionCurreentPictureIndex = 0;

			comparsionWindowNeedsRefresh = true;
			
			break;
		}
	case TOUCH_MOVEMENT_RIGHT:
		{
			//MessageBox(hWnd_Comparsion, L"right", L"Error", MB_OK);
			
			comparsionCurreentPictureIndex--;
			if (comparsionCurreentPictureIndex == -1)
				comparsionCurreentPictureIndex = picsToCompare[0].size() - 1;

			comparsionWindowNeedsRefresh = true;
			
			break;
		}
	case -1:
		//not enough points made to define a direction
		break;
	default:
		MessageBox(NULL, L"invalid direction", L"Error", MB_OK);
	}
}
/*=================================================================================================================================*/
void gallerySelectFolder(int folderIndex)
{
	//mark the folder as selected for comparsion
	folders[folderIndex].marked = true;
	//redraw selection
	skipFindFolders = true;
	galleryWindowNeedsRefresh = true;
}
/*=================================================================================================================================*/
void galleryUnselectFolder(int folderIndex)
{
	//unmark the folder as selected for comparsion
	folders[folderIndex].marked = false;
	//redraw selection
	skipFindFolders = true;
	galleryWindowNeedsRefresh = true;
}
#pragma endregion

/*=================================================================================================================================*/
int InitializeOpenGL(HWND _hWnd, int monitorWidth, int monitorHeight)
{
	HDC _hDc = GetDC(_hWnd);
	// установка параметров контекста воспроизведения OpenGL
	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize				= sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion			= 1;
	pfd.dwFlags				= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType			= PFD_TYPE_RGBA;
	pfd.cColorBits			= 32;
	pfd.cRedBits			= 0;
	pfd.cRedShift			= 0;
	pfd.cGreenBits			= 0;
	pfd.cGreenShift			= 0;
	pfd.cBlueBits			= 0;
	pfd.cBlueShift			= 0;
	pfd.cAlphaBits			= 0;
	pfd.cAlphaShift			= 0;
	pfd.cAccumBits			= 0;
	pfd.cAccumRedBits		= 0;
	pfd.cAccumGreenBits		= 0;
	pfd.cAccumBlueBits		= 0;
	pfd.cAccumAlphaBits		= 0;
	pfd.cDepthBits			= 32;
	pfd.cStencilBits		= 0;
	pfd.cAuxBuffers			= 0;
	pfd.iLayerType			= PFD_MAIN_PLANE;
	pfd.bReserved			= 0;
	pfd.dwLayerMask			= 0;
	pfd.dwVisibleMask		= 0;
	pfd.dwDamageMask		= 0;
	// попытка подобрать подходящий формат пикселя
	int PixelFormat = ChoosePixelFormat(_hDc, &pfd);
	if(PixelFormat == 0)
		return 0;
	// устанавливаем формат пикселя
	if(SetPixelFormat(_hDc, PixelFormat, &pfd) == FALSE)
		return 0;
	// создаем контекст воспроизведения OpenGL
	HGLRC hGLRC = wglCreateContext(_hDc);
	if(hGLRC == NULL)
		return 0;
	// делаем его текущим
	if(wglMakeCurrent(_hDc, hGLRC) == false)
		return 0;
	ReleaseDC(_hWnd, _hDc);
	
	/*
	// получаемразмеры окна
	const POINT ptZero = { 0, 0 };
	HMONITOR monitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo = { sizeof(monitorInfo) };
	int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
	*/

	//Устанавливаем область вывода для приложений OpenGL
	glViewport(0, 0, monitorWidth, monitorHeight);
	//Выбираем матрицу проекции
	glMatrixMode(GL_PROJECTION);
	//Сброс матрицы проекции
	glLoadIdentity();
	//Установка ортогональной проекции
	glOrtho(0, monitorWidth, 0, monitorHeight, -1.0, 1.0);
	//Выбираем матрицу вида модели
	glMatrixMode(GL_MODELVIEW);
	//Сброс матрицы вида модели
	glLoadIdentity();
	//Разрешаем плавное цветовое сглаживание
	glShadeModel(GL_SMOOTH);
	//Устанавливаем цвет отчистки экрана
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//Разрешаем отчистку буфера глубины
	glClearDepth(1.0f);
	return 1;
}
/*=================================================================================================================================*/
void drawMain(void * args)
{
	const POINT ptZero = { 0, 0 };
	HMONITOR monitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo = { sizeof(monitorInfo) };
	if (!GetMonitorInfo(monitor, &monitorInfo)) 
		return ;
	int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	if(InitializeOpenGL(hWnd_Main, monitorWidth, monitorHeight) < 1)
	{
		MessageBox(NULL, L"OpenGL initialization error", L"Main window", MB_OK);
		DestroyWindow(hWnd_Main);
		return;
	}

	glRasterPos2f(0, 0);
	char* rotatedBuffer = new char[PICTURE_WIDTH * PICTURE_HEIGHT * 3];
	char* finalBuffer = new char[PICTURE_WIDTH * PICTURE_HEIGHT * 3];
	
	while(!finishAllThreads)
	{
		if (!Cameras.IsShowVideo)
			Sleep(100);
		else
		{
			//original picture
			//glDrawPixels(PICTURE_WIDTH, PICTURE_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, Cameras.CameraBuffer);
			
			//customRotate(Cameras.CameraBuffer, rotatedBuffer);
			for (int row = 0; row < PICTURE_HEIGHT; row++)
			{
				for (int col = 0; col < PICTURE_WIDTH; col++)
				{
					rotatedBuffer[(col*PICTURE_HEIGHT + (PICTURE_HEIGHT - row))*3 + 0] = Cameras.CameraBuffer[(row*PICTURE_WIDTH + col)*3 + 0];
					rotatedBuffer[(col*PICTURE_HEIGHT + (PICTURE_HEIGHT - row))*3 + 1] = Cameras.CameraBuffer[(row*PICTURE_WIDTH + col)*3 + 1];
					rotatedBuffer[(col*PICTURE_HEIGHT + (PICTURE_HEIGHT - row))*3 + 2] = Cameras.CameraBuffer[(row*PICTURE_WIDTH + col)*3 + 2];
				}
			}

			//customFlip(rotatedBuffer, finalBuffer);
			//for vertical picture (w = PICTURE_HEIGHT, h = PICTURE_WIDTH)
			for (int row = 0; row < PICTURE_WIDTH; row++)
			{
				memcpy(&finalBuffer[(PICTURE_WIDTH - row - 1) * PICTURE_HEIGHT * 3], &rotatedBuffer[row * PICTURE_HEIGHT * 3], PICTURE_HEIGHT * 3);
			}
			
			glDrawPixels(PICTURE_HEIGHT, PICTURE_WIDTH, GL_RGB, GL_UNSIGNED_BYTE, finalBuffer);
			
			if (showCountdownWarning)
			{
				if (snapshotWarningStatus > 0)
				{
					//draw countdown
					char countDown[2] = "";
					itoa(snapshotWarningStatus, countDown, 10);
					openGlDrawText(90, 70, countDown, 4);
				}
				else if (snapshotWarningStatus == 0)
				{
					openGlDrawText(15, 230, "PLEASE WAIT", 0.5);
				}
				else if (snapshotWarningStatus < 0)
				{
					openGlDrawText(80, 230, "DONE", 0.5);
				}
			}
			SwapBuffers(hDc_Main);

			Sleep(33);
		}
	}
}
/*=================================================================================================================================*/
void drawGallery(void * args)
{
	//get monitor size
	const POINT ptZero = { 0, 0 };
	HMONITOR hmon = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = { sizeof(mi) };
	if (!GetMonitorInfo(hmon, &mi)) 
		return ;
	int monitorWidth = mi.rcMonitor.right - mi.rcMonitor.left;
	int monitorHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;

	if(InitializeOpenGL(hWnd_Gallery, monitorWidth, monitorHeight-(BACK_BUTTON_HEIGHT+5)) < 1)
	{
		MessageBox(NULL, L"OpenGL initialization Error", L"Gallery window", MB_OK);
		return ;
	}

	while(!finishAllThreads)
	{
		if (!galleryWindowNeedsRefresh)
			Sleep(50);
		else
		{
			//clear the screen
			openGlDrawRect(0,0, monitorWidth, (monitorHeight-(BACK_BUTTON_HEIGHT+5)), 0x000000ff);

			if (skipFindFolders == false)
				findFoldersWithPictures();

			const int columnsNumber = monitorWidth/(folderPlaceholderWidth + hGap);
			const int rowsNumber = (monitorHeight-(BACK_BUTTON_HEIGHT+5))/(folderPlaceholderHeight+vGap);
			int col = 0, row = 0;

			for (int i = 0; i < folders.size(); i++)
			{
				if (row < rowsNumber)
				{
					/*
					//compose a path to 1.jpg
					wstring pathTo1jpg = folders[i].name;
					wstring s2 (L"\\1.jpg");
					pathTo1jpg = pathTo1jpg+s2;
					wchar_t* wchart_pathTo1jpg = const_cast<wchar_t*>(pathTo1jpg.c_str()); //convert wstring to char*
					char* asciiPathTo1jpg = new char[wcslen(wchart_pathTo1jpg) + 1];
					wcstombs(asciiPathTo1jpg, wchart_pathTo1jpg, wcslen(wchart_pathTo1jpg) + 1 );

					//read from HDD
					IplImage* imgOriginal = cvLoadImage(asciiPathTo1jpg, 1);

					//convert BGR -> RGB
					char symb;
					for (int j=0; j<imgOriginal->width * imgOriginal->height * 3; j+=3)
					{
						symb = imgOriginal->imageData[j+0];
						imgOriginal->imageData[j+0] = imgOriginal->imageData[j+2];
						imgOriginal->imageData[j+2] = symb;
					}

					//rotate
					IplImage* imgRotated = rotateImage(imgOriginal);

					//scale to fit a placeholder
					int scaledImgWidth = 0;
					int scaledImgHeight = 0;
					calculateScaledImageSize(folderPlaceholderWidth, folderPlaceholderHeight, imgRotated->width, imgRotated->height, &scaledImgWidth, &scaledImgHeight);
					IplImage* imgScaled = cvCreateImage(cvSize(scaledImgWidth,scaledImgHeight), imgRotated->depth, imgRotated->nChannels);
					cvResize(imgRotated, imgScaled, CV_INTER_LINEAR);

					//flip
					cvFlip(imgScaled);
					*/

					IplImage *imgScaled = folders[i].thumbnail;

					//calculate coordinates
					int curPictureX = 10 + hGap*col + folderPlaceholderWidth*col;
					int curPictureY = monitorHeight - (BACK_BUTTON_HEIGHT+5) - vGap*row - folderPlaceholderHeight*(row+1);
					int leftPadding = (folderPlaceholderWidth - imgScaled->width) / 2;
					glRasterPos2f(curPictureX + leftPadding, curPictureY);

					//draw selection if marked
					if (folders[i].marked == true)
						openGlDrawRect(curPictureX, curPictureY, curPictureX + folderPlaceholderWidth, curPictureY + folderPlaceholderHeight, 0x0000ff00);
					
					//draw a picture
					glDrawPixels(imgScaled->width, imgScaled->height, GL_RGB, GL_UNSIGNED_BYTE, imgScaled->imageData);

					//save the current picture rect (in coordinates of a window) to be able detect clicks on it
					RECT r = {
						curPictureX,
						(BACK_BUTTON_HEIGHT+5) + vGap*row + imgScaled->height*row,
						//curPictureX + imgScaled->width,
						curPictureX + folderPlaceholderWidth,
						(BACK_BUTTON_HEIGHT+5) + vGap*row + imgScaled->height*(row + 1)
					};
					folders[i].winRect = r;

					//calculate a new column or/and a new row for the next picture
					col++;
					if (col>columnsNumber-1)
					{
						col = 0;
						row++;
					}
				}
			}
			
			SwapBuffers(hDc_Gallery);
			InvalidateRect(hWnd_Gallery, NULL, false);
			galleryWindowNeedsRefresh = false;
			skipFindFolders = false;
		} 
	}
}
/*=================================================================================================================================*/
void drawSession(void * args)
{
	//get monitor size
	const POINT ptZero = { 0, 0 };
	HMONITOR hmon = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = { sizeof(mi) };
	if (!GetMonitorInfo(hmon, &mi)) 
		return ;
	int monitorWidth = mi.rcMonitor.right - mi.rcMonitor.left;
	int monitorHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;

	if(InitializeOpenGL(hWnd_Session, monitorWidth, monitorHeight-(BACK_BUTTON_HEIGHT+5)) < 1)
	{
		MessageBox(NULL, L"OpenGL initialization Error", L"Session window", MB_OK);
		return ;
	}

	glRasterPos2f(0, 0);

	while(!finishAllThreads)
	{
		if (!sessionWindowNeedsRefresh)
			Sleep(50);
		else
		{
			if (!skipFindPicturesInFolder)
			{
				findPicturesInCurrentFolder();
				skipFindPicturesInFolder = true;
			}

			/*
			//compose a path to the current picture
			wstring pathTojpg = folders[sessionCurrentFolderIndex].name + L"\\" + picturesInCurrentFolder[sessionCurrentPictureIndex];
			wchar_t* wchart_pathTojpg = const_cast<wchar_t*>(pathTojpg.c_str());//convert wstring to char*
			char* asciiPathTojpg = new char[wcslen(wchart_pathTojpg) + 1];
			wcstombs(asciiPathTojpg, wchart_pathTojpg, wcslen(wchart_pathTojpg) + 1);
			
			//load from HDD
			IplImage* imgOriginal = cvLoadImage(asciiPathTojpg, 1);
			
			//convert BGR -> RGB
			char symb;
			for (int j=0; j<imgOriginal->width * imgOriginal->height * 3; j+=3)
			{
				symb = imgOriginal->imageData[j+0];
				imgOriginal->imageData[j+0] = imgOriginal->imageData[j+2];
				imgOriginal->imageData[j+2] = symb;
			}

			//rotate
			IplImage* imgRotated = rotateImage(imgOriginal);

			//scale to fit a screen
			int newWidth = 0, newHeight = 0;
			calculateScaledImageSize(monitorWidth, monitorHeight-(BACK_BUTTON_HEIGHT+5), imgRotated->width, imgRotated->height, &newWidth, &newHeight);
			IplImage* imgScaled = cvCreateImage(cvSize(newWidth,newHeight), imgRotated->depth, imgRotated->nChannels);
			cvResize(imgRotated, imgScaled, CV_INTER_LINEAR);
			
			//flip
			cvFlip(imgScaled);
			*/

			IplImage *imgScaled = picturesInCurrentFolder[sessionCurrentPictureIndex].image;

			//draw
			glDrawPixels(imgScaled->width, imgScaled->height, GL_RGB, GL_UNSIGNED_BYTE, imgScaled->imageData);

			SwapBuffers(hDc_Session);

			//free the used memory
			/*cvReleaseImage(&imgOriginal);
			cvReleaseImage(&imgRotated);
			cvReleaseImage(&imgScaled);
			delete[] asciiPathTojpg;*/
			//Sleep(30);
			
			//InvalidateRect(hWnd_Session, NULL, false);
			sessionWindowNeedsRefresh = false;
		}
	}
}
/*=================================================================================================================================*/
void drawComparsion(void * args)
{
	//get monitor size
	const POINT ptZero = { 0, 0 };
	HMONITOR hmon = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = { sizeof(mi) };
	if (!GetMonitorInfo(hmon, &mi)) 
		return ;
	int monitorWidth = mi.rcMonitor.right - mi.rcMonitor.left;
	int monitorHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;

	if(InitializeOpenGL(hWnd_Comparsion, monitorWidth, monitorHeight-(BACK_BUTTON_HEIGHT+5)) < 1)
	{
		MessageBox(NULL, L"OpenGL initialization Error", L"Session window", MB_OK);
		return ;
	}

	int markedFolderCounter = -1;
	const int columnsNumber = 2;
	const int rowsNumber = 2;
	

	while(!finishAllThreads)
	{
		if (!comparsionWindowNeedsRefresh)
			Sleep(50);
		else
		{
			//clear the screen
			openGlDrawRect(0,0, monitorWidth, (monitorHeight-(BACK_BUTTON_HEIGHT+5)), 0x000000ff);

			if (!skipFindPicturesInSelectedFolder)
			{
				markedFolderCounter = findPicturesInSelectedFolders();
				skipFindPicturesInSelectedFolder = true;
			}

			int col = 0, row = 0;
			for (int i = 0; i < markedFolderCounter; i++)
			{
				IplImage *imgScaled = picsToCompare[i][comparsionCurreentPictureIndex].image;

				//calculate coordinates
				int curPictureX = imgScaled->width*col;
				int curPictureY = monitorHeight - (BACK_BUTTON_HEIGHT+5) - imgScaled->height*(row+1);

				glRasterPos2f(curPictureX, curPictureY);
				//draw
				glDrawPixels(imgScaled->width, imgScaled->height, GL_RGB, GL_UNSIGNED_BYTE, imgScaled->imageData);

				col++;
				if (col>columnsNumber-1)
				{
					col = 0;
					row++;
				}
			}
			
			SwapBuffers(hDc_Comparsion);

			//InvalidateRect(hWnd_Session, NULL, false);
			comparsionWindowNeedsRefresh = false;
		}
	}
}
/*=================================================================================================================================*/
void findFoldersWithPictures()
{
	//hide "compare" button
	ShowWindow(hWndGalleryCompareButton, SW_HIDE);
	
	//free all memory under IplImages in "folders"
	for (int i = 0; i < folders.size(); i++)
		cvReleaseImage(&folders[i].thumbnail);

	folders.clear();

	WIN32_FIND_DATA fd;
	//find all files in the application directory 
    HANDLE hFind=::FindFirstFile(convertCharArrayToLPCWSTR("*.*"), &fd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        do{
			//check if it's a directory
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wstring s1(fd.cFileName);
				wstring s2(L"\\1.jpg");
				s1 = s1 + s2;
				WIN32_FIND_DATA fd2;
				//find 1.jpg in the current directory
				HANDLE hFind2=::FindFirstFile(s1.c_str(), &fd2);
				if(hFind2 != INVALID_HANDLE_VALUE)
				{
					wstring s3(fd.cFileName);
					FolderProps fp = {s3, {0,0,0,0}, false, NULL};

					//convert wstring to char*
					wchar_t* wchart_pathTo1jpg = const_cast<wchar_t*>(s1.c_str()); //convert wstring to char*
					char* asciiPathTo1jpg = new char[wcslen(wchart_pathTo1jpg) + 1];
					wcstombs(asciiPathTo1jpg, wchart_pathTo1jpg, wcslen(wchart_pathTo1jpg) + 1 );

					//load 1.jpg
					IplImage* imgOriginal = cvLoadImage(asciiPathTo1jpg, 1);

					//convert BGR -> RGB
					char symb;
					for (int j=0; j<imgOriginal->width * imgOriginal->height * 3; j+=3)
					{
						symb = imgOriginal->imageData[j+0];
						imgOriginal->imageData[j+0] = imgOriginal->imageData[j+2];
						imgOriginal->imageData[j+2] = symb;
					}

					//rotate
					IplImage* imgRotated = rotateImage(imgOriginal);

					//scale to fit a placeholder
					int scaledImgWidth = 0;
					int scaledImgHeight = 0;
					calculateScaledImageSize(folderPlaceholderWidth, folderPlaceholderHeight, imgRotated->width, imgRotated->height, &scaledImgWidth, &scaledImgHeight);
					IplImage* imgScaled = cvCreateImage(cvSize(scaledImgWidth,scaledImgHeight), imgRotated->depth, imgRotated->nChannels);
					cvResize(imgRotated, imgScaled, CV_INTER_LINEAR);

					//flip
					cvFlip(imgScaled);

					//free used memory
					cvReleaseImage(&imgOriginal);
					cvReleaseImage(&imgRotated);
					delete[] asciiPathTo1jpg;

					fp.thumbnail = imgScaled;
					folders.push_back(fp);
				}
			}
        }while(::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
}
/*=================================================================================================================================*/
void findPicturesInCurrentFolder()
{
	//get monitor size
	const POINT ptZero = { 0, 0 };
	HMONITOR hmon = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = { sizeof(mi) };
	if (!GetMonitorInfo(hmon, &mi)) 
		return ;
	int monitorWidth = mi.rcMonitor.right - mi.rcMonitor.left;
	int monitorHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;

	//free all memory under IplImages in "picturesInCurrentFolder"
	for (int i = 0; i < picturesInCurrentFolder.size(); i++)
		cvReleaseImage(&picturesInCurrentFolder[i].image);

	picturesInCurrentFolder.clear();
	loadPicturesFromFolderIntoVector(sessionCurrentFolderIndex, &picturesInCurrentFolder, monitorWidth, monitorHeight-(BACK_BUTTON_HEIGHT+5));
}
/*=================================================================================================================================*/
void loadPicturesFromFolderIntoVector(int folderIndex, vector<PictureProps> *pics, int placeholderW, int placeholderH)
{
	WIN32_FIND_DATA fd;
	wstring searchPath= folders[folderIndex].name + L"\\*.jpg";
	
	HANDLE hFind=::FindFirstFile(searchPath.c_str(), &fd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        do{
			wstring curFileName(fd.cFileName);
			PictureProps pp = {curFileName, NULL};

			//compose a path to the current picture
			wstring pathTojpg = folders[folderIndex].name + L"\\" + curFileName;
			wchar_t* wchart_pathTojpg = const_cast<wchar_t*>(pathTojpg.c_str());//convert wstring to char*
			char* asciiPathTojpg = new char[wcslen(wchart_pathTojpg) + 1];
			wcstombs(asciiPathTojpg, wchart_pathTojpg, wcslen(wchart_pathTojpg) + 1);
			
			//load from HDD
			IplImage* imgOriginal = cvLoadImage(asciiPathTojpg, 1);
			
			//convert BGR -> RGB
			char symb;
			for (int j=0; j<imgOriginal->width * imgOriginal->height * 3; j+=3)
			{
				symb = imgOriginal->imageData[j+0];
				imgOriginal->imageData[j+0] = imgOriginal->imageData[j+2];
				imgOriginal->imageData[j+2] = symb;
			}

			//rotate
			IplImage* imgRotated = rotateImage(imgOriginal);
			
			if (placeholderW != 0 && placeholderH != 0) // if "= 0" - no need to scale
			{
				//scale to fit a screen
				int newWidth = 0, newHeight = 0;
				calculateScaledImageSize(placeholderW, placeholderH, imgRotated->width, imgRotated->height, &newWidth, &newHeight);
				IplImage* imgScaled = cvCreateImage(cvSize(newWidth,newHeight), imgRotated->depth, imgRotated->nChannels);
				cvResize(imgRotated, imgScaled, CV_INTER_LINEAR);
			
				//flip
				cvFlip(imgScaled);

				cvReleaseImage(&imgRotated);

				pp.image = imgScaled;
			}
			else
			{
				//flip
				cvFlip(imgRotated);

				pp.image = imgRotated;
			}

			cvReleaseImage(&imgOriginal);
			delete[] asciiPathTojpg;
			pics->push_back(pp);
        }while(::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
}
/*=================================================================================================================================*/
int findPicturesInSelectedFolders()
{
	//get monitor size
	const POINT ptZero = { 0, 0 };
	HMONITOR hmon = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = { sizeof(mi) };
	if (!GetMonitorInfo(hmon, &mi)) 
		return -1;
	int monitorWidth = mi.rcMonitor.right - mi.rcMonitor.left;
	int monitorHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;

	//free all memory under IplImages in "picsToCompare"
	for (int i = 0; i < 4; i++)
	{
		for (int j = picsToCompare[i].size() - 1; j >= 0 ; j--)
		{
			cvReleaseImage(&(picsToCompare[i][j].image));
			picsToCompare[i].erase(picsToCompare[i].begin() + j);
		}
	}
	
	int markedFolderCounter = 0;
	for (int i = 0; i < folders.size(); i++)
		if (folders[i].marked == true)
		{
			loadPicturesFromFolderIntoVector(i, &picsToCompare[markedFolderCounter], monitorWidth / 2, (monitorHeight-(BACK_BUTTON_HEIGHT+5)) / 2);
			markedFolderCounter++;
		}

	return markedFolderCounter;
}
/*=================================================================================================================================*/
wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString=new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}
/*=================================================================================================================================*/
void calculateScaledImageSize(int placeholderW, int placeholderH, int originalW, int originalH, int* newW, int* newH)
{
	float placeholderAspectRatio = (float)placeholderW / (float)placeholderH;
	float originalAspectRatio = (float)originalW / (float)originalH;

	if (placeholderAspectRatio >= originalAspectRatio)
	{
		*newH = placeholderH;
		*newW = originalAspectRatio * placeholderH;
	}
	else
	{
		*newW = placeholderW;
		*newH = placeholderW / originalAspectRatio;
	}
}
/*=================================================================================================================================*/
void takeSnapshots(void * args)
{
	showCountdownWarning = true;

	for (int i = 5; i >= 0; i--)
	{
		if (showCountdownWarning)
		{
			snapshotWarningStatus = i;
			Sleep(1000);
		}
	}

	if (showCountdownWarning)
	{
		Cameras.EndShow();
		//Sleep(1000);
		Cameras.savePicturesFromActiveCamerasToDisc();
		Cameras.BeginShow();

		snapshotWarningStatus = -1;
		Sleep(2000);
	}

	showCountdownWarning = false;
}
/*=================================================================================================================================*/
void openGlDrawText(GLfloat x, GLfloat y, char* text, float size)
{
    glPushMatrix();
    glTranslatef(x, y, 0);		//position
    glScalef(size,size,size);	//size
	glColor3f(1.0f, 1.0f, 1.0f);//color
	glLineWidth(size*3);		//thickness
	
    for( char* p = text; *p; p++)
    {
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *p);
    }
    glPopMatrix();
}
/*=================================================================================================================================*/
void openGlDrawRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, unsigned int rgba)
{
    glPushMatrix();
    
	glColor4f(
		((rgba>>24)&0xff)/255.0f,
		((rgba>>16)&0xff)/255.0f, 
		((rgba>>8)&0xff)/255.0f,
		(rgba&0xff)/255.0f);
	glRectf(x1,y1,x2,y2);

    glPopMatrix();
}
/*=================================================================================================================================*/
// Rotate the image clockwise (or counter-clockwise if negative).
// Remember to free the returned image.
IplImage *rotateImage(IplImage *src)
{
	float angleDegrees = 90;
	// Create a map_matrix, where the left 2x2 matrix
	// is the transform and the right 2x1 is the dimensions.
	float m[6];
	CvMat M = cvMat(2, 3, CV_32F, m);
	int w = src->width;
	int h = src->height;
	float angleRadians = angleDegrees * ((float)CV_PI / 180.0f);
	m[0] = (float)( cos(angleRadians) );
	m[1] = (float)( sin(angleRadians) );
	m[3] = -m[1];
	m[4] = m[0];
	m[2] = w*0.5f;  
	m[5] = h*0.5f;  
	
	// Make a spare image for the result
	CvSize sizeRotated;
	sizeRotated.width = cvRound(h);
	sizeRotated.height = cvRound(w);
	
	// Rotate
	IplImage *imageRotated = cvCreateImage( sizeRotated, src->depth, src->nChannels );

	// Transform the image
	cvGetQuadrangleSubPix( src, imageRotated, &M);

	return imageRotated;
}

