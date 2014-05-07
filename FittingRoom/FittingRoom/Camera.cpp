#include "StdAfx.h"
#include "Camera.h"

Camera::Camera(void)
{
	buffer = new char[PICTURE_SIZE];
	memset(buffer, 255, PICTURE_SIZE);
	flagStop = false;
	cameraStopped = CreateEvent(NULL, true, false, L"cameraStopped");
}

Camera::~Camera(void)
{
	delete[] buffer;
}

void Camera::BeginShow()
{
	flagInit = false;
	flagStop = true;
	_beginthread(BeginShowThreadEntry, 0, this);
}

void Camera::BeginShowThreadEntry(void * args)
{
	((Camera *) args)->BeginShowThreadBody();
}

void Camera::BeginShowThreadBody()
{
	getFotosFromOneCam(buffer, PICTURE_WIDTH, PICTURE_HEIGHT, systemIndex, &flagInit, &flagStop, true, false, FPS);
	SetEvent(cameraStopped);
}

void Camera::EndShow()
{
	ResetEvent(cameraStopped);
	flagStop = false;
	WaitForSingleObject(cameraStopped, INFINITE);
}

