#pragma once 

#include "common.h"
#include "FaceVerificationDll.h"
#include <map>
#include "PreProcessFace.h"
#include "facedetect-dll.h"
#include "SHA1.h"

class FaceVerification : IFaceVerification
{
public:
	FaceVerification(HINSTANCE hModule = NULL);

	void initialize();

	void trainFaceImage(const std::string &user_name , bool replace = false);
	void faceVerifier();
	bool predict(const cv::Mat &image);

	void setPredictScoreThreshold(const double score);
	double getPredictScoreThreshold()const;

	void setCameraId(int id);
	void setCameraWindowName(const std::string &name);

	void setCameraWindowSize(int width, int height);
	void setCameraWindowSize(cv::Size  size);
	cv::Size getCameraWindowSize() const;

	double getPredictionScore()const;
	void setTrainingImageCount(int count);

	cv::Mat	getCapturedFrame();
	cv::Mat getProcessedFace();

	void setBlinkThreshold(int count);
	int getBlinkThreshold()const;

	void setHwndForShowCameraVideo(HWND	hWnd);
	void setHwndForShowStatusTips(HWND hWnd);

	bool getFaceVerifierReuslt()const;

	NtkoFaceVerifactionResult getOperateResult()const;

	//set status string format:
	void setMUIStrFormatForEyeBlinkTips(const std::wstring &strFormat);
	void setMUIStrFormatForFaceVerifyTips(const std::wstring &strFormatTimeout);
	void setMUIStrFormatForFaceTrainingTips(const std::wstring &strFormat);

	std::string	getCurrentModulePath()const;
	void quit();

private:
	void detectFace(cv::Mat frame, bool isPredict, const std::string &user_name = "");
	void processFace(cv::Mat frame, cv::Mat grayFrame, bool isPredict, const std::string &user_name = "");
	bool faceAlignment(cv::Mat face, cv::Mat &outFace);

	static void blinkThreadProc(void *);

	bool loadLableInfo();
	bool saveLableInfo();
	bool isTrainedThisUser(const std::string &user_name);
	bool fileIsExists(const std::string &fileName);

	cv::Mat addMaskToFace(cv::Mat face);
	cv::Mat shrinkInputImage(cv::Mat image);
	cv::Mat preProcessInputImage(cv::Mat img);
	cv::Mat addMaskToVideo(cv::Mat &frame);

	void showCameraFrameImage(cv::Mat &frame);
	void fillBitmapInfo(BITMAPINFO* bmi, int width, int height, int bpp, int origin);

	bool initilzeVideoCapture(cv::VideoCapture **ppCapture);

	bool isTimeoutUseTickCount(unsigned long startTime, unsigned long timeThreshold);

	std::string genLBPHFaceModelFileName();

	FaceVerification(const FaceVerification&);
	FaceVerification& operator = (const FaceVerification&);
private:

	int			m_faceWidth;
	int			m_faceHeight;

	std::map<int, std::string>	m_labelInfo;
	int			m_currentUserID;

	std::string	m_trainModelFilePath;
	HINSTANCE	m_hModule;

	int			m_positiveImageCount;
	int			m_minPositiveImageCount;
	int			m_cameraID;
	int			m_minNeighbors;

	double		m_scaleFactor;
	double		m_predictionScore;

	bool		m_trainStatus;

	bool		m_faceVerifierSuccessed;
	int			m_faceVerifierSuccessedCount;
	double		m_facePredictScoreThreshold;

	cv::Size	m_minSize;
	cv::Size	m_maxSize;
	cv::Size	m_cameraSize;

	cv::Scalar		m_eyeColor;
	cv::Scalar		m_textColor;

	std::string		m_videoWindowName;
	std::string		m_currentPath;

	std::string		m_faceCascadeFileName;
	std::string		m_eyeCascadeFileName;
	std::string		m_eyeGlassesCascadeFileName;

	std::vector<cv::Rect>	m_faceAreas;
	std::vector<cv::Mat>	m_faces;
	std::vector<cv::Mat>	m_reVerifierFaces;

	cv::Ptr<cv::FaceRecognizer>		m_LBPHFaceRecognizer;
	PreprocessFace	m_preProcessor;

	cv::CascadeClassifier	m_faceCascade;
	cv::CascadeClassifier	m_eyeCascade;
	cv::CascadeClassifier	m_eyeGlassesCascade;

	cv::Mat		m_preProcessedFace;
	cv::Mat		m_currentCaptureFrame;
	cv::Rect	m_faceRect;
	cv::Point	m_leftEye, m_rightEye;
	cv::Rect	m_searchedLeftEye, m_searchedRightEye;
	
	//eye blink
	int			m_blinkCount;
	int			m_blinkCountThreshold;
	bool		m_isEyeOpened;
	bool		m_needVerifierEyeBlink;
	bool		m_isTimeoutWhenVerifierEyeBlink;
	unsigned long	m_startTimeWhenEyeBlink;
	unsigned long	m_blinkTimeThreshold;

	//show Camera Video
	bool		m_isShowInWnd;
	HWND		m_hwndCameraVideo;

	//show Status Tips
	bool		m_isShowTipInWnd;
	HWND		m_hwndStatusTip;

	std::wstring		m_strFormatEyeBlinkTips;
	std::wstring		m_strFormatFaceVerifyTips;
	std::wstring		m_strFormatFaceTraining;

	bool		m_isQuitNow;

	//Result code
	NtkoFaceVerifactionResult		m_opResultCode;

	//verify timeout setting
	unsigned long	m_startTimeWhenVerifyFace;
	unsigned long	m_verifyTimeThreshold;
};