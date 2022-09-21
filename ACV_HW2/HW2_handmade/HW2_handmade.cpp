#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

vector<unsigned char> header(54); // 存放header的空間
vector<unsigned char> pixels_Input(512 * 512 * 3); // 存放raw data的空間
vector<unsigned char> equivalenceLabel; // 存放判斷衝突的label空間

void bmpRead(const char* pictureName) {
    FILE* fp = fopen(pictureName, "rb"); // 開啟InputImage,然後啟用讀取二進位的模式
    if (fp == NULL) throw "file is NULL"; // 判斷有沒有讀到檔案
    fread(&header[0], sizeof(unsigned char), 54, fp); // 存取header資訊
    fread(&pixels_Input[0], sizeof(unsigned char), 512 * 512 * 3, fp); // 讀取的資料為圖片的左下到右上,圖片大小 512*512*3 + 54 = 786,486 Bytes
    fclose(fp);
}

void bmpWrite(char const* pictureName, vector<unsigned char>header, vector <unsigned char> pixels_Output, unsigned int width, unsigned int height) {
    FILE* fp = fopen(pictureName, "wb"); // 傳入參數分別為圖片檔名、圖片的Header、圖片的imagePixels、圖片的寬、長
    fwrite(&header[0], sizeof(unsigned char), 54, fp);
    fwrite(&pixels_Output[0], sizeof(unsigned char), width * height * 3, fp);
    fclose(fp);
}

vector<vector<unsigned char>> oneDimension2twoDimension(vector<unsigned char> pixels_input) {
    vector<vector<unsigned char>>pixels_input2D(512, vector<unsigned char>(512 * 3));

    int index = 0; // 寫一個把vector從一維512*512*3轉為二維512*(512*3)的function
    for (int row = 511; row > -1; row--) {
        for (int col = 0; col < 512 * 3; col++) {
            pixels_input2D[row][col] = pixels_input[index++];
        }
    }
    return pixels_input2D;
}

vector<unsigned char> twoDimension2oneDimension(vector<vector<unsigned char>> pixels_input2D) {
    vector<unsigned char>pixels_input(512 * 512 * 3);

    int index = 0;; // 寫一個把vector從二維512*(512*3)轉為一維512*512*3的function
    for (int row = 511; row > -1; row--) {
        for (int col = 0; col < 512 * 3; col++) {
            pixels_input[index++] = pixels_input2D[row][col] ;
        }
    }
    return pixels_input;
}

vector<unsigned char> rgb2gray(vector<unsigned char> pixels_input) {
    vector<unsigned char>pixels_output(512 * 512 * 3); // 利用權重的算法算出gray值, 讓BGR三個值相等, 維持three channels
    for (int index = 0; index < 512 * 512 * 3; index += 3) {
        int b = pixels_input[index];
        int g = pixels_input[index + 1];
        int r = pixels_input[index + 2];
        int gray = 0.2989 * r + 0.5870 * g + 0.1140 * b;
        pixels_output[index] = pixels_output[index + 1] = pixels_output[index + 2] = gray;
    }
    return pixels_output;
}

vector<unsigned char> gray2binary(vector<unsigned char> pixels_input) {
    vector<unsigned char>pixels_output(512 * 512 * 3); // 利用threshold比大小的方式把gray scale轉為binary image
    for (int index = 0; index < 512 * 512 * 3; index += 3) {
        if (pixels_input[index] > 210) { // threshold = 210
            pixels_output[index] = pixels_output[index + 1] = pixels_output[index + 2] = 0;
        }
        else {
            pixels_output[index] = pixels_output[index + 1] = pixels_output[index + 2] = 255;
        }
    }
    return pixels_output;
}

