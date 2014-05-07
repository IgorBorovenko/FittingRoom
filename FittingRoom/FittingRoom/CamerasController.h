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

#include "Camera.h"

class CamerasController
{
	private:  
		// variables

		// methods
		bool ReadConfig();
		wchar_t *convertCharArrayToLPCWSTR(const char* charArray);
		int getNextLeftCameraIndex(int cameraIndex);
		int getNextRightCameraIndex(int cameraIndex);
	public:
		// variables
		Camera *LeftCamera;
		Camera *CurrentCamera;
		Camera *RightCamera;
		Camera Cameras[CAMERAS_COUNT];
		bool allCamerasWorking;

		// methods
		CamerasController(void);
		~CamerasController(void);
		// Initializing cameras. Error codes returning:
		//  0 - all right
		// -1 - no active cameras
		// -2 - error occured during read config
		int InitializeCameras();
		// set next left camera
		void SetNextLeftCamera();
		// set next right camera
		void SetNextRightCamera();
		void savePicturesFromActiveCamerasToDisc();
		void StartAllCameras();
		void StopAllCameras();
};