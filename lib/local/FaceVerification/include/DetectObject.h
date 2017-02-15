#pragma once

#include <cstdio>
#include <iostream>
#include <vector>

// Include OpenCV's C++ Interface
#include "opencv2/opencv.hpp"

class DetectObject
{
public:

// Search for just a single object in the image, such as the largest face, storing the result into 'largestObject'.
// Can use Haar cascades or LBP cascades for Face Detection, or even eye, mouth, or car detection.
// Input is temporarily shrunk to 'scaledWidth' for much faster detection, since 240 is enough to find faces.
// Note: detectLargestObject() should be faster than detectManyObjects().
	void detectLargestObject(const cv::Mat &img, cv::CascadeClassifier &cascade, cv::Rect &largestObject, int scaledWidth = 320);
	void detectObjectsCustom(const cv::Mat &img, cv::CascadeClassifier &cascade, cv::vector<cv::Rect> &objects, int scaledWidth, int flags, cv::Size minFeatureSize, float searchScaleFactor, int minNeighbors);

// Search for many objects in the image, such as all the faces, storing the results into 'objects'.
// Can use Haar cascades or LBP cascades for Face Detection, or even eye, mouth, or car detection.
// Input is temporarily shrunk to 'scaledWidth' for much faster detection, since 240 is enough to find faces.
// Note: detectLargestObject() should be faster than detectManyObjects().
	void detectManyObjects(const cv::Mat &img, cv::CascadeClassifier &cascade, cv::vector<cv::Rect> &objects, int scaledWidth = 320);
};