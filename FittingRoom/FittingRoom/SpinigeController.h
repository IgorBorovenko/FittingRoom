#pragma once
#include "MyConstants.h"
#include <vector>
#pragma comment(lib,"Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

class SpinigeController
{
public:
	SpinigeController(void);
	~SpinigeController(void);
	void createSpinigeStorage(wstring folderName);
	bool SpinigeController::loadPicturesFromFolderIntoArray(char *data, BYTE *imageCount, UINT *imageSizes, wstring folderName);

#pragma pack(push,1)
	struct SpinigeHeader
	{
		DWORD		MID;				// ���������� ��� ����� ���� ������	0x12675432
		BYTE		VER;				//1
		BYTE		DEVICE_INFO;		//���������� - ��������
		BYTE		QR_CODE[37];		//QR ���
		BYTE		PHOTO_TYPE;			//��� ����������
		GUID		guid;				// guid photo
		BYTE		DEVICE_CODE[17];	//	���������� ��� ����������
		BYTE		COUNT_IMAGES;		//���������� ��������, ����� 64
		WORD		IMG_WIDTH;			//.. ������ ��������
		WORD		IMG_HEIGHT;			//������ ��������
		BYTE		USERNAME[33];		//.. ��� ������������
		LONGLONG	IMG_INDEX[64];		// ������� � ����� �� ��������
		UINT 		IMG_SIZE[64];
	};
#pragma pack(pop)
};