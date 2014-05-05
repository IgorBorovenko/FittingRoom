#pragma once
#include "iostream"
#include "fstream"
#include "process.h"
#include <ctime>
#include "MyConstants.h"
//#include "GetFotosFromCams.h"
//#pragma comment(lib, "GetFotosFromCams.lib")
#include "getFotosFromCamsAlt.h"
#pragma comment(lib, "GetFotosFromCamsAlt.lib")
#include "opencv\cv.h"
#include "opencv\highgui.h"
#pragma comment(lib, "opencv_core241d.lib")
#pragma comment(lib, "opencv_highgui241d.lib")
#pragma comment(lib, "opencv_imgproc241d.lib")

using namespace std;

class CamerasController
{
	private:  
		struct Camera
		{
			int Index;
			int SystemIndex;
			bool IsActive;
		};
		// variables
		
		Camera * CurrentCamera;
		Camera Cameras[CAMERAS_COUNT];
		int fps;
		HANDLE cameraStopped;
		// methods
		bool ReadConfig();
		// get video from current camera
		static void BeginShowThreadEntry(void * args);
		static void BeginShowThreadEntry1(void * args);
		static void BeginShowThreadEntry2(void * args);
		static void BeginShowThreadEntry3(void * args);
		static void BeginShowThreadEntry4(void * args);

		void BeginShowThreadBody();
		void BeginShowThreadBody1();
		void BeginShowThreadBody2();
		void BeginShowThreadBody3();
		void BeginShowThreadBody4();
		wchar_t *CamerasController::convertCharArrayToLPCWSTR(const char* charArray);
		void changeFlagInit();
	public:
		// variables
		char *CameraBuffer;
		bool IsShowVideo;
		
		// methods
		CamerasController(void);
		~CamerasController(void);
		// Initializing cameras. Error codes returning:
		//  0 - all right
		// -1 - no active cameras
		// -2 - error occured during read config
		int InitializeCameras();
		// Begin show video from current camera
		void BeginShow();
		// End show video
		void EndShow();
		// set next left camera
		void SetNextLeftCamera();
		// set next right camera
		void SetNextRightCamera();
		void CamerasController::savePicturesFromActiveCamerasToDisc();
		static void smFnc(void * args);
};