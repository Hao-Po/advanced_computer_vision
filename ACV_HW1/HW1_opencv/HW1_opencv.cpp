#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;

Mat inputImage; // 宣告傳入圖片的空間
Mat inputImage_Transpose; // 宣告圖片經過轉置後的空間
Mat image_Rotated; // 宣告圖片順時針旋轉90度(轉置後垂直鏡像)的空間
vector<Mat> BGR_channels; // 宣告把圖片拆分成BGR三層的空間
Mat red_Image; // 宣告RedImage的空間
Mat green_Image; // 宣告GreenImage的空間
Mat blue_Image; // 宣告BlueImage的空間
Mat doubleSize_Image; // 宣告DoubleSizeImage的空間
Mat oneHalfSize_Image; // 宣告OneHalfSizeImage的空間

void bmpRead() {
	inputImage = imread("InputImage.bmp"); // 利用opencv的imread函數讀圖
}

void bmpWrite(char const* pictureName, Mat OutputImage) {
	imwrite(pictureName, OutputImage); // 利用opencv的imwrite函數,輸出圖片
}

void ImageRotation() {
	transpose(inputImage, inputImage_Transpose); // 利用opencv的transpose函數,把圖片經過一次轉置
	flip(inputImage_Transpose, image_Rotated, 1); // 再經由opencv的flip函數,對轉置後的圖片做垂直鏡像,等同順時針旋轉90度
}

void channelSeparation(){
	split(image_Rotated, BGR_channels); // 將旋轉後的圖片拆分成BGR三層,放到BGR_channels[0],BGR_channels[1],BGR_channels[2]
	Mat zeroChannel = Mat::zeros(Size(image_Rotated.cols, image_Rotated.rows), CV_8UC1); // 製作一層純為0的channel,可以跟BGR單層合併,變成只有單一色的RGB圖

	vector<Mat>MergeImageChannels; // 宣告儲存用來合併BGR三層的channels
	MergeImageChannels.push_back(BGR_channels[0]); // 製作blueImage,依序放入BGR三層的data
	MergeImageChannels .push_back(zeroChannel);
	MergeImageChannels.push_back(zeroChannel);
	merge(MergeImageChannels, blue_Image);
	MergeImageChannels.clear(); // 清空channels以便下次使用不用宣告新的

	MergeImageChannels.push_back(zeroChannel); // 製作greenImage,依序放入BGR三層的data
	MergeImageChannels.push_back(BGR_channels[1]);
	MergeImageChannels.push_back(zeroChannel);
	merge(MergeImageChannels, green_Image);
	MergeImageChannels.clear();

	MergeImageChannels.push_back(zeroChannel); // 製作RedImage,依序放入BGR三層的data
	MergeImageChannels.push_back(zeroChannel);
	MergeImageChannels.push_back(BGR_channels[2]);
	merge(MergeImageChannels, red_Image);
	MergeImageChannels.clear();
}

void double_Size() {
	resize(image_Rotated, doubleSize_Image, Size((image_Rotated.cols) * 2, (image_Rotated.rows) * 2)); // 利用opencv的resize函數將旋轉後的圖片放大兩倍
}

void one_half_Size() {
	resize(image_Rotated, oneHalfSize_Image, Size((image_Rotated.cols) / 2, (image_Rotated.rows) / 2));// 利用opencv的resize函數將旋轉後的圖片縮小為1/2倍
}

int main() {
	bmpRead(); // 呼叫以上寫的Function完成作業要求
	ImageRotation();
	channelSeparation();
	double_Size();
	one_half_Size();
	bmpWrite("Rotated.bmp", image_Rotated);
	bmpWrite("BlueImage.bmp", blue_Image);
	bmpWrite("GreenImage.bmp", green_Image);
	bmpWrite("RedImage.bmp", red_Image);
	bmpWrite("doubleSize.bmp", doubleSize_Image);
	bmpWrite("oneHalfSize.bmp", oneHalfSize_Image);
}