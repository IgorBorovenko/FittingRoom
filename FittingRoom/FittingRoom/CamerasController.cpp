#include "stdafx.h"
#include "CamerasController.h"
#include "FittingRoom.h"

wchar_t *CamerasController::convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString=new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}
/*=================================================================================================================================*/
CamerasController::CamerasController(void)
{
	fps = 20;
	CameraBuffer = new char[PICTURE_SIZE];
	IsShowVideo = false;
	CurrentCamera = NULL;
	memset(CameraBuffer, 255, PICTURE_SIZE);
	cameraStopped = CreateEvent(NULL, true, false, L"cameraStopped");
}
/*=================================================================================================================================*/
CamerasController::~CamerasController(void)
{
	delete[] CameraBuffer;
}
/*=================================================================================================================================*/
bool CamerasController::ReadConfig()
{
	//initialize 'Camera' objects with start values
	for (int i = 0; i < CAMERAS_COUNT; i++)
	{
		Cameras[i].Index = 666;
		Cameras[i].SystemIndex = 666;
	}

	ifstream myStream(CAMERAS_CONFIG, ios::in);
	if(!myStream.fail())
	{
		int cameraIndex = 0, counter = 0;
		while(!myStream.eof() && counter < CAMERAS_COUNT)
		{
			myStream>>cameraIndex;
			Cameras[cameraIndex].Index = cameraIndex;
			Cameras[cameraIndex].SystemIndex = counter++;
		}
		return true;
	}
	return false;
}
/*=================================================================================================================================*/
int CamerasController::InitializeCameras()
{
	int errorCode = -1;
	if(!ReadConfig())
		errorCode = -2;
	else
	{
		bool temp = false;
		for(int i = 0; i < CAMERAS_COUNT; i++)
		{
			//Cameras[i].IsActive = getFotosFromOneCam(NULL, PICTURE_WIDTH, PICTURE_HEIGHT, Cameras[i].SystemIndex, false, 2, &temp, &temp, false) == 1;
			Cameras[i].IsActive = getFotosFromOneCam(NULL, PICTURE_WIDTH, PICTURE_HEIGHT, Cameras[i].SystemIndex, &temp, &temp, false, true, fps) == 1;
			if(Cameras[i].IsActive && CurrentCamera == NULL)
			{
				CurrentCamera = &Cameras[i];
				errorCode = 0;
			}
		}
	}
	return errorCode;
}
/*=================================================================================================================================*/
void CamerasController::BeginShow()
{
	IsShowVideo = true;
	_beginthread(BeginShowThreadEntry, 0, this);
	/*_beginthread(BeginShowThreadEntry1, 0, this);
	_beginthread(BeginShowThreadEntry2, 0, this);
	_beginthread(BeginShowThreadEntry3, 0, this);
	_beginthread(BeginShowThreadEntry4, 0, this);*/
}
/*=================================================================================================================================*/
void CamerasController::BeginShowThreadEntry(void * args)
{
	((CamerasController *) args)->BeginShowThreadBody();
}
void CamerasController::BeginShowThreadEntry1(void * args)
{
	((CamerasController *) args)->BeginShowThreadBody1();
}
void CamerasController::BeginShowThreadEntry2(void * args)
{
	((CamerasController *) args)->BeginShowThreadBody2();
}
void CamerasController::BeginShowThreadEntry3(void * args)
{
	((CamerasController *) args)->BeginShowThreadBody3();
}
void CamerasController::BeginShowThreadEntry4(void * args)
{
	((CamerasController *) args)->BeginShowThreadBody4();
}
/*=================================================================================================================================*/
void CamerasController::BeginShowThreadBody()
{
	bool flagInit = false;
	getFotosFromOneCam(CameraBuffer, PICTURE_WIDTH, PICTURE_HEIGHT, CurrentCamera->SystemIndex, &flagInit, &IsShowVideo, true, false, fps);
	SetEvent(cameraStopped);
}

