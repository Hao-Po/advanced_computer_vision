#include <opencv2\opencv.hpp>
#include <vector>
#define pi 3.1415926

using namespace std;
using namespace cv;

double alpha = 15 * pi / 180;
double dx = 4;
double dy = -10;
double dz = 5;
double gamma_zero = 0;
double theta_zero = 0.025;

Mat bmpRead(string imageName) {
	Mat inputImage = imread(imageName);

	try {
		if (!inputImage.data) { throw "File is NULL"; }
	}catch (const char* msg) {
		cerr << msg << endl;
		exit(0);
	}
	
	return inputImage;
}

void bmpWrite(char const* pictureName, Mat OutputImage) {
	imwrite(pictureName, OutputImage); 
}

Mat rgb2gray(Mat inputImage) {
	Mat grayImage;
	cvtColor(inputImage, grayImage, COLOR_BGR2GRAY); 

	return grayImage;
}

Mat oneChannelstoThreeChannels(Mat inputImage) {
	Mat inputImage_3Channels;

	vector<Mat> threeChannels;
	threeChannels.push_back(inputImage);
	threeChannels.push_back(inputImage);
	threeChannels.push_back(inputImage);
	merge(threeChannels, inputImage_3Channels);

	return inputImage_3Channels;
}

Mat LabelLines(Mat inputImage, Mat grayImage) {
	Mat blurImage;
	Mat edgeImage;

	GaussianBlur(grayImage, blurImage, cv::Size(3, 3), 15, 0); 
	Canny(blurImage, edgeImage, 70, 255); 

	vector<vector<Point>> contour; 
	vector<Point> points;
	points.push_back(Point(0, 552));
	points.push_back(Point(514, 346));
	points.push_back(Point(537, 346));
	points.push_back(Point(887, 767));
	points.push_back(Point(1023, 767));
	points.push_back(Point(1023, 0));
	points.push_back(Point(0, 0));
	contour.push_back(points);
	fillPoly(edgeImage, contour, Scalar(0, 0, 0));

	vector<Vec4i> lines;
	HoughLinesP(edgeImage, lines, 1, CV_PI / 180, 50, 50, 10);

	double x_difference, y_difference, m;
	vector<Vec2i> blue_points, red_points, green_points;
	
	for (int index = 0; index < lines.size(); index++) {
		x_difference = lines[index][1] - lines[index][3];
		y_difference = lines[index][0] - lines[index][2];
		if (x_difference != 0) {
			m = y_difference / x_difference;
		}

		if (m < -2.3) { // blue
			line(inputImage, Point(lines[index][0], lines[index][1]), Point(lines[index][2], lines[index][3]), Scalar(255, 0, 0), 8);
		}
		if (-0.9 < m && m < -0.7) { // red
			line(inputImage, Point(lines[index][0], lines[index][1]), Point(lines[index][2], lines[index][3]), Scalar(0, 0, 255), 20);
		}
		if (m > 0.8) { // green
			line(inputImage, Point(lines[index][0], lines[index][1]), Point(lines[index][2], lines[index][3]), Scalar(0, 255, 0), 35);
		}
	}
	
	return inputImage;
}

Mat warping(Mat inputImage, int width, int height) {
	Mat outputImage;
	
	double uHorizon = (height - 1) / (2 * alpha) * (-1 * theta_zero + alpha);
	Point2f src_points[] = { Point2f(0, uHorizon),Point2f(1023,uHorizon),Point2f(0,767),Point2f(1023,767) };
	Point2f dst_points[] = { Point2f(1023,0),Point2f(1023,767),Point2f(0,380),Point2f(0,390) };
	Mat M = getPerspectiveTransform(src_points, dst_points);
	warpPerspective(inputImage, outputImage, M, inputImage.size(), INTER_LINEAR);

	return outputImage;
}

Mat crop(Mat inputImage) {
	Mat outputImage;
	Rect crop_region(0, 347, 74, 74);
	outputImage = inputImage(crop_region);

	return outputImage;
}

int main() {
	Mat roadImage;
	Mat grayImage;
	Mat labelImage;
	Mat warpImage_normal;
	Mat cropImage_normal;
	Mat warpImage_label;
	Mat cropImage_label;
	int img_width = 1024, img_height = 768;

	roadImage = bmpRead("road.bmp");
	warpImage_normal = warping(roadImage, img_width, img_height);
	cropImage_normal = crop(warpImage_normal);
	grayImage = rgb2gray(roadImage);
	labelImage = LabelLines(roadImage, grayImage);
	warpImage_label = warping(labelImage, img_width, img_height);
	cropImage_label = crop(warpImage_label);

	// bmpWrite("labelImage.bmp", labelImage);
	// bmpWrite("warpImage_normal.bmp", warpImage_normal);
	bmpWrite("cropImage_normal.bmp", cropImage_normal);
	// bmpWrite("warpImage_label.bmp", warpImage_label);
	bmpWrite("cropImage_label.bmp", cropImage_label);
}