#include "StdAfx.h"
#include "FileStore.h"
#include <Rpcdce.h>
#include "MemStore.h"

CFileStore fs;

CFileStore::CFileStore(void)
{
	
}

CFileStore::~CFileStore(void)
{
}


CString CFileStore::GenUID() {
	UUID generated;
	CString n;

	if (::UuidCreate(&generated) != RPC_S_OK) {
		return "";
	}

	last_guid  = generated;
	n.Format("%x-%x-%x-%x%x-%x%x%x%x%x%x",generated.Data1,generated.Data2, generated.Data3, generated.Data4[0], generated.Data4[1],generated.Data4[2],generated.Data4[3],generated.Data4[4],generated.Data4[5],generated.Data4[6],generated.Data4[7]);

	return n;

}

UUID CFileStore::GenUID2() {
	UUID generated;
	CString n;

	if (::UuidCreate(&generated) != RPC_S_OK) {
		return generated;
	}

	last_guid  = generated;

	return generated;

}

BOOL CFileStore::is_open() {
	CString name = file.GetFileName();

	if (name.GetLength()==0)
		return FALSE;

	return TRUE;
}

ULONGLONG CFileStore::get_filesize() {
	return file.GetLength();
}


BOOL CFileStore::Open(CString f) {

	fname = f;

	if (!file.Open(fname,  CFile::typeBinary  | CFile::modeReadWrite,0))
		return FALSE;

	file.SeekToBegin();
	UINT r = file.Read(&h, sizeof(FILE_STORE_HEADER_V1));
	if (r!=sizeof(FILE_STORE_HEADER_V1) ) {
		file.Close();
		return FALSE;
	}

	if (h.VER <FILE_STORE_VER ) {
		TRACE("FS incorrect version\n");
		return FALSE;
	}
	if (h.MID != FS_MAGIC_CODE ) {
		TRACE("FS invalid format\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CFileStore::New(CString f) {

	fname = f;
	if (!file.Open(fname, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite ,0))
		return FALSE;

	memset(&h, 0, sizeof(FILE_STORE_HEADER_V1));
	h.MID = FS_MAGIC_CODE;
	h.VER = FILE_STORE_VER;
	h.DEVICE_INFO = _DEVICE_INFO;
	h.PHOTO_TYPE = _PHOTO_TYPE;


	return TRUE;
}

BOOL CFileStore::Close() {

	file.Close();
	memset(&h, 0, sizeof(FILE_STORE_HEADER_V1));
	
	return TRUE;
}

FILE_STORE_HEADER_V1 CFileStore::GetCurrentHeader() {
	return h;
}


BOOL CFileStore::UpdateHeader(FILE_STORE_HEADER_V1 *hh) {
	memcpy(&h, hh, sizeof(h));

	return TRUE;

}


BOOL CFileStore::StartToSaveImages() {
	// seek  sizeof(h)
	file.SeekToBegin();
	file.Write(&h, sizeof(h));

	LONGLONG lOffset = sizeof(FILE_STORE_HEADER_V1);
	ULONGLONG lActual;
	lActual = file.Seek( lOffset, CFile::begin );
	return TRUE;
	
}


BOOL CFileStore::StopToSaveImages() {
	// seek 0
	file.SeekToBegin();
	file.Write(&h, sizeof(h));
	return TRUE;

}

BOOL CFileStore::ReadHeader(FILE_STORE_HEADER_V1 *hh) {
	// seek 0
	file.SeekToBegin();
	file.Read(hh, sizeof(FILE_STORE_HEADER_V1));
	return TRUE;
}

BOOL CFileStore::GetImage(LPVOID buffer, UINT index) {

	if (index>63)
		return FALSE;
	
	LONGLONG lOffset = h.IMG_INDEX[index];
	file.Seek( lOffset, CFile::begin );
	
 
	UINT r = file.Read(buffer, h.IMG_SIZE[index]);
	if (r!=h.IMG_SIZE[index]) {			
		return FALSE;		
	}
	
	return TRUE;
}


BOOL CFileStore::SaveImage(pATOM *a) {

	//	

	LONGLONG lOffset;
	lOffset = file.SeekToEnd();
	
	h.IMG_INDEX[h.COUNT_IMAGES] = lOffset;
	h.IMG_SIZE[h.COUNT_IMAGES] = a->jpg_size;
	h.COUNT_IMAGES++;

	file.Write(a->jpg, a->jpg_size);

	return TRUE;
}


BOOL CFileStore::SaveBufFile(CString fn, LPVOID buffer, ULONG size) {

	
	CFile f;

	if (!f.Open(fn, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite ,0))
		return FALSE;
	
	f.Write(buffer, size);

	f.Close();


	return TRUE;
}

/*
BOOL CFileStore::SaveJpegImage(CString fn, GFL_BITMAP *img, int q) {

	CFile f;

	if (!f.Open(fn, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite ,0))
		return FALSE;

	GFL_SAVE_PARAMS sparams;
	unsigned char* dst;
	GFL_UINT32 dst_size ;

	GFL_ERROR error; 


	gflGetDefaultSaveParams( &sparams ); 
	sparams.FormatIndex = gflGetFormatIndexByName("jpeg");
	// default
	sparams.Quality = 60;
	
	if (q == QUALITY_MAX)
		sparams.Quality = 100;
	if (q == QUALITY_MID)
		sparams.Quality = 75;
	if (q == QUALITY_LOW)
		sparams.Quality = 60;

	
	sparams.Compression = GFL_NO_COMPRESSION;
	

	error = gflSaveBitmapIntoMemory(&dst, &dst_size, img, &sparams); 
	const char * str = gflGetErrorString( error ); 

	if (error) {
		TRACE("FS SAVE IMAGE ERROR-> %s\n", error);
		return FALSE;
	}


	f.Write(dst, dst_size);
	f.Close();

	gflMemoryFree(dst);

	return TRUE;
}
*/
BOOL CFileStore::SaveJpegImage(CString fn, pATOM *a) {

	CFile f;

	if (!f.Open(fn, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite ,0))
		return FALSE;

	f.Write(a->jpg, a->jpg_size);
	f.Close();


	return TRUE;
}


BOOL CFileStore::SaveJpegImageFromAtom(CString fn, pATOM *a) {

	CFile f;

	if (!f.Open(fn, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite ,0))
		return FALSE;

	f.Write(a->jpg, a->jpg_size);
	f.Close();


	return TRUE;
}



BOOL CFileStore::DeleteTmp(CString fn) {

	try
	{
		CFile::Remove(fn);
	}
	catch (CFileException* pEx)
	{
	#ifdef _DEBUG
		afxDump << "File " << fn << " cannot be removed\n";
	#endif
		pEx->Delete();
	}


/*	try {
		f.Remove(fn);
	} catch (CFileException, pEx) {
		return FALSE;
	}*/

	
	return TRUE;
}

BOOL CFileStore::Rename(CString f1, CString f2) {

	CFile f;

	f.Rename(f1, f2);

	
	return TRUE;
}
int CFileStore::UpdateLogin(CString fname, CString login)
{
	FILE_STORE_HEADER_V1 h;


	try {

	if (!file.Open(fname,  CFile::typeBinary |  CFile::modeReadWrite ,0))
		return FALSE;

	memset(&h, 0, sizeof(FILE_STORE_HEADER_V1));

	file.SeekToBegin();
	UINT r = file.Read(&h, sizeof(FILE_STORE_HEADER_V1));
	if (r!=sizeof(FILE_STORE_HEADER_V1) ) {
		file.Close();
		return FALSE;
	}

	if (h.VER <FILE_STORE_VER ) {
		TRACE("FS incorrect version\n");
		file.Close();
		return FALSE;
	}
	if (h.MID != FS_MAGIC_CODE ) {
		TRACE("FS invalid format\n");
		file.Close();
		return FALSE;
	}

	if (login.GetLength()<sizeof(h.USERNAME))
		sprintf((char *)h.USERNAME, "%s", login.GetBuffer());
	else {
		file.Close();
		return FALSE;
	}
	file.SeekToBegin();
	file.Write(&h, sizeof(FILE_STORE_HEADER_V1));	
	file.Close();
	return TRUE;
	}
	catch (CFileException* pEx)
	{
		return FALSE;
	}
	
}
