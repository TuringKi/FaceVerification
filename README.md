## 人脸验证

> 通过从摄像头获取用户面部图片，处理图片并通过眨眼确定图片为真人。采用分类级联器训练采集到的数据生成训练集.
> 以后通过摄像头验证用户是否可登录系统。

****

### 功能点：

1. 用户配置信息

2. 人脸验证


	void trainFaceImage(const std::string &user_name, bool replace = false)
	
	void faceVerifier()

	bool predict(const cv::Mat &image)
    


###项目依赖相关库
- libfacedetection开源库:[https://github.com/ShiqiYu/libfacedetechtion](https://github.com/ShiqiYu/libfacedetechtion "libfacedetection")
> 
libfacedetect.lib

- OpenCV开源库：
> 
opencv_contrib2413d.lib
opencv_highgui2413d.lib
opencv_core2413d.lib
opencv_imgproc2413d.lib
opencv_objdetect2413d.lib
opencv_video2413d.lib
opencv_ml2413d.lib
opencv_calib3d2413d.lib
opencv_features2d2413d.lib
opencv_flann2413d.lib
opencv_gpu2413d.lib

###目录结构

#### lib 库文件目录

> 3rdParty
	- libfacedetection	用于快速识别人面部定位
	- OpenCV2.4.13		OpenCV 2.4.13版本的头文件和lib库
	
> local
 	- FaceVerification 封装库用于从摄像头训练面部数据和验证面部

#### exe 测试项目目录

> Test_faceVerification 命令行测试程序
> Test_Win32  窗口测试程序