vector<unsigned char> dilation(vector<unsigned char> pixels_input) { // dilation做法為觀察點如果為白點, 那將觀察點設為原點的十字都成為白點
    vector<vector<unsigned char>>pixels_input2D(512, vector<unsigned char>(512 * 3));
    vector<vector<unsigned char>>pixels_dilation2D(512, vector<unsigned char>(512 * 3));
    vector<unsigned char>pixels_dilation(512 * 512 * 3);

    pixels_input2D = oneDimension2twoDimension(pixels_input);

    for (int row = 0; row < 512; row++) { // cross filter
        for (int col = 0; col < 512 * 3; col += 3) {
            if (pixels_input2D[row][col] == 255) { // 觀察點本身
                pixels_dilation2D[row][col] = pixels_dilation2D[row][col + 1] = pixels_dilation2D[row][col + 2] = 255;
                if (row != 0) { // up
                    pixels_dilation2D[row - 1][col] = pixels_dilation2D[row - 1][col + 1] = pixels_dilation2D[row - 1][col + 2] = 255;
                }
                if (row != 511) { // down
                    pixels_dilation2D[row + 1][col] = pixels_dilation2D[row + 1][col + 1] = pixels_dilation2D[row + 1][col + 2] = 255;
                }
                if (col != 0){ // left
                    pixels_dilation2D[row][col -1] = pixels_dilation2D[row][col - 2] = pixels_dilation2D[row][col - 3] = 255;
                }
                if (col != 512 * 3 - 3) {
                    pixels_dilation2D[row][col + 3] = pixels_dilation2D[row][col + 4] = pixels_dilation2D[row][col + 5] = 255;
                }
            }
        }
    }

    pixels_dilation = twoDimension2oneDimension(pixels_dilation2D);

    return pixels_dilation;
}

void giveLabel(vector<vector<unsigned char>> &pixels_label2D, int row, int col, int label) { // 用以方便同時給BGR三個值相同的label
    pixels_label2D[row][col] = pixels_label2D[row][col + 1] = pixels_label2D[row][col + 2] = label;
}

int find_root(int x) { // 找根點, 用以更新所有衝突的label
    if (equivalenceLabel[x] == x) {
        return x;
    }
    int root = find_root(equivalenceLabel[x]);
    equivalenceLabel[x] = root;
    
    return root;
}

vector<unsigned char> connectedComponents(vector<unsigned char> pixels_input) { // 使用classic algorithm的作法搭配equivalence Label
    vector<vector<unsigned char>>pixels_input2D(512, vector<unsigned char>(512 * 3));
    vector<vector<unsigned char>>pixels_output2D(512, vector<unsigned char>(512 * 3));
    vector<unsigned char>pixels_output(512 * 512 * 3);

    pixels_input2D = oneDimension2twoDimension(pixels_input);
   
    unsigned char labelCount = 1;
    unsigned char upLabel, leftLabel;

    equivalenceLabel.push_back(0); 
    for (int row = 0; row < 512; row++) { // 從圖片的左上掃到右下, 判斷左邊跟上面的label, 如果沒有就給新的, 如果有衝突就給小的那個
        for (int col = 0; col < 512*3; col += 3) {
            if (pixels_input2D[row][col] == 255) {
                if (row != 0) { // 給定一個左邊label及上面label的變數，比較好判斷
                    upLabel = pixels_output2D[row - 1][col];
                }
                if (col != 0) {
                    leftLabel = pixels_output2D[row][col - 1];
                }
                if (row == 0 && col == 0) {
                    giveLabel(pixels_output2D, row, col, labelCount);
                    equivalenceLabel.push_back(labelCount++);
                }
                else if(row == 0 && col != 0) {
                    if (leftLabel != 0) {
                        giveLabel(pixels_output2D, row, col, leftLabel);
                    }
                    else {
                        giveLabel(pixels_output2D, row, col, labelCount);
                        equivalenceLabel.push_back(labelCount++);
                    }
                }
                else if (row != 0 && col == 0) {
                    if (upLabel != 0) {
                        giveLabel(pixels_output2D, row, col, upLabel);
                    }
                    else {
                        giveLabel(pixels_output2D, row, col, labelCount);
                        equivalenceLabel.push_back(labelCount++);
                    }
                }
                else { // row != 0 && col != 0 
                    if (upLabel == 0 && leftLabel == 0) {
                        giveLabel(pixels_output2D, row, col, labelCount);
                        equivalenceLabel.push_back(labelCount++);
                    }
                    else if (upLabel == 0 && leftLabel != 0) {
                        giveLabel(pixels_output2D, row, col, leftLabel);
                    }
                    else if (upLabel != 0 && leftLabel == 0){
                        giveLabel(pixels_output2D, row, col, upLabel);
                    }
                    else { // upLabel != 0 && leftLabel != 0
                        if (upLabel == leftLabel) {
                            giveLabel(pixels_output2D, row, col, upLabel);
                        }
                        else { // 做第一次的衝突判斷
                            giveLabel(pixels_output2D, row, col, min(upLabel, leftLabel));
                            equivalenceLabel[int(max(upLabel, leftLabel))] = equivalenceLabel[int(min(upLabel, leftLabel))];
                        }
                    }
                }
            }
        }
    } 

    for (int index = 0; index < equivalenceLabel.size(); index++) { // 利用遞迴將衝突的label更新
        find_root(index);
    }
    
    for (int row = 0; row < 512; row++) { // 經過equivalence後,更新pixels_label2D的label
        for (int col = 0; col < 512 * 3; col += 3) {
            giveLabel(pixels_output2D, row, col, equivalenceLabel[int(pixels_output2D[row][col])]);
        }
    }

    pixels_output = twoDimension2oneDimension(pixels_output2D); // 轉回一維用以寫入圖片
  
    // 這裡是整理所有的更新後的label, 用sort → unique → erase,留下每一種label剩一個,之後作圖判斷使用
    sort(equivalenceLabel.begin(), equivalenceLabel.end()); // 整理label
    auto last = unique(equivalenceLabel.begin(), equivalenceLabel.end()); // last is iterator
    equivalenceLabel.erase(last, equivalenceLabel.end());
    equivalenceLabel.erase(equivalenceLabel.begin()); // 刪除背景label
        
    return pixels_output;
}

