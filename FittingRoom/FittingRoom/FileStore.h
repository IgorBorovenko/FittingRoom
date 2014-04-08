#pragma once
#include "GFL.h"
using namespace std;
#define U_KEY 0x195566
#define FORMAT_JPG	0x01
#define FORMAT_RAW	0x02
#define FORMAT_NEF	0x03
#define FORMAT_PNG	0x04
#define FORMAT_TIF	0x05

#define VERSION 0x03

/*
typedef struct {
	ULONG key; /// uniqual key
	UINT  ver;	// fs version
	UINT frames;	// summary frames
	UINT time;	// summary time
	UINT frame;	// frame counter
	BYTE format;	// format id
	ULONG size;	// size of image
	UINT q; // quality
	unsigned char uid[16];   /// uid platform
	unsigned char auid[16];  // site auth uid
	unsigned char qr_code[128];

}FILE_STRORE_HEADER;

*/

#define FS_MAGIC_CODE 0x12675433

#pragma pack(1)

typedef struct {
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
} FILE_STORE_HEADER_V1;


#define _DEVICE_INFO	0x05	//   юник 
#define _PHOTO_TYPE		0x01	//   набор фоток
#define FILE_STORE_VER	0x01	// версия


class CFileStore
{
public:
	CFileStore(void);
	~CFileStore(void);

	BOOL New(wstring f);
	BOOL Open(wstring f);
	BOOL Close();
	

	BOOL is_open();
	ULONGLONG get_filesize();
	wstring GenUID();
	UUID CFileStore::GenUID2();
	BOOL SaveBufFile(CString fn, LPVOID buffer, ULONG size);
//	BOOL SaveJpegImage(CString fn, GFL_BITMAP *img, int q);	
	BOOL SaveJpegImageFromAtom(CString fn, pATOM *a);
	BOOL SaveJpegImage(CString fn, pATOM *a);
	BOOL DeleteTmp(CString fn);
	//BOOL AddHeaderV3(UINT frames, UINT time,  unsigned char *uid, unsigned char  *auid, char *qr_code);
	

	//// Write to store.....

	BOOL UpdateHeader(FILE_STORE_HEADER_V1 *hh);
	BOOL StartToSaveImages();			// start to save images
	BOOL SaveImage(pATOM * a);			// save image
	BOOL StopToSaveImages();			/// save header
	FILE_STORE_HEADER_V1 GetCurrentHeader();

	BOOL ReadHeader(FILE_STORE_HEADER_V1 *hh);
	BOOL GetImage(LPVOID buffer, UINT index);
	
	BOOL Rename(CString f1, CString f2);

	UUID last_guid;

	

private:
	CFile file;
	CString fname;

	FILE_STORE_HEADER_V1 h;
	
public:
	int UpdateLogin(CString fname, CString login);
};

extern CFileStore fs;
