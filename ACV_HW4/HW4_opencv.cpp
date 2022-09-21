#include <opencv2\opencv.hpp>
#include <string>

using namespace std;
using namespace cv; 

Mat motionEstimation(Mat first_frame, Mat second_frame);
Mat frameReconstruction(vector<Mat> rgbBlocks40_40, vector<Mat> grayBlocks40_40, vector<Mat> grayBlocks80_80, Point frameSize);
Mat denseOpticalFlow(Mat previous_frame, Mat next_frame);
double getSNR(Mat original_frame, Mat reconstruct_frame);

int main() {
	VideoCapture inputVideo = VideoCapture("HW4_video.avi");

	if (!inputVideo.isOpened()) {
		cout << "No video input!";
		return 1;
	}

	Mat previous_frame;
	inputVideo.read(previous_frame);

	int frame_index = 0;
	double clock_start = clock();
	while (inputVideo.isOpened()) {
		Mat next_frame, reconstruct_frame, opticalFlow_frame;
		bool ret = inputVideo.read(next_frame);

		if (ret && frame_index % 30 == 0) {
			reconstruct_frame = motionEstimation(previous_frame, next_frame);
			double SNR = getSNR(next_frame, reconstruct_frame);
			cout << "SNR at " + to_string(frame_index) + "th reconstruct_frame: " << to_string(SNR) << endl;
			//imshow("reconstruct_frame", reconstruct_frame);
			imwrite("reconstructFrame_" + to_string(frame_index) + "th.jpg", reconstruct_frame);

			opticalFlow_frame = denseOpticalFlow(previous_frame, next_frame);
			//imshow("opticalFlow_frame", opticalFlow_frame);
			imwrite("opticalFrame_" + to_string(frame_index) + "th.jpg", opticalFlow_frame);
		}
		if (!ret) {
			double clock_end = clock();
			double computational_time = (clock_end - clock_start) / CLOCKS_PER_SEC;
			cout << "執行時間: " << computational_time << " sec" << endl;
			break;
		}

		if (waitKey(1) == 'q') {
			break;
		}

		previous_frame = next_frame;
		frame_index++;
		
	}
	inputVideo.release();
	return 0;
}

Mat motionEstimation(Mat previous_frame, Mat next_frame) {
	vector<Mat> rgbBlocks40_40, grayBlocks40_40, grayBlocks80_80;
	Mat gray_frame, padding_frame;

	for (int row = 0; row < previous_frame.rows; row += 40) { // template 40*40
		for (int col = 0; col < previous_frame.cols; col += 40) {
			Mat temp40_40(previous_frame, Range(row, row + 40), Range(col, col + 40));
			Mat gray_temp40_40;
			cvtColor(temp40_40, gray_temp40_40, COLOR_BGR2GRAY);
			rgbBlocks40_40.push_back(temp40_40);
			grayBlocks40_40.push_back(gray_temp40_40);
		}
	}

	cvtColor(next_frame, gray_frame, COLOR_BGR2GRAY);
	copyMakeBorder(gray_frame, padding_frame, 20, 20, 20, 20, BORDER_CONSTANT, 0);

	for (int row = 0; row < gray_frame.rows; row += 40) { // 以原本40*40的template往外擴張-20~20的尋找區間
		for (int col = 0; col < gray_frame.cols; col += 40) {
			Mat temp80_80(padding_frame, Range(row, row + 80), Range(col, col + 80));
			grayBlocks80_80.push_back(temp80_80);
		}
	}

	Mat output_frame = frameReconstruction(rgbBlocks40_40, grayBlocks40_40, grayBlocks80_80, gray_frame.size());
	return output_frame;
}

Mat frameReconstruction(vector<Mat> rgbBlocks40_40, vector<Mat> grayBlocks40_40, vector<Mat> grayBlocks80_80, Point frameSize) {
	Mat reconstruct_Image = Mat::ones(760, 1320, CV_8UC3);
	Mat image_matched, block_Image;
	double minVal, maxVal;
	Point minLoc, maxLoc;

	int index = 0;
	for (int row = 0; row < frameSize.y; row += 40) {
		for (int col = 0; col < frameSize.x; col += 40) {
			matchTemplate(grayBlocks80_80[index], grayBlocks40_40[index], image_matched, TM_CCOEFF_NORMED);
			minMaxLoc(image_matched, &minVal, &maxVal, &minLoc, &maxLoc);
			block_Image = reconstruct_Image(Rect(col + maxLoc.y, row + maxLoc.x, 40, 40));
			rgbBlocks40_40[index].copyTo(block_Image);

			index++;
		}
	}

	Mat output_frame = reconstruct_Image(Rect(20, 20, 1280, 720));
	return output_frame;
}

Mat denseOpticalFlow(Mat previous_frame, Mat next_frame) {
	Mat grayPrevious_frame, grayNext_frame;
	Mat flow(previous_frame.size(), CV_32FC2);

	cvtColor(previous_frame, grayPrevious_frame, COLOR_BGR2GRAY);
	cvtColor(next_frame, grayNext_frame, COLOR_BGR2GRAY);
	calcOpticalFlowFarneback(grayPrevious_frame, grayNext_frame, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

	Mat flow_parts[2], magnitude, angle, magnitude_normalize;
	split(flow, flow_parts);
	cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
	normalize(magnitude, magnitude_normalize, 0.0, 1.0, NORM_MINMAX);
	angle *= ((1.0 / 360.0) * (180.0 / 255.0));

	Mat _hsv[3], hsv, hsv8, output_frame;
	_hsv[0] = angle;
	_hsv[1] = Mat::ones(angle.size(), CV_32F);
	_hsv[2] = magnitude_normalize;
	merge(_hsv, 3, hsv);

	hsv.convertTo(hsv8, CV_8U, 255.0);
	cvtColor(hsv8, output_frame, COLOR_HSV2BGR);

	Mat channels[3];
	split(output_frame, channels);
	channels[0] = 255 - channels[0];
	channels[1] = 255 - channels[1];
	channels[2] = 255 - channels[2];
	merge(channels, 3, output_frame);

	return output_frame;
}

double getSNR(Mat original_frame, Mat reconstruct_frame) {
	int channels = original_frame.channels();
	double sigma = 0.0;
	double mse = 0.0;
	
	for (int row = 0; row < original_frame.rows; row++) {
		for (int col = 0; col < original_frame.cols * channels; col++) {
			sigma += pow(original_frame.ptr<uchar>(row)[col], 2);
			mse += pow(original_frame.ptr<uchar>(row)[col] - reconstruct_frame.ptr<uchar>(row)[col], 2);
		}
	}
	double SNR = 20 * log10(sigma / mse);

	return SNR;
}

