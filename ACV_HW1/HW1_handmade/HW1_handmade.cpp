#include <iostream>
#include <vector>

using namespace std;

struct header { //宣告各種Header內容,char[]的+1是為了\0需要的空間
    unsigned char bfType [2+1];
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
    unsigned int biSize;
    unsigned int biWidth;
    unsigned int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    unsigned int biXPelsPerMeter;
    unsigned int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
};

unsigned char pixels_Input [512*512*3+1]; // 宣告imagePixels的空間 
unsigned char pixels_Rotated [512*512*3+1]; // 存放旋轉後的imagePixels
unsigned char pixels_redChannel [512*512*3+1]; // 存放red imagePixels
unsigned char pixels_greenChannel [512*512*3+1]; // 存放green imagePixels
unsigned char pixels_blueChannel [512*512*3+1]; // 存放blue imagePixels
unsigned char pixels_doubleSize [1024*1024*3+1]; // 存放doubleSize的imagePixels
unsigned char pixels_oneHalfSize [256*256*3+1]; // 存放one-HalfSize的imagePixels
struct header header; // 宣告存放Header的structure
struct header doubleSize_header; // 宣告存放doubleSize_header的structure
struct header oneHalfSize_header; // 宣告存放one-halfSize_header的structure
vector<vector<char>> pixels2D(512,vector<char>(512*3)); // 宣告一個在Rotate時暫時存放的二維陣列

void bmpRead(){
    FILE* fp = fopen("InputImage.bmp", "rb"); // 開啟InputImage,然後啟用讀取二進位的模式
    if (fp == NULL) throw "file is NULL"; // 判斷有沒有讀到檔案
    fread(header.bfType, sizeof(unsigned char), 2, fp); // 將header的資訊一個一個放入給定的空間
    fread(&header.bfSize, sizeof(unsigned int), 1, fp);
    fread(&header.bfReserved1, sizeof(unsigned short), 1, fp);
    fread(&header.bfReserved2, sizeof(unsigned short), 1, fp);
    fread(&header.bfOffBits, sizeof(unsigned int), 1, fp);
    fread(&header.biSize, sizeof(unsigned int), 1, fp);
    fread(&header.biWidth, sizeof(unsigned int), 1, fp);
    fread(&header.biHeight, sizeof(unsigned int), 1, fp);
    fread(&header.biPlanes, sizeof(unsigned short), 1, fp);
    fread(&header.biBitCount, sizeof(unsigned short), 1, fp);
    fread(&header.biCompression, sizeof(unsigned int), 1, fp);
    fread(&header.biSizeImage, sizeof(unsigned int), 1, fp);
    fread(&header.biXPelsPerMeter, sizeof(unsigned int), 1, fp);
    fread(&header.biYPelsPerMeter, sizeof(unsigned int), 1, fp);
    fread(&header.biClrUsed, sizeof(unsigned int), 1, fp);
    fread(&header.biClrImportant, sizeof(unsigned int), 1, fp);
    fread(pixels_Input, sizeof(unsigned char), 512*512*3, fp); // 讀取的資料為圖片的左下到右上,圖片大小 512*512*3 + 54 = 786,486 Bytes
    fclose(fp);
}

void bmpWrite(char const* pictureName, struct header header, unsigned char pixels_Output [], unsigned int width, unsigned int height){
    FILE* fp = fopen(pictureName, "wb"); // 傳入參數分別為圖片檔名、圖片的Header、圖片的imagePixels、圖片的寬、長
    fwrite(header.bfType, sizeof(unsigned char), 2, fp); // 寫入header資訊、imagePixels
    fwrite(&header.bfSize, sizeof(unsigned int), 1, fp);
    fwrite(&header.bfReserved1, sizeof(unsigned short), 1, fp);
    fwrite(&header.bfReserved2, sizeof(unsigned short), 1, fp);
    fwrite(&header.bfOffBits, sizeof(unsigned int), 1, fp);
    fwrite(&header.biSize, sizeof(unsigned int), 1, fp);
    fwrite(&header.biWidth, sizeof(unsigned int), 1, fp);
    fwrite(&header.biHeight, sizeof(unsigned int), 1, fp);
    fwrite(&header.biPlanes, sizeof(unsigned short), 1, fp);
    fwrite(&header.biBitCount, sizeof(unsigned short), 1, fp);
    fwrite(&header.biCompression, sizeof(unsigned int), 1, fp);
    fwrite(&header.biSizeImage, sizeof(unsigned int), 1, fp);
    fwrite(&header.biXPelsPerMeter, sizeof(unsigned int), 1, fp);
    fwrite(&header.biYPelsPerMeter, sizeof(unsigned int), 1, fp);
    fwrite(&header.biClrUsed, sizeof(unsigned int), 1, fp);
    fwrite(&header.biClrImportant, sizeof(unsigned int), 1, fp);
    fwrite(pixels_Output, sizeof(unsigned char), width*height*3, fp);
    fclose(fp);
}

