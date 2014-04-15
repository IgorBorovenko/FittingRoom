#pragma once
class FolderProperties
{
public:
	FolderProperties(void);
	~FolderProperties(void);

	//wstring name; 
	RECT winRect; 
	bool marked; 
	//IplImage* thumbnail;
};

