#pragma once

#ifdef FACEVERIFIERDLL_EXPORTS	
	#define FACEVERIFIERDLL_API _declspec(dllexport)
#else
	#define FACEVERIFIERDLL_API _declspec(dllimport)
#endif

#include <Windows.h>
#include <opencv2/opencv.hpp>


//Result Code
typedef enum _tag_FaceVerifactionREsult{
	NTKO_FACEOP_OK = 0,						//success
	NTKO_FACEOP_VIDEO_CAPTURE_FAIEL,		//获取摄像头数据失败
	NTKO_FACEOP_HWND_NULL,					//传入窗口句柄为空或无效
	NTKO_FACEIOP_ERR_ARG,					//参数错误
	NTKO_FACEOP_TRAINING_USERNAME_EMPTY,	//训练用户名为空
	NTKO_FACEOP_TRAINING_USERNAME_EXIST,	//训练的用户已经存在
	NTKO_FACEOP_VERIFY_NO_TRAIN_INFO,		//没有查找到训练数据 
	NTKO_FACEOP_VERIFY_EYE_BLINK_TIMEOUT,	//
	NTKO_FACEOP_VERIFY_TIMEOUT				//
}NtkoFaceVerifactionResult;

class FACEVERIFIERDLL_API IFaceVerification
{
public:
	virtual void initialize() = 0;

	virtual void trainFaceImage(const std::string &user_name, bool replace = false) = 0;
	virtual void faceVerifier() = 0;
	virtual bool predict(const cv::Mat &image) = 0;

	virtual void setPredictScoreThreshold(const double score) = 0;
	virtual double getPredictScoreThreshold()const = 0;

	virtual void setCameraId(int id) = 0;
	virtual void setCameraWindowName(const std::string &name) = 0;

	virtual void setCameraWindowSize(int width, int height) = 0;
	virtual void setCameraWindowSize(cv::Size  size) = 0;
	virtual cv::Size getCameraWindowSize() const = 0;

	virtual double getPredictionScore()const = 0;
	virtual void setTrainingImageCount(int count) = 0;

	virtual cv::Mat	getCapturedFrame() = 0;
	virtual cv::Mat getProcessedFace() = 0;

	virtual void setBlinkThreshold(int count) = 0;
	virtual int getBlinkThreshold()const = 0;

	virtual void setHwndForShowCameraVideo(HWND	hWnd) = 0;
	virtual void setHwndForShowStatusTips(HWND hWnd) = 0;

	virtual bool getFaceVerifierReuslt()const = 0;

	virtual NtkoFaceVerifactionResult getOperateResult()const = 0;

	//set status string format:
	virtual void setMUIStrFormatForEyeBlinkTips(const std::wstring &strFormat) = 0;
	virtual void setMUIStrFormatForFaceVerifyTips(const std::wstring &strFormat) = 0;
	virtual void setMUIStrFormatForFaceTrainingTips(const std::wstring &strFormat) = 0;

	virtual std::string	getCurrentModulePath()const = 0;
	virtual void quit() = 0;
};

FACEVERIFIERDLL_API IFaceVerification*	AllocFaceVerificationInstance(HINSTANCE hModule);
FACEVERIFIERDLL_API void				FreeFaceVerificationInstance(IFaceVerification* pFace);