void ImageRotation(){ // 先把imagePixel整理成二維的思維旋轉，再轉回一維陣列
    int pixels1Dindex = 0;
    for(int col = 0; col < 512*3; col += 3){ // 將圖片順時針旋轉90度
        for(int row = 0; row < 512; row++){
            pixels2D[row][col] = pixels_Input[pixels1Dindex];
            pixels2D[row][col+1] = pixels_Input[pixels1Dindex+1];
            pixels2D[row][col+2] = pixels_Input[pixels1Dindex+2];
            pixels1Dindex += 3;
        }
    }

    pixels1Dindex = 0;
    for(int row = 511; row > -1 ; row--){ // 將圖片從左下到右上寫回去一維陣列
        for(int col = 0 ; col < 512*3; col++){
            pixels_Rotated[pixels1Dindex] = pixels2D[row][col];
            pixels1Dindex++;
        }
    }
}

void channelSeparation(){
    for (int pixels1Dindex = 0; pixels1Dindex < 512*512*3; pixels1Dindex++){ // 將三個顏色的Channel值先預設為0
        pixels_redChannel[pixels1Dindex] = 0;
        pixels_greenChannel[pixels1Dindex] = 0;
        pixels_blueChannel[pixels1Dindex] = 0;
    }

    for (int pixels1Dindex = 0; pixels1Dindex < 512*512*3; pixels1Dindex += 3){ // blueChannel只在B的部分有值，G、R其他維持0
        pixels_blueChannel[pixels1Dindex] = pixels_Rotated[pixels1Dindex];
    }
    
    for (int pixels1Dindex = 1; pixels1Dindex < 512*512*3; pixels1Dindex += 3){ // greenChannel只在G的部分有值，B、R其他維持0
        pixels_greenChannel[pixels1Dindex] = pixels_Rotated[pixels1Dindex];
    }
    
    for (int pixels1Dindex = 2; pixels1Dindex < 512*512*3; pixels1Dindex += 3){ // redChannel只在R的部分有值，B、G其他維持0
        pixels_redChannel[pixels1Dindex] = pixels_Rotated[pixels1Dindex];
    }
}

void double_Size(){ // 圖片大小 1024*1024*3 + 54 = 3,145,782 Bytes
    doubleSize_header = header;
    doubleSize_header.bfSize = 3145782; // 將header的Size、width、Height改為doubleSize的值
    doubleSize_header.biWidth = 1024;
    doubleSize_header.biHeight = 1024;

    int pixdels_double_inedex = 0;
    for(int row = 511; row > -1; row--){ // 將原本的圖片的每個點在height&width的部分都要重複兩次
        for(int counter_1 = 0; counter_1 < 2; counter_1++){
            for(int col = 0; col < 512*3; col += 3){
                for(int counter_2 = 0; counter_2 < 2; counter_2++){
                    pixels_doubleSize[pixdels_double_inedex] = pixels2D[row][col];
                    pixels_doubleSize[pixdels_double_inedex+1] = pixels2D[row][col+1];
                    pixels_doubleSize[pixdels_double_inedex+2] = pixels2D[row][col+2];
                    pixdels_double_inedex+=3;
                }
            }        
        }
    }
}

void one_half_Size(){ //圖片大小 256*256*3 + 54 = 196,662 Bytes
    oneHalfSize_header = header;
    oneHalfSize_header.bfSize = 196662; // 將header的Size、width、Height改為one-HalfSize的值
    oneHalfSize_header.biWidth = 256;
    oneHalfSize_header.biHeight = 256;
    
    int pixels_oneHalf_index = 0;
    for(int row = 511; row > -1; row -= 2){ // 將原本的圖片的height&width,每兩個點取一次到新的imagePixels
        for(int col = 0; col < 512*3; col += 6){
            pixels_oneHalfSize[pixels_oneHalf_index] = pixels2D[row][col];
            pixels_oneHalfSize[pixels_oneHalf_index+1] = pixels2D[row][col+1];
            pixels_oneHalfSize[pixels_oneHalf_index+2] = pixels2D[row][col+2];
            pixels_oneHalf_index += 3;
        }
    }
}

int main(){
    bmpRead();
    ImageRotation();
    channelSeparation();
    double_Size();
    one_half_Size();
    bmpWrite("Rotated.bmp", header, pixels_Rotated, 512, 512);
    bmpWrite("BlueChannel.bmp", header, pixels_blueChannel, 512, 512);
    bmpWrite("GreenChannel.bmp", header, pixels_greenChannel, 512, 512);
    bmpWrite("RedChannel.bmp", header, pixels_redChannel, 512, 512);
    bmpWrite("doubleSize.bmp", doubleSize_header, pixels_doubleSize, 1024, 1024);
    bmpWrite("oneHalfSize.bmp", oneHalfSize_header, pixels_oneHalfSize, 256, 256);
}