void properties_analysis(int row_max, int row_min, int col_max, int col_min) { // 藉由bouning_box的長寬做判斷
    int longest = ((row_max - row_min) > ((col_max - col_min) / 3) ? (row_max - row_min) : ((col_max - col_min) / 3));
    string orientation = ((row_max - row_min) > ((col_max - col_min) / 3) ? ("up to down") : ("left to right"));

    cout << "length: " << longest << ", it is " << orientation << endl;
}

void bounding_box(vector<unsigned char>& pixels_binary, vector<unsigned char>& pixels_label, unsigned char label, int part) {
    vector<vector<unsigned char>>pixels_binary2D(512, vector<unsigned char>(512 * 3));
    vector<vector<unsigned char>>pixels_label2D(512, vector<unsigned char>(512 * 3));

    pixels_binary2D = oneDimension2twoDimension(pixels_binary); 
    pixels_label2D = oneDimension2twoDimension(pixels_label);
    
    // 利用label的最大最小row、col知道邊框座標,運算bounding box的長寬,再繪圖
    int row_max, row_min, col_max, col_min;
    for (int row = 0; row < 512; row++) { // 做出一個預設值
        for (int col = 0; col < 512 * 3; col++) {
            if (pixels_label2D[row][col] == label) {
                row_max = row_min = row;
                col_max = col_min = col;
                goto update;
            }
        }
    }

    update:
    for (int row = 0; row < 512; row++) { // 更新邊框值
        for (int col = 0; col < 512 * 3; col++) {
            if (pixels_label2D[row][col] == label) {
                if (row > row_max) {
                    row_max = row;
                }
                if (row < row_min) {
                    row_min = row;
                }
                if (col > col_max) {
                    col_max = col;
                }
                if (col < col_min) {
                    col_min = col;
                }
            }
        }
    }
    
    cout << "Part " << part+1 << " ----> ";
    properties_analysis(row_max, row_min, col_max, col_min);  

    for (int row = row_min; row < row_max + 1; row++) {
        pixels_binary2D[row][col_min] = pixels_binary2D[row][col_min + 1] = 0;
        pixels_binary2D[row][col_min + 2] = 255;
        pixels_binary2D[row][col_max - 2] = pixels_binary2D[row][col_max - 1] = 0;
        pixels_binary2D[row][col_max] = 255;
    }
    for (int col = col_min; col < col_max + 1; col += 3) {
        pixels_binary2D[row_min][col] = pixels_binary2D[row_min][col + 1] = 0;
        pixels_binary2D[row_min][col + 2] = 255;
        pixels_binary2D[row_max][col] = pixels_binary2D[row_max][col + 1] = 0;
        pixels_binary2D[row_max][col + 2] = 255;
    }

    pixels_binary = twoDimension2oneDimension(pixels_binary2D);
}