void CamerasController::BeginShowThreadBody1()
{
	bool flagInit = true;
	getFotosFromOneCam(CameraBuffer, PICTURE_WIDTH, PICTURE_HEIGHT, 1, &flagInit, &flagInit, true, false, fps);
	//getFotosFromOneCam(CameraBuffer, PICTURE_WIDTH, PICTURE_HEIGHT, 1, &flagInit, &flagInit, true, false);
}
void CamerasController::BeginShowThreadBody2()
{
	bool flagInit = true;
	getFotosFromOneCam(CameraBuffer, PICTURE_WIDTH, PICTURE_HEIGHT, 2, &flagInit, &flagInit, true, false, fps);
	//getFotosFromOneCam(CameraBuffer, PICTURE_WIDTH, PICTURE_HEIGHT, 2, &flagInit, &flagInit, true, false);
}
void CamerasController::BeginShowThreadBody3()
{
	bool flagInit = true;
	//getFotosFromOneCam(CameraBuffer, PICTURE_WIDTH, PICTURE_HEIGHT, 3, &flagInit, &flagInit, true, false);
}
void CamerasController::BeginShowThreadBody4()
{
	bool flagInit = true;
	//getFotosFromOneCam(CameraBuffer, PICTURE_WIDTH, PICTURE_HEIGHT, 4, &flagInit, &flagInit, true, false);
}
/*=================================================================================================================================*/
void CamerasController::EndShow()
{
	ResetEvent(cameraStopped);
	IsShowVideo = false;
	WaitForSingleObject(cameraStopped, INFINITE);
}
/*=================================================================================================================================*/
void CamerasController::SetNextLeftCamera()
{
	int nextLeftIndex = -1;
	for(int i = CurrentCamera->Index - 1; i >=  0; i--)
	{
		if(Cameras[i].IsActive)
		{
			nextLeftIndex = i;
			break;
		}
	}
	if(nextLeftIndex == -1)
	{
		for(int i = CAMERAS_COUNT - 1; i >  CurrentCamera->Index; i--)
		{
			if(Cameras[i].IsActive)
			{
				nextLeftIndex = i;
				break;
			}
		}
	}
	if(nextLeftIndex != -1)
	{
		CurrentCamera = &Cameras[nextLeftIndex];
		EndShow();

		//while(!flagCameraStopped)
		//	Sleep(20);

		BeginShow();
	}
}
/*=================================================================================================================================*/
void CamerasController::SetNextRightCamera()
{
	int nextRightIndex = -1;
	for(int i = CurrentCamera->Index + 1; i < CAMERAS_COUNT; i++)
	{
		if(Cameras[i].IsActive)
		{
			nextRightIndex = i;
			break;
		}
	}
	if(nextRightIndex == -1)
	{
		for(int i = 0; i < CurrentCamera->Index; i++)
		{
			if(Cameras[i].IsActive)
			{
				nextRightIndex = i;
				break;
			}
		}
	}
	if(nextRightIndex != -1)
	{
		CurrentCamera = &Cameras[nextRightIndex];
		EndShow();
		
		/*while(!flagCameraStopped)
			Sleep(20);*/

		BeginShow();
	}
}
/*=================================================================================================================================*/
void CamerasController::savePicturesFromActiveCamerasToDisc()
{
	int fileCounter = 1;

	char *curCameraBuffer = new char[PICTURE_SIZE];
	bool flagInit = false;
	bool flagStop = true;
	bool directoryIsCreated = false;
	
	int p[3];
	p[0] = CV_IMWRITE_JPEG_QUALITY;
    p[1] = 100;//jpeg compressor quality 
    p[2] = 0;
	
	const char extension [5] = ".jpg";

	//form a name for the folder to which we're going to put files with pictures
	char foldername[80];
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(foldername,80,"%Y-%m-%d_%H-%M-%S",timeinfo);

	for(int i=0; i < CAMERAS_COUNT; i++)
	{
		//memset(curCameraBuffer, 255, PICTURE_SIZE);

		if (Cameras[i].IsActive)
		{
			int result = getFotosFromOneCam(curCameraBuffer, PICTURE_WIDTH, PICTURE_HEIGHT, Cameras[i].SystemIndex, &flagInit, &flagStop, true, true, fps);
			if (result == 1)
			{
				//success

				//create a directory for pictures after a camera has been successfully read
				if(!directoryIsCreated)
				{
					CreateDirectory(convertCharArrayToLPCWSTR(foldername), NULL);
					directoryIsCreated = true;
				}
				
				//convert BGR -> RGB
				char symb;
				for (int j=0; j<PICTURE_SIZE; j+=3)
				{
					symb = curCameraBuffer[j+0];
					curCameraBuffer[j+0] = curCameraBuffer[j+2];
					curCameraBuffer[j+2] = symb;
				}

				//form a name for the file to which we're going to save a picture
				char filename[10];
				itoa(fileCounter, filename, 10);
				strcat(filename, extension);

				//form a full path to the file from it's name and the folder name
				char fullPath[100];
				strcpy(fullPath, foldername);
				strcat(fullPath, "\\");
				strcat(fullPath, filename);
				
				//save a picture as a JPEG
				IplImage *img = cvCreateImage(cvSize(PICTURE_WIDTH, PICTURE_HEIGHT), 8, 3);
				img->imageData = curCameraBuffer;
				cvFlip(img,img,0);
				int res = cvSaveImage(fullPath, img, p);
				if (res == 0)
				{
					//handle an error
					LPCWSTR errorDetails;
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorDetails, 0, NULL);
					MessageBox(NULL, errorDetails, L"error", MB_OK);
				}
				cvReleaseImage(&img);
				fileCounter++;
			}
		}
	}
	delete[] curCameraBuffer;
}