#include "FaceVerificationDll.h"
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;


int main()
{
	IFaceVerification*	pFaceVer = AllocFaceVerificationInstance(NULL);
	pFaceVer->trainFaceImage("test1");
	cout << "train finished test1" << endl;
	//system("pause");
	//faceVer.trainFaceImage("test2");
	//faceVer.faceVerifier();
	//input image to predict
	std::string path;
	char fileName[256];
	Mat image;
	path = pFaceVer->getCurrentModulePath();
	for (int i=0; i<=63; i++)
	{
		sprintf(fileName, "\\faces\\test2\\face_%d.jpg", i);
		cout << fileName << endl;
		image = imread(path + fileName, CV_LOAD_IMAGE_GRAYSCALE);
		//imshow("input faces", image);
		pFaceVer->predict(image);
	}

	FreeFaceVerifactionInstance(pFaceVer);
	return 0;
}