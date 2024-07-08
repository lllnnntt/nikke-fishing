#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <Windows.h>
using namespace cv;
using namespace std;

// ����һ����
void PressKey(WORD key) {
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// ���¼�
	ip.ki.wVk = key; // virtual-key code
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// �ɿ���
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
}

// ģ��ƥ��
bool CVFindIMage(Mat& Image1, Mat& Image2) {
	if (Image2.rows > Image1.rows || Image2.cols > Image1.cols) {
		return false;
	}
	Mat resultImage;
	// ����ģ��ƥ��
	cv::matchTemplate(Image1, Image2, resultImage, cv::TM_CCOEFF_NORMED);
	// ������ֵ
	double threshold = 0.7;
	// Ѱ�Ҵ�����ֵ����ƥ��λ��
	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(resultImage, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	// ���˵�������ֵ��ƥ����
	if (maxVal >= threshold) {
		return true;
	}
	else {
		return false;
	}
}

// ��ȡ�����������
RECT GetClientRect(HWND hwnd) {
	RECT rect;
	if (GetClientRect(hwnd, &rect)) {
		POINT topLeft = { rect.left, rect.top };
		// �����������Ͻǵ�����ת��Ϊ��Ļ����
		ClientToScreen(hwnd, &topLeft);
		rect.left = topLeft.x;
		rect.top = topLeft.y;

		POINT bottomRight = { rect.right, rect.bottom };
		// �����������½ǵ�����ת��Ϊ��Ļ����
		ClientToScreen(hwnd, &bottomRight);
		rect.right = bottomRight.x;
		rect.bottom = bottomRight.y;
	}
	return rect;
}

// ��ȡ���ھ���Ĺ�������С
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

// ��ȡ����
cv::Mat Getscreen(HDC hScreenDC, HWND hWnd, RECT rect2) {
	int width = static_cast<int>(720 / 1.5);
	int height = static_cast<int>(1280 / 1.5);

	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	SelectObject(hMemoryDC, hBitmap);

	// ������Ļ��λͼ
	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, rect2.left, rect2.top, SRCCOPY);

	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height; // ������ʾ���ϵ��µ�ͼ��
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

	// ת��Ϊ��ͨ��ͼ��BGR��
	cv::Mat screenBGR;
	cv::cvtColor(screen, screenBGR, cv::COLOR_BGRA2BGR);

	// ������Դ
	DeleteObject(hDIB);
	DeleteObject(hBitmap);
	DeleteDC(hMemoryDC);
	ReleaseDC(NULL, hScreenDC);

	return screenBGR;
}

// �ü�ͼ��
Mat GetRangescreen(Mat screen, int x1, int y1, int x2, int y2) {
	cv::Rect roi(x1, y1, int(x2 - x1), int(y2 - y1));
	// ��ȡ����
	cv::Mat croppedImage = screen(roi);
	return croppedImage;
}


int main() {
	// ��ȡ��Ϸ���ڵľ��
	HWND GhWnd = FindWindow(L"UnityWndClass", L"NIKKE");
	if (GhWnd == NULL) {
		MessageBox(NULL, (L"û���ҵ����"), (L"����"), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		exit(0);
	}
	// ��ȡ��Ϸ��������
	RECT rect2 = GetClientRect(GhWnd);
	std::cout << "��Ϸ��������: " << "x: " << rect2.left << "y: " << rect2.top << std::endl;

	//��ȡ������
	HWND hWnd = GetDesktopWindow();

	//��ɫ��ͷ
	cv::Mat bleft = cv::imread("./imgae/bleft.png");
	cv::Mat bright = cv::imread("./imgae/bright.png");
	cv::Mat bup = cv::imread("./imgae/bup.png");
	cv::Mat bdown = cv::imread("./imgae/bdown.png");
	//��ɫ��ͷ
	cv::Mat yleft = cv::imread("./imgae/yleft.png");
	cv::Mat yright = cv::imread("./imgae/yright.png");
	cv::Mat yup = cv::imread("./imgae/yup.png");
	cv::Mat ydown = cv::imread("./imgae/ydown.png");



	while (true)
	{	// ��ȡ�豸������
		HDC hScreenDC = GetDC(hWnd);
		Mat screen = Getscreen(hScreenDC, hWnd, rect2);
		Mat croppedImage = GetRangescreen(screen, 381, 202, 440, 258);

		if (CVFindIMage(croppedImage, bleft) || CVFindIMage(croppedImage, yleft)) {
			PressKey(0x25);
			printf("��\n");
		}
		else if (CVFindIMage(croppedImage, bright) || CVFindIMage(croppedImage, yright)) {
			PressKey(0x27);
			printf("��\n");
		}
		else if (CVFindIMage(croppedImage, bup) || CVFindIMage(croppedImage, yup)) {
			PressKey(0x26);
			printf("��\n");
		}
		else if (CVFindIMage(croppedImage, bdown)|| CVFindIMage(croppedImage, ydown)) {
			PressKey(0x28);
			printf("��\n");
		}

	}
	return 0;
}

