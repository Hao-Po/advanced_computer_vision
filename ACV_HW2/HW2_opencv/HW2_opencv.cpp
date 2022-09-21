#include <opencv2\opencv.hpp>+
#include <vector>

using namespace cv;
using namespace std;


Mat bmpRead(string imageName) {
	Mat inputImage = imread(imageName); // 利用opencv的imread函數讀圖

	return inputImage;
}

void bmpWrite(char const* pictureName, Mat OutputImage) {
	imwrite(pictureName, OutputImage); // 利用opencv的imwrite函數,輸出圖片
}

Mat rgb2gray(Mat inputImage) {
	Mat grayImage;
	cvtColor(inputImage, grayImage, COLOR_BGR2GRAY); // 利用opencv的cvtColor函數,把RGB圖轉為灰階

	return grayImage;
}

Mat gray2binary(Mat inputImage) {
	Mat binaryImage;
	threshold(inputImage, binaryImage, 220, 255, THRESH_BINARY_INV); // 利用opencv的threshold函數,將灰階圖二值化,threshlod值設為220

	return binaryImage;
}

Mat oneChannelstoThreeChannels(Mat inputImage) { 
	Mat inputImage_3Channels;

	vector<Mat> threeChannels; // 宣告一個Mat的vector,再將三層一樣的Mat merge在一起,使one Channel到three Channels
	threeChannels.push_back(inputImage);
	threeChannels.push_back(inputImage);
	threeChannels.push_back(inputImage);
	merge(threeChannels, inputImage_3Channels);

	return inputImage_3Channels;
}

Mat dilation(Mat inputImage) {
	Mat dilationImage; // 先宣告一個filter, 再利用opencv的dilate函數做dilate
	Mat filter = (Mat_<unsigned char>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0); // cross filter
	dilate(inputImage, dilationImage, filter);

	return dilationImage;
}

Mat connectedComponent_self (Mat inputImage) { // 數值很小 所以看起來很黑
	Mat labelImage, stats, centroid; // stats存有boundingbox所需的邊框值, 每個區域的area, centroid存有每個區域的質心座標
	int num_labels = connectedComponentsWithStats(inputImage, labelImage, stats, centroid, 4); // 利用opencv的connectedComponentsWithStats直接產生
	
	cout << "Arae & Centroid: (圖片手中的紅點為centroid)" << endl;
	for (int row = 1; row < centroid.rows; row++) {
		cout << "Part " << row << " ----> area: " << stats.at<int>(row, 4) << ", centroid: (";;
		cout << int(centroid.at<double>(row, 0)) << ", " << int(centroid.at<double>(row, 1)) << ")" << endl;
	}cout << endl;
	
	return labelImage;
}

void properties_analysis(int width, int height, int index) {
	int longest = (width > height) ? width : height;
	string orientation = (width > height) ? "left to right" : "up to down";
	
	cout << "Part " << index << " ----> length: " << longest << ", it is " << orientation << endl;
}

Mat bounding_box(Mat inputImage) {
	Mat labelImage, drawingImage, stats, centroid;
	int num_labels = connectedComponentsWithStats(inputImage, labelImage, stats, centroid, 4);

	drawingImage = oneChannelstoThreeChannels(inputImage); // 由於要在圖片上畫紅框, 先將one Channel的圖轉為three Channels

	cout << "Properties Analysis:" << endl;
	for (int row = 1; row < stats.rows; row++) { // Rect(x, y, width, height) 利用Rect把方框座標存取起來,再利用opencv的rectangle在圖上繪出
		Rect rect_box = Rect(stats.at<int>(row, 0), stats.at<int>(row, 1), stats.at<int>(row, 2), stats.at<int>(row, 3));
		rectangle(drawingImage, rect_box, Scalar(0, 0, 255), 1, 1, 0); // rectangle(outputImage, coordinate, color, 線的粗細, 線的type, shift)

		properties_analysis(stats.at<int>(row, 2), stats.at<int>(row, 3), row); // 用來判斷longest跟orientation

		// 畫質心
		Rect rect_centroid = Rect(int(centroid.at<double>(row, 0)), int(centroid.at<double>(row, 1)), 1, 1);
		rectangle(drawingImage, rect_centroid, Scalar(0, 0, 255), 1, 1, 0); 
	}

	return drawingImage;
}


int main() {
	Mat inputImage; // 宣告傳入圖片的空間
	Mat grayImage; // 灰階圖片
	Mat binaryImage; // 二值化圖片
	Mat dilationImage; // dilation圖片
	Mat labelImage; // 存放label的圖片
	Mat drawingImage; // 用來畫紅框、質心的圖片

	inputImage = bmpRead("hand.bmp");
	
	/*binarizing*/
	double binarizing_start = clock();
	grayImage = rgb2gray(inputImage);
	binaryImage = gray2binary(grayImage);
	double binarizing_end = clock();
	double binarizing_time = (binarizing_end - binarizing_start) / CLOCKS_PER_SEC;

	/*morphology*/
	double morphology_start = clock();
	dilationImage = dilation(dilation(binaryImage));
	double morphology_end = clock();
	double morphology_time = (morphology_end - morphology_start) / CLOCKS_PER_SEC;

	/*connected component + centroid + area*/
	double connectedComponent_start = clock();
	labelImage = connectedComponent_self(dilationImage); // cv::connneted conponent can only use one channel image
	double connectedComponent_end = clock();
	double connectedComponent_time = (connectedComponent_end - connectedComponent_start) / CLOCKS_PER_SEC;

	/*bounding box(drawing) + properties analyisis*/
	double boundingBox_start = clock();
	drawingImage = bounding_box(dilationImage);
	double boundingBox_end = clock();
	double boundingBox_time = (boundingBox_end - boundingBox_start) / CLOCKS_PER_SEC;

	cout << endl << "╔================================================================================================╗";
	cout << endl << "∥" << setw(10) << "Time(sec.)" << " ∥ " << setw(10) << "binarizing" << " ∥ " << setw(10) << "morphology" << " ∥ ";
	cout << setw(19) << "connected component" << " ∥ " << setw(19) << "properties analysis" << " ∥ " << setw(7) << "drawing" << "∥";
	cout << endl << "∥" << setw(10) << "OpenCV" << " ∥ " << setw(10) << binarizing_time << " ∥ " << setw(10) << morphology_time << " ∥ ";
	cout << setw(19) << connectedComponent_time << " ∥ " << setw(19) << connectedComponent_time << " ∥ " << setw(7) << boundingBox_time << "∥";
	cout << endl << "╚================================================================================================╝" << endl;

	//bmpWrite("grayImage.bmp", grayImage);
	bmpWrite("binaryImage.bmp", binaryImage);
	//bmpWrite("dilationImage.bmp", dilationImage);
	//bmpWrite("labelImage.bmp", labelImage);
	bmpWrite("drawingImage.bmp", drawingImage);

	system("pause");
	return 0;
}