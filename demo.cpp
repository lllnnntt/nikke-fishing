#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <Windows.h>
using namespace cv;
using namespace std;

// 按下一个键
void PressKey(WORD key) {
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// 按下键
	ip.ki.wVk = key; // virtual-key code
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// 松开键
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
}

// 模板匹配
bool CVFindIMage(Mat& Image1, Mat& Image2) {
	if (Image2.rows > Image1.rows || Image2.cols > Image1.cols) {
		return false;
	}
	Mat resultImage;
	// 进行模板匹配
	cv::matchTemplate(Image1, Image2, resultImage, cv::TM_CCOEFF_NORMED);
	// 设置阈值
	double threshold = 0.7;
	// 寻找大于阈值的最匹配位置
	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(resultImage, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	// 过滤掉低于阈值的匹配结果
	if (maxVal >= threshold) {
		return true;
	}
	else {
		return false;
	}
}

// 获取句柄窗口坐标
RECT GetClientRect(HWND hwnd) {
	RECT rect;
	if (GetClientRect(hwnd, &rect)) {
		POINT topLeft = { rect.left, rect.top };
		// 将工作区左上角的坐标转换为屏幕坐标
		ClientToScreen(hwnd, &topLeft);
		rect.left = topLeft.x;
		rect.top = topLeft.y;

		POINT bottomRight = { rect.right, rect.bottom };
		// 将工作区右下角的坐标转换为屏幕坐标
		ClientToScreen(hwnd, &bottomRight);
		rect.right = bottomRight.x;
		rect.bottom = bottomRight.y;
	}
	return rect;
}

// 获取窗口句柄的工作区大小
void GetClientAreaSize(HWND hwnd, int& width, int& height) {
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect)) {
		width = clientRect.right - clientRect.left;
		height = clientRect.bottom - clientRect.top;
	}
	else {
		std::cerr << "Failed to get client rect." << std::endl;
		width = 0;
		height = 0;
	}
}

// 获取桌面
cv::Mat Getscreen(HDC hScreenDC, HWND hWnd, RECT rect2) {
	int width = static_cast<int>(720 / 1.5);
	int height = static_cast<int>(1280 / 1.5);

	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	SelectObject(hMemoryDC, hBitmap);

	// 复制屏幕到位图
	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, rect2.left, rect2.top, SRCCOPY);

	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height; // 负数表示从上到下的图像
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	BITMAPINFO bmi;
	bmi.bmiHeader = bi;

	void* pBits = nullptr;
	HBITMAP hDIB = CreateDIBSection(hScreenDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

	if (hDIB == NULL) {
		std::cerr << "Error creating DIB section." << std::endl;
		DeleteObject(hBitmap);
		DeleteDC(hMemoryDC);
		ReleaseDC(NULL, hScreenDC);
		return cv::Mat();
	}

	GetDIBits(hMemoryDC, hBitmap, 0, height, pBits, &bmi, DIB_RGB_COLORS);

	cv::Mat screen(height, width, CV_8UC4, pBits);

	// 转换为三通道图像（BGR）
	cv::Mat screenBGR;
	cv::cvtColor(screen, screenBGR, cv::COLOR_BGRA2BGR);

	// 清理资源
	DeleteObject(hDIB);
	DeleteObject(hBitmap);
	DeleteDC(hMemoryDC);
	ReleaseDC(NULL, hScreenDC);

	return screenBGR;
}

// 裁剪图像
Mat GetRangescreen(Mat screen, int x1, int y1, int x2, int y2) {
	cv::Rect roi(x1, y1, int(x2 - x1), int(y2 - y1));
	// 截取区域
	cv::Mat croppedImage = screen(roi);
	return croppedImage;
}


int main() {
	// 获取游戏窗口的句柄
	HWND GhWnd = FindWindow(L"UnityWndClass", L"NIKKE");
	if (GhWnd == NULL) {
		MessageBox(NULL, (L"没有找到句柄"), (L"错误"), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		exit(0);
	}
	// 获取游戏窗口坐标
	RECT rect2 = GetClientRect(GhWnd);
	std::cout << "游戏窗口坐标: " << "x: " << rect2.left << "y: " << rect2.top << std::endl;

	//获取桌面句柄
	HWND hWnd = GetDesktopWindow();

	//蓝色箭头
	cv::Mat bleft = cv::imread("./imgae/bleft.png");
	cv::Mat bright = cv::imread("./imgae/bright.png");
	cv::Mat bup = cv::imread("./imgae/bup.png");
	cv::Mat bdown = cv::imread("./imgae/bdown.png");
	//黄色箭头
	cv::Mat yleft = cv::imread("./imgae/yleft.png");
	cv::Mat yright = cv::imread("./imgae/yright.png");
	cv::Mat yup = cv::imread("./imgae/yup.png");
	cv::Mat ydown = cv::imread("./imgae/ydown.png");



	while (true)
	{	// 获取设备上下文
		HDC hScreenDC = GetDC(hWnd);
		Mat screen = Getscreen(hScreenDC, hWnd, rect2);
		Mat croppedImage = GetRangescreen(screen, 381, 202, 440, 258);

		if (CVFindIMage(croppedImage, bleft) || CVFindIMage(croppedImage, yleft)) {
			PressKey(0x25);
			printf("左\n");
		}
		else if (CVFindIMage(croppedImage, bright) || CVFindIMage(croppedImage, yright)) {
			PressKey(0x27);
			printf("右\n");
		}
		else if (CVFindIMage(croppedImage, bup) || CVFindIMage(croppedImage, yup)) {
			PressKey(0x26);
			printf("上\n");
		}
		else if (CVFindIMage(croppedImage, bdown)|| CVFindIMage(croppedImage, ydown)) {
			PressKey(0x28);
			printf("下\n");
		}

	}
	return 0;
}

