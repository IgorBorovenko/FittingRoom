#ifndef __GetFotosFromCams__
#define __GetFotosFromCams__

//ѕолучение фото с одной камеры
extern "C" __declspec(dllexport) int getFotosFromOneCam(char *img, int width, int height, int index, bool isShowSettingsWindow, int work, bool *flagInit, bool *flagStop, bool isFlipImg);
//Ќепрерывное получение изображений с двух камер
extern "C" __declspec(dllexport) int getVideoFromTwoCams(char *img1, char *img2, int width, int height, int index1, int index2, bool *flagInit, bool *flagStop, bool isShowSettingsWindow, bool isFlipImg);

#endif