void centroid_area(vector<unsigned char>& pixels_binary, vector<unsigned char>& pixels_label, unsigned char label, int part) {
    vector<vector<unsigned char>>pixels_binary2D(512, vector<unsigned char>(512 * 3));
    vector<vector<unsigned char>>pixels_label2D(512, vector<unsigned char>(512 * 3));

    pixels_binary2D = oneDimension2twoDimension(pixels_binary);
    pixels_label2D = oneDimension2twoDimension(pixels_label);

    // 加總每個區域白點的row、col再除以白點的數量(area),得到平均(centroid 座標)
    int row_sum = 0, col_sum = 0, amounts = 0;
    for (int row = 0; row < 512; row++) {
        for (int col = 0; col < 512 * 3; col += 3) {
            if (pixels_label2D[row][col] == label) {
                row_sum += row;
                col_sum += col;
                amounts++;
            }
        }
    }
    int centroid_row = row_sum / amounts;
    int centroid_col = col_sum / amounts;

    if (centroid_col % 3 == 0) {
        pixels_binary2D[centroid_row][centroid_col] = pixels_binary2D[centroid_row][centroid_col + 1] = 0;
        pixels_binary2D[centroid_row][centroid_col + 2] = 255;
    }
    if (centroid_col % 3 == 1) {
        pixels_binary2D[centroid_row][centroid_col] = pixels_binary2D[centroid_row][centroid_col - 1] = 0;
        pixels_binary2D[centroid_row][centroid_col + 1] = 255;
    }
    if (centroid_col % 3 == 2) {
        pixels_binary2D[centroid_row][centroid_col - 1] = pixels_binary2D[centroid_row][centroid_col - 2] = 0;
        pixels_binary2D[centroid_row][centroid_col] = 255;
    }

    pixels_binary = twoDimension2oneDimension(pixels_binary2D);

    cout << "Part " << part + 1 << " ----> ";
    cout << "area: " << amounts << ",  centroid: (" << centroid_row << ", " << centroid_col / 3 << ")" << endl;
}

int main() {
    vector<unsigned char> pixels_gray(512 * 512 * 3);
    vector<unsigned char> pixels_binary(512 * 512 * 3);
    vector<unsigned char> pixels_binary_afterDilation(512 * 512 * 3);
    vector<unsigned char> pixels_label(512 * 512 * 3);

    bmpRead("hand.bmp");

    /*binarizing*/
    double binarizing_start = clock();
    pixels_gray = rgb2gray(pixels_Input);
    pixels_binary = gray2binary(pixels_gray);
    double binarizing_end = clock();
    double binarizing_time = (binarizing_end - binarizing_start) / CLOCKS_PER_SEC;
    
    /*morphology*/
    double morphology_start = clock();
    pixels_binary_afterDilation = dilation(dilation(pixels_binary));
    double morphology_end = clock();
    double morphology_time = (morphology_end - morphology_start) / CLOCKS_PER_SEC;

    /*connected component + centroid + area*/
    double connectedComponent_start = clock();
    cout << "Area & Centroid: (圖片手中的紅點為centroid)" << endl;
    pixels_label = connectedComponents(pixels_binary_afterDilation);
    for (int index = 0; index < equivalenceLabel.size(); index++) {
        centroid_area(pixels_binary_afterDilation, pixels_label, equivalenceLabel[index], index);
    }
    double connectedComponent_end = clock();
    double connectedComponent_time = (connectedComponent_end - connectedComponent_start) / CLOCKS_PER_SEC;
    
    /*bounding box (drawing) + properties analysis*/
    double boundingBox_start = clock();
    cout << endl << "Properties Analysis: " << endl;
    for (int index = 0; index < equivalenceLabel.size(); index++) {
        bounding_box(pixels_binary_afterDilation, pixels_label, equivalenceLabel[index], index); // drawingImage is binary_dilation.bmp
    }
    double boundingBox_end = clock();
    double boundingBox_time = (boundingBox_end - boundingBox_start) / CLOCKS_PER_SEC;
    
    cout << endl << "╔================================================================================================╗";
    cout << endl << "∥" << setw(10) << "Time(sec.)" << " ∥ " << setw(10) << "binarizing" << " ∥ " << setw(10) << "morphology" << " ∥ ";
    cout << setw(19) << "connected component" << " ∥ " << setw(19) << "properties analysis" << " ∥ " << setw(7) << "drawing" << "∥";
    cout << endl << "∥" << setw(10) << "C program" << " ∥ " << setw(10) << binarizing_time << " ∥ " << setw(10) << morphology_time << " ∥ ";
    cout << setw(19) << connectedComponent_time << " ∥ " << setw(19) << connectedComponent_time << " ∥ " << setw(7) << boundingBox_time << "∥";
    cout << endl << "╚================================================================================================╝" << endl;

    bmpWrite("binaryImage.bmp", header, pixels_binary, 512, 512);
    //bmpWrite("gray.bmp", header, pixels_gray, 512, 512);
    bmpWrite("drawingImage.bmp", header, pixels_binary_afterDilation, 512, 512);
    //bmpWrite("label.bmp", header, pixels_label, 512, 512);

    system("pause");
    return 0;
}