#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;

Mat inputImage; // �ŧi�ǤJ�Ϥ����Ŷ�
Mat inputImage_Transpose; // �ŧi�Ϥ��g�L��m�᪺�Ŷ�
Mat image_Rotated; // �ŧi�Ϥ����ɰw����90��(��m�᫫���蹳)���Ŷ�
vector<Mat> BGR_channels; // �ŧi��Ϥ������BGR�T�h���Ŷ�
Mat red_Image; // �ŧiRedImage���Ŷ�
Mat green_Image; // �ŧiGreenImage���Ŷ�
Mat blue_Image; // �ŧiBlueImage���Ŷ�
Mat doubleSize_Image; // �ŧiDoubleSizeImage���Ŷ�
Mat oneHalfSize_Image; // �ŧiOneHalfSizeImage���Ŷ�

void bmpRead() {
	inputImage = imread("InputImage.bmp"); // �Q��opencv��imread���Ū��
}

void bmpWrite(char const* pictureName, Mat OutputImage) {
	imwrite(pictureName, OutputImage); // �Q��opencv��imwrite���,��X�Ϥ�
}

void ImageRotation() {
	transpose(inputImage, inputImage_Transpose); // �Q��opencv��transpose���,��Ϥ��g�L�@����m
	flip(inputImage_Transpose, image_Rotated, 1); // �A�g��opencv��flip���,����m�᪺�Ϥ��������蹳,���P���ɰw����90��
}

void channelSeparation(){
	split(image_Rotated, BGR_channels); // �N����᪺�Ϥ������BGR�T�h,���BGR_channels[0],BGR_channels[1],BGR_channels[2]
	Mat zeroChannel = Mat::zeros(Size(image_Rotated.cols, image_Rotated.rows), CV_8UC1); // �s�@�@�h�¬�0��channel,�i�H��BGR��h�X��,�ܦ��u����@�⪺RGB��

	vector<Mat>MergeImageChannels; // �ŧi�x�s�ΨӦX��BGR�T�h��channels
	MergeImageChannels.push_back(BGR_channels[0]); // �s�@blueImage,�̧ǩ�JBGR�T�h��data
	MergeImageChannels .push_back(zeroChannel);
	MergeImageChannels.push_back(zeroChannel);
	merge(MergeImageChannels, blue_Image);
	MergeImageChannels.clear(); // �M��channels�H�K�U���ϥΤ��Ϋŧi�s��

	MergeImageChannels.push_back(zeroChannel); // �s�@greenImage,�̧ǩ�JBGR�T�h��data
	MergeImageChannels.push_back(BGR_channels[1]);
	MergeImageChannels.push_back(zeroChannel);
	merge(MergeImageChannels, green_Image);
	MergeImageChannels.clear();

	MergeImageChannels.push_back(zeroChannel); // �s�@RedImage,�̧ǩ�JBGR�T�h��data
	MergeImageChannels.push_back(zeroChannel);
	MergeImageChannels.push_back(BGR_channels[2]);
	merge(MergeImageChannels, red_Image);
	MergeImageChannels.clear();
}

void double_Size() {
	resize(image_Rotated, doubleSize_Image, Size((image_Rotated.cols) * 2, (image_Rotated.rows) * 2)); // �Q��opencv��resize��ƱN����᪺�Ϥ���j�⭿
}

void one_half_Size() {
	resize(image_Rotated, oneHalfSize_Image, Size((image_Rotated.cols) / 2, (image_Rotated.rows) / 2));// �Q��opencv��resize��ƱN����᪺�Ϥ��Y�p��1/2��
}

int main() {
	bmpRead(); // �I�s�H�W�g��Function�����@�~�n�D
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