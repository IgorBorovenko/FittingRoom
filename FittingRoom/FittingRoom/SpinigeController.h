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
		DWORD		MID;				// магический код дожен быть всегда	0x12675432
		BYTE		VER;				//1
		BYTE		DEVICE_INFO;		//Устройство - источник
		BYTE		QR_CODE[37];		//QR код
		BYTE		PHOTO_TYPE;			//тип фотографии
		GUID		guid;				// guid photo
		BYTE		DEVICE_CODE[17];	//	аппаратный код устройства
		BYTE		COUNT_IMAGES;		//количество картинок, менше 64
		WORD		IMG_WIDTH;			//.. размер картинки
		WORD		IMG_HEIGHT;			//размер картинки
		BYTE		USERNAME[33];		//.. имя пользователя
		LONGLONG	IMG_INDEX[64];		// позиция в файле на картинку
		UINT 		IMG_SIZE[64];
	};
#pragma pack(pop)
};