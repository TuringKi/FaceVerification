#pragma once
#include <string>

const	int		FACEVERIFIER_FACE_WIDTH		= 120;
const	int		FACEVERIFIER_FACE_HEIGHT	= 120;
const	int		DETECTION_WIDTH				= 320;

const	int		FACEVERIFIER_SUCCEED_COUNT_THRESHOLD	= 5;	//the threshold count of face verification 

const	int		DEFAULT_TRAINING_IMAGE_COUNT			= 60;
const	int		DEFAULT_EYE_BLINK_MAX_COUNT				= 3;
const	unsigned long	DEFAULT_EYE_BLINK_TIME_OUT		= 10 * 1000;	//default 10s
const	unsigned long	DEFAULT_VERIFIER_TIME_OUT		= 30 * 1000;	//default 30s 

const	double  DESIRED_LEFT_EYE_X			= 0.19;		// Controls how much of the face is visible after preprocessing.
const	double  DESIRED_LEFT_EYE_Y			= 0.17;
const	double	FACE_ELLIPSE_CY				= 0.40;
const	double	FACE_ELLIPSE_W				= 0.50;         
const	double	FACE_ELLIPSE_H				= 0.80;		// Controls how tall the face mask is.

const	double	DEFAULT_PREDICT_SCORE_THRESHOLD	= 55.0;	//the threshold of predict score, for LBPH below 55.0 is good
const	double	PREDICT_SCORE_THRESHOLD_MIN	= 30.0;		//the MIN threshold of predict score
const	double	PREDICT_SCORE_THRESHOLD_MAX = 70.0;		//the MAX threshold of predict score	

const	double	FACE_MASK_WEIGHTED_VALUE	= 0.94;		//add mask ellipse in face pos use addWeighted alpha value
const	double	FACE_MASK_ELLIPSE_W			= 0.20;         
const	double	FACE_MASK_ELLIPSE_H			= 0.35;		// Controls how tall the face mask is.

const	std::string DEFAULT_LBPH_MODLE_FILE_NAME	= "LBPH_TRIANED_MODLE.xml";


//////////////////////////////////////////////////////////////////////////
//
//
#define NTKO_FACE_SHOW_DEBUG_INFO
#define	NTKO_FACE_USE_SIGNAL_PERSON_MODE