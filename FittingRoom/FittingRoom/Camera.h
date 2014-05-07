#pragma once
#include "process.h"
#include "MyConstants.h"
#include "getFotosFromCamsAlt.h"
#pragma comment(lib, "GetFotosFromCamsAlt.lib")
#include "opencv\cv.h"
#include "opencv\highgui.h"
#pragma comment(lib, "opencv_core241d.lib")
#pragma comment(lib, "opencv_highgui241d.lib")
#pragma comment(lib, "opencv_imgproc241d.lib")

class Camera
{
public:
	Camera(void);
	~Camera(void);

	int index;
	int systemIndex;
	bool isActive;
	bool flagStop;
	bool flagInit;
	char *buffer;
	HANDLE cameraStopped;

	void BeginShow();
	static void BeginShowThreadEntry(void * args);
	void BeginShowThreadBody();
	void EndShow();
	
};

