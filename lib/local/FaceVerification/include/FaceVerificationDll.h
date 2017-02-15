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
	NTKO_FACEOP_VIDEO_CAPTURE_FAIEL,		//��ȡ����ͷ����ʧ��
	NTKO_FACEOP_HWND_NULL,					//���봰�ھ��Ϊ�ջ���Ч
	NTKO_FACEIOP_ERR_ARG,					//��������
	NTKO_FACEOP_TRAINING_USERNAME_EMPTY,	//ѵ���û���Ϊ��
	NTKO_FACEOP_TRAINING_USERNAME_EXIST,	//ѵ�����û��Ѿ�����
	NTKO_FACEOP_VERIFY_NO_TRAIN_INFO,		//û�в��ҵ�ѵ������ 
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