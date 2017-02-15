#define FACEVERIFIERDLL_EXPORTS

#include "FaceVerification.h"
#include <fstream>
#include <sstream>
#include <windowsx.h>


IFaceVerification* AllocFaceVerificationInstance(HINSTANCE hModule)
{
	FaceVerification* pFaceVerifaction = new FaceVerification(hModule);
	return (IFaceVerification*)pFaceVerifaction;
}

void FreeFaceVerificationInstance(IFaceVerification* pFace)
{
	if (pFace)
	{
		delete pFace;
		pFace = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
FaceVerification::FaceVerification(HINSTANCE hModule)
{
	if (hModule)
		m_hModule = hModule;
	else
		m_hModule = NULL;
	initialize();
}

void FaceVerification::initialize()
{
	m_positiveImageCount = 0;
	m_minPositiveImageCount = DEFAULT_TRAINING_IMAGE_COUNT;
	m_cameraID = 0;
	m_predictionScore = 0.0;

	m_currentUserID = 1001;

	m_faceWidth = FACEVERIFIER_FACE_WIDTH;
	m_faceHeight = FACEVERIFIER_FACE_HEIGHT;

	m_faces.clear();
	m_faceAreas.clear();
	m_reVerifierFaces.clear();

	m_minNeighbors = 3;
	m_scaleFactor = 1.08;
	m_minSize = cv::Size(64, 64);
	m_maxSize = cv::Size(400, 400);
	m_eyeColor = cv::Scalar(127, 127, 117);
	m_textColor = cv::Scalar(255, 0, 0);

	m_cameraSize = cv::Size(640, 480);

	m_trainStatus = false;

	m_faceVerifierSuccessed = false;
	m_faceVerifierSuccessedCount = 0;
	m_facePredictScoreThreshold = DEFAULT_PREDICT_SCORE_THRESHOLD;

	m_videoWindowName = "FaceVerification";
	m_currentPath = getCurrentModulePath();
	m_faceCascadeFileName = m_currentPath + "\\Cascades\\haarcascades\\haarcascade_frontalface_alt.xml";
	m_eyeCascadeFileName = m_currentPath + "\\Cascades\\haarcascades\\haarcascade_eye.xml";
	m_eyeGlassesCascadeFileName = m_currentPath + "\\Cascades\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml";

	m_trainModelFilePath = genLBPHFaceModelFileName();

	m_faceCascade.load(m_faceCascadeFileName);
	m_eyeCascade.load(m_eyeCascadeFileName);
	m_eyeGlassesCascade.load(m_eyeGlassesCascadeFileName);

	m_LBPHFaceRecognizer = cv::createLBPHFaceRecognizer();
	m_LBPHFaceRecognizer->setDouble("threshold", 150.0);

	m_preProcessedFace = NULL;
	m_currentCaptureFrame = NULL;

	//eye blink
	m_isEyeOpened = false;
	m_blinkCount = 0;
	m_blinkCountThreshold = DEFAULT_EYE_BLINK_MAX_COUNT;
	m_needVerifierEyeBlink = true;
	m_isTimeoutWhenVerifierEyeBlink = false;
	m_startTimeWhenEyeBlink = 0;
	m_blinkTimeThreshold = DEFAULT_EYE_BLINK_TIME_OUT;

	//show Camera Video in window
	m_isShowInWnd = false;
	m_hwndCameraVideo = NULL;

	//show status tips in window
	m_isShowTipInWnd = false;
	m_hwndStatusTip = NULL;
	m_strFormatEyeBlinkTips.clear();
	m_strFormatFaceTraining.clear();
	m_strFormatFaceVerifyTips.clear();

	m_isQuitNow = false;

	m_opResultCode = NTKO_FACEOP_OK;

	//face verify time out setting
	m_startTimeWhenVerifyFace = 0;
	m_verifyTimeThreshold = DEFAULT_VERIFIER_TIME_OUT;
}

void FaceVerification::trainFaceImage(const std::string & user_name, bool replace)
{
	m_opResultCode = NTKO_FACEOP_OK;
	if (user_name.empty())
	{
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
		std::cout << __FUNCTION__ << " : user_name is empty." << std::endl;
#endif
		m_opResultCode = NTKO_FACEOP_TRAINING_USERNAME_EMPTY;
		return;
	}

#ifndef NTKO_FACE_USE_SIGNAL_PERSON_MODE  //
	if (loadLableInfo())
	{
		if (fileIsExists(m_trainModelFilePath))
		{
			try
			{
				;//m_LBPHFaceRecognizer->load(m_trainModelFilePath);
			}
			catch (cv::Exception*) {
			}
		}

		if (isTrainedThisUser(user_name))
		{
			
			//replace old train info
			if (replace)
			{
				m_currentUserID--;
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
				std::cout << __FUNCTION__ << " : this user name has in train sets. but now replace it" << std::endl;
#endif
			}
			else
			{
				m_trainStatus = true;
				m_opResultCode = NTKO_FACEOP_TRAINING_USERNAME_EXIST;
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
				std::cout << __FUNCTION__ << " : this user name has in train sets." << std::endl;
#endif
				return;
			}
		}
	}
#endif

	cv::VideoCapture *pCapture = NULL;
	if (!initilzeVideoCapture(&pCapture))
	{
		m_opResultCode = NTKO_FACEOP_VIDEO_CAPTURE_FAIEL;
		return;
	}

	cv::Mat frame;
	m_positiveImageCount = 0;
	m_needVerifierEyeBlink = false;
	
	while (m_positiveImageCount <= m_minPositiveImageCount)
	{
		*pCapture >> frame;
		m_currentCaptureFrame = frame;

		detectFace(frame, false, user_name);

		// show the Camera frame
		showCameraFrameImage(frame);

		if ((27 == cv::waitKey(100)) || m_isQuitNow)
			break;
	}
	if (pCapture)
	{
		pCapture->release();
		delete pCapture;
	}
}

void FaceVerification::faceVerifier()
{
	m_opResultCode = NTKO_FACEOP_OK;

#ifdef NTKO_FACE_USE_SIGNAL_PERSON_MODE  //
	if (!fileIsExists(m_trainModelFilePath))
	{
		m_opResultCode = NTKO_FACEOP_VERIFY_NO_TRAIN_INFO;
		return;
	}
#else
	if (!m_trainStatus)
	{
		if (loadLableInfo() && fileIsExists(m_trainModelFilePath))
		{
			;// m_trainStatus = true;
		}
		else
		{
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
			std::cout << __FUNCTION__ << " Hasn't train user face" << std::endl;
#endif
			m_opResultCode = NTKO_FACEOP_VERIFY_NO_TRAIN_INFO;
			return;
		}
	}

#endif

	cv::VideoCapture *pCapture = NULL;
	if (!initilzeVideoCapture(&pCapture))
	{
		m_opResultCode = NTKO_FACEOP_VIDEO_CAPTURE_FAIEL;
		return;
	}

	cv::Mat frame;
	m_needVerifierEyeBlink = true;
	m_startTimeWhenVerifyFace = GetTickCount();
	m_startTimeWhenEyeBlink	= m_startTimeWhenVerifyFace;

	while (!m_faceVerifierSuccessed && !m_isTimeoutWhenVerifierEyeBlink)
	{
		*pCapture >> frame;
		m_currentCaptureFrame = frame;

		if (m_needVerifierEyeBlink)
		{
			if (m_isShowTipInWnd && m_hwndStatusTip && !m_strFormatEyeBlinkTips.empty())
			{
				::SetWindowTextW(m_hwndStatusTip, m_strFormatEyeBlinkTips.c_str());
			}

			if(isTimeoutUseTickCount(m_startTimeWhenEyeBlink, m_blinkTimeThreshold))
			{
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
				std::ostringstream strStream;
				strStream << "It's time out  when eye blink";
				std::cout << strStream.str() << std::endl;

				cv::putText(m_currentCaptureFrame, strStream.str(),
					cv::Point(5, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, m_textColor);
#endif
				m_isTimeoutWhenVerifierEyeBlink = true;
				m_opResultCode = NTKO_FACEOP_VERIFY_EYE_BLINK_TIMEOUT;
				break;
			}

			if (m_blinkCount >= m_blinkCountThreshold)
			{
				m_blinkCount = 0;
				m_needVerifierEyeBlink = false;
				m_startTimeWhenEyeBlink = 0;

				if (m_isShowTipInWnd && m_hwndStatusTip && !m_strFormatFaceVerifyTips.empty())
				{
					::SetWindowTextW(m_hwndStatusTip, m_strFormatFaceVerifyTips.c_str());
				}
			}
		}

		if(isTimeoutUseTickCount(m_startTimeWhenVerifyFace, m_verifyTimeThreshold))
		{
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
			std::ostringstream strStream;
			strStream << "It's time out  when verifier face";
			std::cout << strStream.str() << std::endl;

			cv::putText(m_currentCaptureFrame, strStream.str(),
				cv::Point(5, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, m_textColor);
#endif
			m_opResultCode = NTKO_FACEOP_VERIFY_TIMEOUT;
			break;
		}

		detectFace(frame, true);

		// show the Camera frame
		showCameraFrameImage(frame);

		if ((27 == cv::waitKey(30)) || m_isQuitNow)
			break;
	}
	if (pCapture)
	{
		pCapture->release();
		delete pCapture;
	}
}

void FaceVerification::setCameraId(int id)
{
	if (id >= 0)
		m_cameraID = id;
}

void FaceVerification::setCameraWindowName(const std::string &name)
{
	if (!name.empty())
	{
		m_videoWindowName = name;
	}
}

void FaceVerification::setCameraWindowSize(int width, int height)
{
	cv::Size size(width, height);
	setCameraWindowSize(size);
}

void FaceVerification::setCameraWindowSize(cv::Size size)
{
	if (size.width >= 200 && size.height >= 150)
	{
		m_cameraSize = size;
	}
}

cv::Size FaceVerification::getCameraWindowSize() const
{
	return m_cameraSize;
}

bool FaceVerification::predict(const cv::Mat &faceGray)
{
	int	label = -1;
	std::string predictUser;
	m_predictionScore = 0.0;
	bool	bSuccess = false;

	if (!fileIsExists(m_trainModelFilePath))
		return false;

	try
	{
		m_LBPHFaceRecognizer->load(m_trainModelFilePath);
	}
	catch (cv::Exception*) {
		return false;
	}

	m_LBPHFaceRecognizer->predict(faceGray, label, m_predictionScore);
	predictUser = m_LBPHFaceRecognizer->getLabelInfo(label);

#ifdef NTKO_FACE_SHOW_DEBUG_INFO
	std::ostringstream strStream;
	strStream << "Predict Score:" << m_predictionScore << ", user:[" << predictUser << "], label: " << label;

	std::cout << strStream.str() << std::endl;
	cv::putText(m_currentCaptureFrame, strStream.str(), cv::Point(5, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, m_textColor);
#endif

	//if (m_isShowTipInWnd && m_hwndStatusTip)
	//{
	//	::SetWindowTextA(m_hwndStatusTip, strStream.str().c_str());
	//}

	if ((label != -1) && m_predictionScore < m_facePredictScoreThreshold)
	{
		bSuccess = true;
		std::ostringstream strStream;

		if (++m_faceVerifierSuccessedCount >= FACEVERIFIER_SUCCEED_COUNT_THRESHOLD)
		{
			m_faceVerifierSuccessed = true;
			m_startTimeWhenVerifyFace = 0;

			strStream << "Face Verifier Succeed!";
		}
		else
		{
			strStream << "Current Verifier Count: " << m_faceVerifierSuccessedCount;
		}

#ifdef NTKO_FACE_SHOW_DEBUG_INFO
		cv::putText(m_currentCaptureFrame, strStream.str(), cv::Point(5, 60), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, m_textColor);
		std::cout << strStream.str() << std::endl;
#endif

		//if (m_isShowTipInWnd && m_hwndStatusTip)
		//{
		//	::SetWindowTextA(m_hwndStatusTip, strStream.str().c_str());
		//}

		m_reVerifierFaces.clear();
	}

	m_reVerifierFaces.push_back(faceGray);
	return bSuccess;
}

void FaceVerification::detectFace(cv::Mat frame, bool isPredict, const std::string &user_name)
{
	cv::Mat grayFrame;

	grayFrame = preProcessInputImage(frame);

	int *pResults = NULL;
	m_faceAreas.clear();

#if 0
	pResults = facedetect_frontal_tmp((unsigned char*)(grayFrame.ptr(0)),
		grayFrame.cols, grayFrame.rows, grayFrame.step, (float)m_scaleFactor, m_minNeighbors, m_minSize.width);
#else
	pResults = facedetect_frontal_surveillance((unsigned char*)(grayFrame.ptr(0)),
		grayFrame.cols, grayFrame.rows, grayFrame.step, (float)m_scaleFactor, m_minNeighbors, m_minSize.width);
#endif 
	for (int i = 0; i < (pResults ? *pResults : 0); i++)
	{
		short * p = ((short*)(pResults + 1)) + 6 * i;
		int x = p[0];
		int y = p[1];
		int w = p[2];
		int h = p[3];
		//int neighbors = p[4];

		m_faceAreas.push_back(cv::Rect(x, y, w, h));
	}
	processFace(frame, grayFrame, isPredict, user_name);
}

double FaceVerification::getPredictionScore()const
{
	return m_predictionScore;
}

void FaceVerification::setTrainingImageCount(int count)
{
	if (count >= 60)
		m_minPositiveImageCount = count;
}

cv::Mat FaceVerification::getCapturedFrame()
{
	if (m_currentCaptureFrame.empty())
		return cv::Mat();

	return m_currentCaptureFrame;
}

cv::Mat FaceVerification::getProcessedFace()
{
	if (m_preProcessedFace.empty())
		return cv::Mat();

	return m_preProcessedFace;
}

void FaceVerification::setBlinkThreshold(int count)
{
	if (count >= DEFAULT_EYE_BLINK_MAX_COUNT)
	{
		m_blinkCountThreshold = count;
	}
}

int FaceVerification::getBlinkThreshold() const
{
	return m_blinkCountThreshold;
}

void FaceVerification::setHwndForShowCameraVideo(HWND hWnd)
{
	m_opResultCode = NTKO_FACEOP_OK;
	if (hWnd == NULL || !::IsWindow(hWnd))
	{
		m_opResultCode = NTKO_FACEOP_HWND_NULL;
		return;
	}

	m_hwndCameraVideo = hWnd;
	m_isShowInWnd = true;
}

void FaceVerification::setHwndForShowStatusTips(HWND hWnd)
{
	m_opResultCode = NTKO_FACEOP_OK;
	if (hWnd == NULL || !::IsWindow(hWnd))
	{
		m_opResultCode = NTKO_FACEOP_HWND_NULL;
		return;
	}
	m_hwndStatusTip = hWnd;
	m_isShowTipInWnd = true;
}

bool FaceVerification::getFaceVerifierReuslt() const
{
	return m_faceVerifierSuccessed;
}

NtkoFaceVerifactionResult FaceVerification::getOperateResult() const
{
	return m_opResultCode;
}

void FaceVerification::processFace(cv::Mat frame, cv::Mat grayFrame, bool isPredict, const std::string &user_name)
{
#if 0
	std::ostringstream strStream;
	strStream << "face count: " << m_faceAreas.size();
	cv::putText(frame, strStream.str(), cv::Point(5, 40), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, m_textColor);
	std::cout << strStream.str() << std::endl;
#endif

	if (!m_faceAreas.size())
	{
		if (m_needVerifierEyeBlink)
		{
			m_blinkCount = 0;
		}
		return;
}

	//choose the biggest face
	cv::Rect faceArea = m_faceAreas[0];
	for (size_t i = 1; i < m_faceAreas.size(); i++)
	{
		cv::Rect face_i = m_faceAreas[i];
		if (face_i.width > faceArea.width)
			faceArea = face_i;
	}
	cv::rectangle(frame, faceArea, cv::Scalar(0, 255, 0), 1);

	cv::Mat faceGray;
	{
		//keep face in the image.
		if (faceArea.x < 0) {
			faceArea.x = 0;
		}
		if (faceArea.y < 0) {
			faceArea.y = 0;
		}
		if ((faceArea.x + faceArea.width) > grayFrame.cols) {
			faceArea.width = grayFrame.cols - faceArea.x;
		}
		if ((faceArea.y + faceArea.height) > grayFrame.rows) {
			faceArea.height = grayFrame.rows - faceArea.y;
		}

		faceGray = grayFrame(faceArea);

#if 0
		m_preProcessedFace = m_preProcessor.getPreprocessedFace(
			face, m_faceWidth, m_faceCascade, m_eyeCascade, m_eyeGlassesCascade, true,
			&m_faceRect, &m_leftEye, &m_rightEye, &m_searchedLeftEye, &m_searchedRightEye);
#else
		m_preProcessor.detectBothEyes(faceGray, m_eyeCascade, m_eyeGlassesCascade, m_leftEye, m_rightEye, &m_searchedLeftEye, &m_searchedRightEye);
#endif

		if (m_leftEye.x > 0)
		{
			//draw '+'
			cv::line(frame, cv::Point(faceArea.x + m_faceRect.x + m_leftEye.x - 10, faceArea.y + m_faceRect.y + m_leftEye.y),
				cv::Point(faceArea.x + m_faceRect.x + m_leftEye.x + 10, faceArea.y + m_faceRect.y + m_leftEye.y), m_eyeColor, 1);
			cv::line(frame, cv::Point(faceArea.x + m_faceRect.x + m_leftEye.x, faceArea.y + m_faceRect.y + m_leftEye.y - 10),
				cv::Point(faceArea.x + m_faceRect.x + m_leftEye.x, faceArea.y + m_faceRect.y + m_leftEye.y + 10), m_eyeColor, 1);
		}

		if (m_rightEye.x > 0)
		{
			//draw '+'
			cv::line(frame, cv::Point(faceArea.x + m_faceRect.x + m_rightEye.x - 10, faceArea.y + m_faceRect.y + m_rightEye.y),
				cv::Point(faceArea.x + m_faceRect.x + m_rightEye.x + 10, faceArea.y + m_faceRect.y + m_rightEye.y), m_eyeColor, 1);
			cv::line(frame, cv::Point(faceArea.x + m_faceRect.x + m_rightEye.x, faceArea.y + m_faceRect.y + m_rightEye.y - 10),
				cv::Point(faceArea.x + m_faceRect.x + m_rightEye.x, faceArea.y + m_faceRect.y + m_rightEye.y + 10), m_eyeColor, 1);
		}

		if (m_leftEye.x > 0 && m_rightEye.x > 0)
		{
			if (m_needVerifierEyeBlink)
			{
				if (!m_isEyeOpened)
				{
					m_isEyeOpened = true;
					m_blinkCount++;
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
					std::ostringstream strStream;
					strStream << "m_blinCount = " << m_blinkCount;

					std::cout << strStream.str() << std::endl;
					cv::putText(frame, strStream.str(), cv::Point(5, 20), 
						cv::FONT_HERSHEY_COMPLEX_SMALL, 1, m_textColor);
#endif
					//if (m_isShowTipInWnd && m_hwndStatusTip)
					//{
					//	::SetWindowTextA(m_hwndStatusTip, strStream.str().c_str());
					//}
				}
				return;
			}

			cv::Mat alignFace;
			faceAlignment(faceGray, alignFace);

			cv::Mat processedFace;
			processedFace = addMaskToFace(alignFace);

			m_preProcessedFace = processedFace;
#ifdef NTKO_FACE_SHOW_DEBUG_INFO	// save faces	
			std::string path;
			char fileName[256];
			sprintf(fileName, "\\faces\\%s\\face_%d.jpg", user_name.c_str(), m_positiveImageCount);
			path = m_currentPath + fileName;
			cv::imwrite(path, processedFace);
#endif 
			if (isPredict)
			{
				predict(processedFace);
			}
			else
			{
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
				std::ostringstream strStream;
				strStream << " get face image:  " << m_positiveImageCount;

				std::cout << strStream.str() << std::endl;
				cv::putText(frame, strStream.str(), cv::Point(5, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, m_textColor);
#endif
				if (m_isShowTipInWnd && m_hwndStatusTip && !m_strFormatFaceTraining.empty())
				{
					::SetWindowTextW(m_hwndStatusTip, m_strFormatFaceTraining.c_str());
				}

				m_faces.push_back(processedFace);
				if (m_positiveImageCount++ >= m_minPositiveImageCount)
				{
					std::vector<int> tags(m_faces.size(), m_currentUserID);

#ifdef NTKO_FACE_SHOW_DEBUG_INFO
					std::cout << "begin train LBPH face recognizer" << std::endl;
#endif
#if 1
					m_LBPHFaceRecognizer->train(m_faces, tags);
#else
					m_LBPHFaceRecognizer->update(m_faces, tags);
#endif

					m_labelInfo.insert(std::make_pair(m_currentUserID, user_name));
					m_LBPHFaceRecognizer->setLabelsInfo(m_labelInfo);

					if (m_trainModelFilePath.empty())
					{
						m_trainModelFilePath = genLBPHFaceModelFileName();
					}

					m_LBPHFaceRecognizer->save(m_trainModelFilePath);

#ifndef NTKO_FACE_USE_SIGNAL_PERSON_MODE
					saveLableInfo();
#endif
					m_trainStatus = true;
				}
			}
		}
		else if (m_needVerifierEyeBlink)
		{
			m_isEyeOpened = false;
		}
	}
}

bool FaceVerification::faceAlignment(cv::Mat face, cv::Mat &outFace)
{
	//use eye point's y coordinate calculate the angle, then aligment face;
	if (m_leftEye.x > 0 && m_leftEye.y > 0 && m_rightEye.x > 0 && m_rightEye.y > 0)
	{
		cv::Point2f eyesCenter = cv::Point2f((m_leftEye.x + m_rightEye.x) * 0.5f, (m_leftEye.y + m_rightEye.y) * 0.5f);
		// Get the angle between the 2 eyes.
		double dy = (m_rightEye.y - m_leftEye.y);
		double dx = (m_rightEye.x - m_leftEye.x);
		double len = sqrt(dx*dx + dy*dy);
		double angle = atan2(dy, dx) * 180.0 / CV_PI;

		const double DESIRED_RIGHT_EYE_X = (1.0f - DESIRED_LEFT_EYE_X);
		double desiredLen = (DESIRED_RIGHT_EYE_X - DESIRED_LEFT_EYE_X) * m_faceWidth;
		double scale = desiredLen / len;

		cv::Mat rot_mat = getRotationMatrix2D(eyesCenter, angle, scale);

		double ex = m_faceWidth *0.5f - eyesCenter.x;
		double ey = m_faceHeight * DESIRED_LEFT_EYE_Y - eyesCenter.y;
		rot_mat.at<double>(0, 2) += ex;
		rot_mat.at<double>(1, 2) += ey;

		cv::Mat warped = cv::Mat(m_faceHeight, m_faceWidth, CV_8U, cv::Scalar(128)); // Clear the output image to a default gray.
		warpAffine(face, warped, rot_mat, warped.size());
		outFace = warped.clone();
		return true;
	}

	return false;
}

void FaceVerification::blinkThreadProc(void *parm)
{
	FaceVerification *pFaceVerifier = (FaceVerification*)parm;

	//setting the waiting time for blink eye when face verification
	Sleep(DEFAULT_EYE_BLINK_TIME_OUT);

	if (pFaceVerifier->m_needVerifierEyeBlink)
	{
		pFaceVerifier->m_isTimeoutWhenVerifierEyeBlink = true;
		std::ostringstream strStream;
		strStream << "It's time out  when eye blink";
		std::cout << strStream.str() << std::endl;
		if(!pFaceVerifier->m_currentCaptureFrame.empty())
		{
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
			cv::putText(pFaceVerifier->m_currentCaptureFrame, strStream.str(),
				cv::Point(5, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, pFaceVerifier->m_textColor);
#endif
		}

		if (pFaceVerifier->m_isShowTipInWnd && pFaceVerifier->m_hwndStatusTip)
		{
			::SetWindowTextA(pFaceVerifier->m_hwndStatusTip, strStream.str().c_str());
		}

		pFaceVerifier->m_opResultCode = NTKO_FACEOP_VERIFY_EYE_BLINK_TIMEOUT;
	}
}

std::string FaceVerification::getCurrentModulePath()const
{
	std::string path;
	wchar_t buffer[MAX_PATH];

	if(m_hModule)
		GetModuleFileName(m_hModule, buffer, sizeof(buffer));
	else
		GetModuleFileName(NULL, buffer, sizeof(buffer));

	std::wstring ws(buffer);
	std::string str(ws.begin(), ws.end());
	const size_t last_slash_idx = str.rfind('\\');

	if (std::string::npos != last_slash_idx)
	{
		path = str.substr(0, last_slash_idx);
	}

	return path;
}

void FaceVerification::quit()
{
	m_isQuitNow = true;
}

bool FaceVerification::loadLableInfo()
{
	if (m_currentPath.empty())
	{
		return false;
	}

	std::ifstream	infile(m_currentPath + "\\labInfo.bin");
	if (infile.is_open())
	{
		std::string line;
		int	user_label;
		std::string user_name;

		m_labelInfo.clear();

		while (std::getline(infile, line))
		{
			std::istringstream	item(line);
			item >> user_label >> user_name;
			m_labelInfo.insert(std::make_pair(user_label, user_name));
		}
		infile.close();

		if (!m_labelInfo.empty())
		{
			m_currentUserID = m_labelInfo.rbegin()->first + 1;
		}
		return true;
	}
	return false;
}

bool FaceVerification::saveLableInfo()
{
	if (m_currentPath.empty())
		return false;

	std::ofstream	outfile(m_currentPath + "\\labInfo.bin");
	if (outfile.is_open())
	{
		if (!m_labelInfo.empty())
		{
			for (std::map<int, std::string>::const_iterator iter = m_labelInfo.begin();
				iter != m_labelInfo.end(); iter++)
			{
				outfile << iter->first << " " << iter->second << std::endl;
			}
		}

		outfile.close();
		return true;
	}
	return false;
}

bool FaceVerification::isTrainedThisUser(const std::string & user_name)
{
	if (m_labelInfo.empty())
		return false;

	bool bFind = false;
	std::map<int, std::string>::const_iterator iter = m_labelInfo.begin();
	for (iter; iter != m_labelInfo.end(); ++iter)
	{
		if (iter->second == user_name)
		{
			bFind = true;
			break;
		}
	}
	return bFind;
}

cv::Mat FaceVerification::addMaskToFace(cv::Mat face)
{
	// Draw a filled ellipse in the middle of the face-sized image.
	cv::Mat mask = cv::Mat(face.size(), CV_8U, cv::Scalar(0));
	cv::Point faceCenter = cv::Point(cvRound(face.cols / 2), cvRound(face.rows * FACE_ELLIPSE_CY));
	cv::Size size = cv::Size(cvRound(face.cols * FACE_ELLIPSE_W), cvRound(face.rows * FACE_ELLIPSE_H));
	ellipse(mask, faceCenter, size, 0, 0, 360, cv::Scalar(255), CV_FILLED);

	cv::Mat filtered = cv::Mat(face.size(), face.type());
	cv::bilateralFilter(face, filtered, 0, 20.0, 2.0);

	cv::Mat desImg = cv::Mat(face.size(), CV_8U, cv::Scalar(128));
	filtered.copyTo(desImg, mask);

	return desImg;
}

cv::Mat FaceVerification::addMaskToVideo(cv::Mat &frame)
{
	double alpha = FACE_MASK_WEIGHTED_VALUE;
	double beta = 1 - alpha;

	// Draw a ellipse in the middle of the frame-sized image.
	cv::Mat mask = cv::Mat(frame.size(), frame.type(), cv::Scalar(0));
	cv::Point faceCenter = cv::Point(cvRound(frame.cols / 2), cvRound(frame.rows *FACE_ELLIPSE_CY));
	cv::Size size = cv::Size(cvRound(frame.cols * FACE_MASK_ELLIPSE_W), cvRound(frame.rows * FACE_MASK_ELLIPSE_H));
	ellipse(mask, faceCenter, size, 0, 0, 360, cv::Scalar(255, 255, 255), CV_FILLED);

	addWeighted(frame, alpha, mask, beta, 0.0, frame);
	return frame;
}

cv::Mat FaceVerification::shrinkInputImage(cv::Mat img)
{
	cv::Mat smallImg;
	float scale = img.cols / (float)DETECTION_WIDTH;
	if (img.cols > DETECTION_WIDTH)
	{
		int scaledHeight = cvRound(img.rows / scale);
		resize(img, smallImg, cv::Size(DETECTION_WIDTH, scaledHeight));
	}
	else
	{
		smallImg = img;
	}
	return smallImg;
}

cv::Mat FaceVerification::preProcessInputImage(cv::Mat img)
{
	if (img.empty())
		return cv::Mat();

	cv::Mat grayImage;

	if (img.channels() == 3)
	{
		cv::cvtColor(img, grayImage, CV_RGB2GRAY);
	}
	else if (img.channels() == 4)
	{
		cv::cvtColor(img, grayImage, CV_BGRA2GRAY);
	}
	else
	{
		grayImage = img;
	}

	cv::Mat equalizeImg;
	//cv::equalizeHist(grayImage, equalizeImg);
	equalizeImg = grayImage;

	return equalizeImg;
}

void FaceVerification::showCameraFrameImage(cv::Mat &frame)
{
	m_opResultCode = NTKO_FACEOP_OK;
	if (frame.empty())
	{
#ifdef NTKO_FACE_SHOW_DEBUG_INFO
		std::cout << __FUNCTION__ << "input arg: frame is EMPTY!" << std::endl;
#endif
		m_opResultCode = NTKO_FACEIOP_ERR_ARG;
		return;
	}

	cv::namedWindow(m_videoWindowName, cv::WINDOW_NORMAL);
	cv::resizeWindow(m_videoWindowName, m_cameraSize.width, m_cameraSize.height);

	//add mask to frame
	addMaskToVideo(frame);

	if (m_isShowInWnd)
	{
#if 0	
		HDC		hDC = NULL;
		RECT	rectWnd;
		long	wndWidth, wndHeight;

		GetClientRect(m_hwndCameraVideo, &rectWnd);
		wndWidth = rectWnd.right - rectWnd.left;
		wndHeight = rectWnd.bottom - rectWnd.top;

		hDC = GetDC(m_hwndCameraVideo);

		uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
		BITMAPINFO* bmi = (BITMAPINFO*)buffer;
		FillBitmapInfo(bmi, wndWidth, wndHeight, frame.channels() * 8, 0);
		StretchDIBits(hDC, 0, 0, wndWidth, wndHeight,
			0, 0, frame.cols, frame.rows, frame.data, bmi,
			DIB_RGB_COLORS, SRCCOPY);
		ReleaseDC(m_hwndCameraVideo, hDC);
#else
		HWND hWnd = (HWND)cvGetWindowHandle(m_videoWindowName.c_str());
		HWND hParent = ::GetParent(hWnd);
		if (hParent != m_hwndCameraVideo)
		{
			::SetParent(hWnd, m_hwndCameraVideo);
			::ShowWindow(hParent, SW_HIDE);
		}
#endif
}
	cv::imshow(m_videoWindowName, frame);
}

void FaceVerification::fillBitmapInfo(BITMAPINFO* bmi, int width, int height, int bpp, int origin)
{
	assert(bmi && width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32));

	BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);

	memset(bmih, 0, sizeof(*bmih));
	bmih->biSize = sizeof(BITMAPINFOHEADER);
	bmih->biWidth = width;
	bmih->biHeight = origin ? abs(height) : -abs(height);
	bmih->biPlanes = 1;
	bmih->biBitCount = (unsigned short)bpp;
	bmih->biCompression = BI_RGB;

	if (bpp == 8)
	{
		RGBQUAD* palette = bmi->bmiColors;
		for (int i = 0; i < 256; i++)
		{
			palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
			palette[i].rgbReserved = 0;
		}
	}
}

bool FaceVerification::fileIsExists(const std::string & fileName)
{
#if 1
	HANDLE	hFile = INVALID_HANDLE_VALUE;

	if (fileName.empty() ||
		(hFile = CreateFileA(fileName.c_str(),
			GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL))
		)
	{
		if (hFile == INVALID_HANDLE_VALUE)
			return false;
		CloseHandle(hFile);
	}
#else
	std::fstream fin;
	if (fileName.empty() || (fin.open(fileName, std::ios::in), !fin))
		return false;
	fin.close();
#endif
	return true;
	}

bool FaceVerification::initilzeVideoCapture(cv::VideoCapture **ppCapture)
{
	cv::VideoCapture *pTempCapture = new cv::VideoCapture(m_cameraID);// capture(m_cameraID);
	if (!pTempCapture->isOpened())
	{
		*ppCapture = NULL;
		return false;
	}
	//pTempCapture->set(CV_CAP_PROP_FRAME_WIDTH, m_cameraSize.width);
	//pTempCapture->set(CV_CAP_PROP_FRAME_HEIGHT, m_cameraSize.height);

	*ppCapture = pTempCapture;
	return true;
}

bool FaceVerification::isTimeoutUseTickCount( unsigned long startTime, unsigned long timeThreshold )
{
	bool isTimeout = false;
	unsigned long dwCurrentCount = GetTickCount();

	if ((dwCurrentCount - startTime) > timeThreshold)
		isTimeout = true;

	return isTimeout;
}

void FaceVerification::setMUIStrFormatForEyeBlinkTips( const std::wstring &strFormat )
{
	if (!strFormat.empty())
	{
		m_strFormatEyeBlinkTips.assign(strFormat);
	}
}

void FaceVerification::setMUIStrFormatForFaceVerifyTips( const std::wstring &strFormat)
{
	if (!strFormat.empty())
	{
		m_strFormatFaceVerifyTips.assign(strFormat);
	}
}

void FaceVerification::setMUIStrFormatForFaceTrainingTips( const std::wstring &strFormat )
{
	if (!strFormat.empty())
	{
		m_strFormatFaceTraining.assign(strFormat);
	}
}

std::string FaceVerification::genLBPHFaceModelFileName()
{
	std::string retFilePath;
	std::string strFileName;

	//Set current system user as train user
	const DWORD	USER_NAME_LENGTH = 256;
	CHAR	szUserName[USER_NAME_LENGTH];
	DWORD	dwLength = USER_NAME_LENGTH;

	ZeroMemory(szUserName, USER_NAME_LENGTH);
	if(GetUserNameA(szUserName, &dwLength))
	{
		char resultKeyBuf[80];

		CSHA1 sha1;
		sha1.Reset();
		sha1.Update((unsigned char*)szUserName, dwLength);
		sha1.Final();

		ZeroMemory((LPVOID)resultKeyBuf, sizeof(resultKeyBuf));
		sha1.ReportHash(resultKeyBuf, CSHA1::REPORT_ASKEY);

		strFileName.assign(resultKeyBuf);
		strFileName += ".xml"; 
	}

	if(strFileName.empty())
		strFileName.assign(DEFAULT_LBPH_MODLE_FILE_NAME);

	retFilePath = getCurrentModulePath() + "\\" + strFileName;
	
	return retFilePath;
}

void FaceVerification::setPredictScoreThreshold( const double score )
{
	if ((score >= PREDICT_SCORE_THRESHOLD_MIN) && 
		(score <= PREDICT_SCORE_THRESHOLD_MAX)
		)
	{
		m_facePredictScoreThreshold = score;
	}
}

double FaceVerification::getPredictScoreThreshold() const
{
	return m_facePredictScoreThreshold;
}
