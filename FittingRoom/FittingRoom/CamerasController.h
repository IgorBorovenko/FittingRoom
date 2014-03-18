#pragma once
#include "iostream"
#include "fstream"
#include "process.h"
#include <ctime>
#include "MyConstants.h"
#include "GetFotosFromCams.h"
#pragma comment(lib, "GetFotosFromCams.lib")
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
		// methods
		bool ReadConfig();
		// get video from current camera
		static void BeginShowThreadEntry(void * args);
		void BeginShowThreadBody();
		wchar_t *CamerasController::convertCharArrayToLPCWSTR(const char* charArray);

	public:
		// variables
		char CameraBuffer[PICTURE_SIZE];
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
};