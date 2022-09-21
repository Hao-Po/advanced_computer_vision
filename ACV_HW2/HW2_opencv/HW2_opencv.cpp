#include <opencv2\opencv.hpp>+
#include <vector>

using namespace cv;
using namespace std;


Mat bmpRead(string imageName) {
	Mat inputImage = imread(imageName); // �Q��opencv��imread���Ū��

	return inputImage;
}

void bmpWrite(char const* pictureName, Mat OutputImage) {
	imwrite(pictureName, OutputImage); // �Q��opencv��imwrite���,��X�Ϥ�
}

Mat rgb2gray(Mat inputImage) {
	Mat grayImage;
	cvtColor(inputImage, grayImage, COLOR_BGR2GRAY); // �Q��opencv��cvtColor���,��RGB���ର�Ƕ�

	return grayImage;
}

Mat gray2binary(Mat inputImage) {
	Mat binaryImage;
	threshold(inputImage, binaryImage, 220, 255, THRESH_BINARY_INV); // �Q��opencv��threshold���,�N�Ƕ��ϤG�Ȥ�,threshlod�ȳ]��220

	return binaryImage;
}

Mat oneChannelstoThreeChannels(Mat inputImage) { 
	Mat inputImage_3Channels;

	vector<Mat> threeChannels; // �ŧi�@��Mat��vector,�A�N�T�h�@�˪�Mat merge�b�@�_,��one Channel��three Channels
	threeChannels.push_back(inputImage);
	threeChannels.push_back(inputImage);
	threeChannels.push_back(inputImage);
	merge(threeChannels, inputImage_3Channels);

	return inputImage_3Channels;
}

Mat dilation(Mat inputImage) {
	Mat dilationImage; // ���ŧi�@��filter, �A�Q��opencv��dilate��ư�dilate
	Mat filter = (Mat_<unsigned char>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0); // cross filter
	dilate(inputImage, dilationImage, filter);

	return dilationImage;
}

Mat connectedComponent_self (Mat inputImage) { // �ƭȫܤp �ҥH�ݰ_�ӫܶ�
	Mat labelImage, stats, centroid; // stats�s��boundingbox�һݪ���ح�, �C�Ӱϰ쪺area, centroid�s���C�Ӱϰ쪺��߮y��
	int num_labels = connectedComponentsWithStats(inputImage, labelImage, stats, centroid, 4); // �Q��opencv��connectedComponentsWithStats��������
	
	cout << "Arae & Centroid: (�Ϥ��⤤�����I��centroid)" << endl;
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

	drawingImage = oneChannelstoThreeChannels(inputImage); // �ѩ�n�b�Ϥ��W�e����, ���None Channel�����ରthree Channels

	cout << "Properties Analysis:" << endl;
	for (int row = 1; row < stats.rows; row++) { // Rect(x, y, width, height) �Q��Rect���خy�Цs���_��,�A�Q��opencv��rectangle�b�ϤWø�X
		Rect rect_box = Rect(stats.at<int>(row, 0), stats.at<int>(row, 1), stats.at<int>(row, 2), stats.at<int>(row, 3));
		rectangle(drawingImage, rect_box, Scalar(0, 0, 255), 1, 1, 0); // rectangle(outputImage, coordinate, color, �u���ʲ�, �u��type, shift)

		properties_analysis(stats.at<int>(row, 2), stats.at<int>(row, 3), row); // �ΨӧP�_longest��orientation

		// �e���
		Rect rect_centroid = Rect(int(centroid.at<double>(row, 0)), int(centroid.at<double>(row, 1)), 1, 1);
		rectangle(drawingImage, rect_centroid, Scalar(0, 0, 255), 1, 1, 0); 
	}

	return drawingImage;
}


int main() {
	Mat inputImage; // �ŧi�ǤJ�Ϥ����Ŷ�
	Mat grayImage; // �Ƕ��Ϥ�
	Mat binaryImage; // �G�ȤƹϤ�
	Mat dilationImage; // dilation�Ϥ�
	Mat labelImage; // �s��label���Ϥ�
	Mat drawingImage; // �Ψӵe���ءB��ߪ��Ϥ�

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

	cout << endl << "��================================================================================================��";
	cout << endl << "��" << setw(10) << "Time(sec.)" << " �� " << setw(10) << "binarizing" << " �� " << setw(10) << "morphology" << " �� ";
	cout << setw(19) << "connected component" << " �� " << setw(19) << "properties analysis" << " �� " << setw(7) << "drawing" << "��";
	cout << endl << "��" << setw(10) << "OpenCV" << " �� " << setw(10) << binarizing_time << " �� " << setw(10) << morphology_time << " �� ";
	cout << setw(19) << connectedComponent_time << " �� " << setw(19) << connectedComponent_time << " �� " << setw(7) << boundingBox_time << "��";
	cout << endl << "��================================================================================================��" << endl;

	//bmpWrite("grayImage.bmp", grayImage);
	bmpWrite("binaryImage.bmp", binaryImage);
	//bmpWrite("dilationImage.bmp", dilationImage);
	//bmpWrite("labelImage.bmp", labelImage);
	bmpWrite("drawingImage.bmp", drawingImage);

	system("pause");
	return 0;
}