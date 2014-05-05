#ifndef __GetFotosFromCamsAlt__
#define __GetFotosFromCamsAlt__

//Получение фото с одной камеры
extern "C" __declspec(dllexport) int getFotosFromOneCam(char *img, int width, int height, int index, bool *flagInit, bool *flagStop, bool isFlip, bool isOneFrame, int fps);

#endif