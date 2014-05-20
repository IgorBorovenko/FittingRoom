#include "StdAfx.h"
#include "SpinigeController.h"
// OpenCV
#include "opencv\cv.h"
#include "opencv\highgui.h"
#include <iostream>
#include <fstream>
using namespace std;
#pragma comment(lib, "opencv_core241d.lib")

char *readBuffer;
#define MAXSUB  200

void process_http(SOCKET S, char *host, char *page, char *poststr, int length);

//#define LISTENQ         1024
//
//extern int h_errno;

SpinigeController::SpinigeController(void)
{
	readBuffer = new char[1920*1080*3];//actually you need less memory, but who cares?
	
}
SpinigeController::~SpinigeController(void)
{
	delete[] readBuffer;
}

bool SpinigeController::loadPicturesFromFolderIntoArray(char *data, BYTE *imageCount, UINT *imageSizes, wstring folderName)
{
	*imageCount = 0;
	unsigned int arrPointer = 0;

	WIN32_FIND_DATA fd;
	wstring searchPath = folderName;
	searchPath.append(L"\\*.jpg");
	folderName.append(L"\\");
	HANDLE hFind=::FindFirstFile(searchPath.c_str(), &fd);
	bool result = true;

    if(hFind != INVALID_HANDLE_VALUE)
    {
        do{
			if (result == true)
			{
				HANDLE hFile;
				//memset(readBuffer, 0, 1920*1080*3);

				try
				{
					wstring pathToFile = folderName;
					pathToFile.append(fd.cFileName);
					hFile = CreateFile(pathToFile.c_str(),			// file to open
										GENERIC_READ,			// open for writing
										0,						// share for writing
										NULL,					// default security
										OPEN_EXISTING,			// Opens a file or device, only if it exists. If the specified file or device does not exist, the function fails and the last-error code is set to ERROR_FILE_NOT_FOUND (2).
										FILE_ATTRIBUTE_NORMAL,	// normal file
										NULL);					// no attr. template
					if (hFile == INVALID_HANDLE_VALUE)
						throw L"CreateFile failed.";
		
					DWORD dwPtr = SetFilePointer( hFile, 0, NULL, FILE_BEGIN); //set pointer position to the beginning of the file
					if (dwPtr == INVALID_SET_FILE_POINTER)
						throw L"SetFilePointer failed.";
		
					bool bErrorFlag = false;
		
					DWORD dwBytesWereRead;
					bErrorFlag = ReadFile(  hFile,          // open file handle
											readBuffer,		// A pointer to the buffer that receives the data read from a file or device.
											1920*1080*3,	// number of bytes to read
											&dwBytesWereRead,// number of bytes that were read
											NULL);          // no overlapped structure
					if (bErrorFlag == false)
						throw L"ReadFile failed.";

					/*CvMat cvm = cvMat(1080,1920, CV_32FC3, (void*)readBuffer);
					IplImage* img = cvDecodeImage(&cvm, 1);
					cvShowImage("wtf",img);*/

					//data->insert(data->end(), readBuffer, readBuffer + dwBytesWereRead - 1);
					memcpy(&data[arrPointer], readBuffer, dwBytesWereRead);
					arrPointer += dwBytesWereRead;
					
					*imageCount = *imageCount + 1;
					imageSizes[*imageCount - 1] = dwBytesWereRead;
				
				}
				catch(wchar_t *errorDesc)
				{
					result = false;
					LPCWSTR errorDetails;
					int errorCode = GetLastError();
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &errorDetails, 0, NULL );
					wchar_t *fullError = new wchar_t[999];
					wcscpy(fullError, errorDesc);
					wcscat(fullError, L" ");
					wcscat(fullError, errorDetails);
					MessageBox(NULL, fullError, L"Error", MB_OK);
					delete[] fullError;
				}

				CloseHandle(hFile);
			}
        }while(::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
	
	return result;
}

void SpinigeController::sendToSpinige(wstring folderName)
{
	unsigned int dataSize = 0;

	//TODO: change 10000000 to real size 
	char *images = new char[10000000];
	memset(images, NULL, 10000000);
	SpinigeHeader sh = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	BYTE imageCount = 0;
	UINT imageSizes[64];
	memset(imageSizes, 0, 64 * sizeof(UINT));
	
	bool success = loadPicturesFromFolderIntoArray(images, &imageCount, imageSizes, folderName);

	if (success == true)
	{
		sh.COUNT_IMAGES = imageCount;
		memcpy(sh.IMG_SIZE, imageSizes, 64 * sizeof(UINT));
		int headerSize = sizeof(SpinigeHeader);
		for (int i = 0; i < imageCount; i++)
			dataSize+=imageSizes[i];
		char *finalArray = new char[headerSize + dataSize];
		memcpy(finalArray, &sh, headerSize);
		memcpy(&finalArray[headerSize], images, dataSize);
		
		WSADATA wsaData;
		WSAStartup(2,&wsaData);

		struct sockaddr_in mySock;
		memset(&mySock, '\0', sizeof(mySock));
		struct hostent *h = gethostbyname("spinige.com");
		mySock.sin_family=AF_INET;
		mySock.sin_port=htons(80);
		mySock.sin_addr = *(struct in_addr *)h->h_addr_list[0];

		SOCKET S = INVALID_SOCKET;
		//TODO check socket
		S = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

		//TODO check connect
		int error = connect(S, (struct sockaddr *)&mySock, sizeof(mySock));
		//if(error==0)
		//	cout<<"connected"<<endl;*/
		char hname[] = "spinige.com";
		char page[] = "/fittingroom/fittingroom.php?action=upload&email=test@test.test&password=test&share=0";
		process_http(S, hname, page, finalArray, headerSize + dataSize);

		closesocket(S);
		WSACleanup();
		delete[] finalArray;
	}
	delete[] images;
}



void process_http(SOCKET S, char *host, char *page, char *data, int dataLength)
{
	char *sendline = new char[dataLength + 200]; //200 - for HTTP header
	memset(sendline, NULL, dataLength + 200);
	SSIZE_T n;
	_snprintf(sendline, 200,
		"POST %s HTTP/1.0\r\n"
		"Host: %s\r\n"
		"Content-type: application/octet-stream\r\n"
		"Content-length: %d\r\n\r\n"
		"", page, host, dataLength);
	int httpHeaderLength = strlen(sendline);
	memcpy(&sendline[httpHeaderLength], data, dataLength);
	
	//TODO check res > 0
	int res = send(S, sendline, httpHeaderLength + dataLength, 0);

	char buff[1024];
	recv(S,buff,1024,0);
	/*ofstream out ("out.txt", ios::out);
	for(int i = 0; i < 1024; i++)
		out<<buff[i];
	out.close();*/

	 delete[] sendline;
}