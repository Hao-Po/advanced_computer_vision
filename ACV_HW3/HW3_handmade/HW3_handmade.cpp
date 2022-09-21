#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#define pi 3.1415926

using namespace std;

double alpha = 15 * pi / 180;
double dx = 4;
double dy = -10;
double dz = 5;
double gamma_zero = 0;
double theta_zero = 0.025;

struct uv_points {
    double u;
    double v;
};

struct xy_points {
    double x;
    double y;
};

double cot(double x) {
    return tan(pi / 2 - x);
}

vector<unsigned char> rawRead(const char* pictureName, int width, int height) {
    vector<unsigned char> pixels_Input(width * height * 3);

    FILE* fp = fopen(pictureName, "rb");
    if (fp == NULL) throw "file is NULL";
    fread(&pixels_Input[0], sizeof(unsigned char), width * height * 3, fp);
    fclose(fp);

    return pixels_Input;
}

void rawWrite(char const* pictureName, vector <unsigned char> pixels_Output, unsigned int width, unsigned int height) {
    FILE* fp = fopen(pictureName, "wb");
    fwrite(&pixels_Output[0], sizeof(unsigned char), width * height * 3, fp);
    fclose(fp);
}

vector<vector<unsigned char>> oneDimension2twoDimension(vector<unsigned char> pixels_input, int width, int height) {
    vector<vector<unsigned char>>pixels_input2D(height, vector<unsigned char>(width * 3));

    int index = 0;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width * 3; col++) {
            pixels_input2D[row][col] = pixels_input[index++];
        }
    }
    return pixels_input2D;
}

vector<unsigned char> twoDimension2oneDimension(vector<vector<unsigned char>> pixels_input2D, int width, int height) {
    vector<unsigned char>pixels_input;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width * 3; col++) {
            pixels_input.push_back(pixels_input2D[row][col]);
        }
    }

    return pixels_input;
}

vector<unsigned char> rgb2binary(vector<unsigned char> pixels_input, int width, int height) {
    vector<unsigned char>pixels_output(width * height * 3); 
    for (int index = 0; index < width * height; index++) {
        int b = pixels_input[index * 3];
        int g = pixels_input[(index * 3) + 1];
        int r = pixels_input[(index * 3) + 2];
        int gray = 0.2989 * r + 0.5870 * g + 0.1140 * b;
        pixels_output[index * 3] = pixels_output[(index * 3) + 1] = pixels_output[(index * 3) + 2] = gray;

        int uHorizon = (height - 1) / (2 * alpha) * (-1 * theta_zero + alpha);

        if (pixels_output[index * 3] == 161 || pixels_output[index * 3] == 254) { // 三條線的閥值
            pixels_output[index * 3] = pixels_output[(index * 3) + 1] = pixels_output[(index * 3) + 2] = 255;
        }
        else {
            pixels_output[index * 3] = pixels_output[(index * 3) + 1] = pixels_output[(index * 3) + 2] = 0;
        }
    }
    return pixels_output;
}

vector<unsigned char> EdgeDetection(vector<unsigned char> pixels_input, int width, int height) {
    vector<vector<unsigned char>>pixels_input2D = oneDimension2twoDimension(pixels_input, width, height);
    vector<vector<unsigned char>>pixels_edge2D(height, vector<unsigned char>(width * 3));
    
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            if (row != 0 && row != height - 1 && col != 0 && col != width - 1) {
                    pixels_edge2D[row][col * 3] = 2*pixels_input2D[row - 1][(col - 1) * 3] - pixels_input2D[row - 1][col * 3] - pixels_input2D[row - 1][(col + 1) * 3] \
                        - pixels_input2D[row][(col - 1) * 3] + 2 * pixels_input2D[row][col * 3] - pixels_input2D[row][(col + 1) * 3] \
                        - pixels_input2D[row + 1][(col - 1) * 3] - pixels_input2D[row + 1][col * 3] +2* pixels_input2D[row + 1][(col + 1) * 3];
            }
        }
    }

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            if (pixels_edge2D[row][col * 3] != 0) {
                pixels_edge2D[row][col * 3] = pixels_edge2D[row][col * 3 + 1] = pixels_edge2D[row][col * 3 + 2] = 255;
            }
        }
    }

    vector<unsigned char>pixels_edge = twoDimension2oneDimension(pixels_edge2D, 1024, 768);
    return pixels_edge;
}

void DrawLines(vector<vector<unsigned char>>& pixels_label2D, int row, int col, int color) { 
    if (color == 0) { // red
        for (int range = -15; range < 15; range++) {
            pixels_label2D[row][(col+range) * 3] = 255;
            pixels_label2D[row][(col + range) * 3 + 1] = 0;
            pixels_label2D[row][(col + range) * 3 + 2] = 0;
        }
    }
    if (color == 1) { // green
        for (int range = -7; range < 7; range++) {
            pixels_label2D[row][(col + range) * 3] = 0;
            pixels_label2D[row][(col + range) * 3 + 1] = 255;
            pixels_label2D[row][(col + range) * 3 + 2] = 0;
        }
    }
    if (color == 2) { // blue
        pixels_label2D[row][col * 3] = 0;
        pixels_label2D[row][col * 3 + 1] = 0;
        pixels_label2D[row][col * 3 + 2] = 255;
    }
}

vector<unsigned char> HoughTransform(vector<unsigned char> pixels_input, vector<unsigned char>pixels_edge, int width, int height) {
    vector<vector<unsigned char>>pixels_input2D = oneDimension2twoDimension(pixels_input, width, height);
    vector<vector<unsigned char>>pixels_edge2D = oneDimension2twoDimension(pixels_edge, width, height);
    int maxDistance = sqrt(width * width + height * height);
    vector<vector<int>>Hspace(2 * maxDistance, vector<int>(181, 0));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (pixels_edge2D[y][x * 3] == 255) {
                for (int theta = 0; theta < 181; theta++) {
                    double rad = theta * pi / 180;
                    int r = int(round(x * cos(rad) + y * sin(rad)));
                    Hspace[r + maxDistance][theta]++;
                }
            }
        }   
    }

    double m, b;
    for (int r = 0; r < 2 * maxDistance; r++) {
        for (int theta = 0; theta < 181; theta++) {
            if (Hspace[r][theta] > 150) {
                double rad = theta * pi / 180;
                double x0 = (r-maxDistance) * cos(rad);
                double y0 = (r-maxDistance) * sin(rad);
                int x1 = int(x0 + 1000 * (-sin(rad)));
                int y1 = int(y0 + 1000 * cos(rad));
                int x2 = int(x0 - 1000 * (-sin(rad)));
                int y2 = int(y0 - 1000 * cos(rad));

                if (x2 - x1 != 0) {
                    m = double(y2 - y1) / double(x2 - x1);
                    b = y1 - m * x1;
                    int uHorizon = (height - 1) / (2 * alpha) * (-1 * theta_zero + alpha);
                    for (int col = 0; col < width; col++) {
                        int row = m * col + b;
                        if (uHorizon < row && row < height) {
                            if (-1.33 < m && m < -1.15) {
                                DrawLines(pixels_input2D, row, col,0);
                            }
                            if (1.14 < m && m < 1.37) {
                                DrawLines(pixels_input2D, row, col, 1);
                            }
                            if (-0.45 < m && m < -0.37) {
                                DrawLines(pixels_input2D, row, col, 2);
                            }
                        }
                    }
                }
            }
        }
    }

    vector<unsigned char>pixels_output = twoDimension2oneDimension(pixels_input2D, width, height);
    return pixels_output;
}

xy_points uv2xy(double u, double v, int width, int height) {
    xy_points xy_point;
    xy_point.x = dz * cot(theta_zero - alpha + u * ((2 * alpha) / (height - 1))) * sin(gamma_zero - alpha + v * ((2 * alpha) / (width - 1))) + dx;
    xy_point.y = dz * cot(theta_zero - alpha + u * ((2 * alpha) / (height - 1))) * cos(gamma_zero - alpha + v * ((2 * alpha) / (width - 1))) + dy;

    return xy_point;
}

uv_points xy2uv(double x, double y, int width, int height) {
    uv_points uv_point;
    if ((x - dx) == 0 || (y - dy) == 0) {
        uv_point.u = uv_point.v = -1;
    }
    else {
        uv_point.u = ((height - 1) / (2 * alpha)) * (atan((dz * sin(atan((x - dx) / (y - dy)))) / (x - dx)) - theta_zero + alpha);
        uv_point.v = ((width - 1) / (2 * alpha)) * (atan((x - dx) / (y - dy)) - gamma_zero + alpha);
    }

    return uv_point;
}

vector<unsigned char> forwardWarping(vector<unsigned char> image_coordinate, int width, int height) {
    vector<vector<unsigned char>>image_coordinate2D = oneDimension2twoDimension(image_coordinate, width, height);

    int uHorizon = (height - 1) / (2 * alpha) * (-1 * theta_zero + alpha); // uHorizon = 346
    vector<xy_points> image_coordinate_xy;

    double xmax = 0, xmin = 0, ymax = 0, ymin = 0;
    for (int u = uHorizon; u < height; u++) {
        for (int v = 0; v < width; v++) {
            xy_points xy_point = uv2xy(u, v, width, height);
            image_coordinate_xy.push_back(xy_point);
            if (xy_point.x > xmax) { xmax = xy_point.x; }
            if (xy_point.x < xmin) { xmin = xy_point.x; }
            if (xy_point.y > ymax) { ymax = xy_point.y; }
            if (xy_point.y < ymin) { ymin = xy_point.y; }
        }
    }
    //cout << xmax << ", " << xmin << ", " << ymax << ", " << ymin;

    double resize_num = 10;
    vector<vector<unsigned char>>world_coordinate2D(ceil((xmax - xmin) / resize_num + 1), vector<unsigned char>(ceil((ymax - ymin) / resize_num + 1)  * 3, 0));
   
    for (int x = xmin / resize_num; x < ceil(xmax / resize_num) + 1; x++) {
        for (int y = ymin / resize_num; y < ceil(ymax / resize_num) + 1; y++) {
            uv_points uv_point = xy2uv(x, y, width, height); 
            if (uHorizon < uv_point.u && uv_point.u < 768 && 0 < uv_point.v && uv_point.v < 1024) {
                world_coordinate2D[floor(x - (xmin / resize_num))][floor(y - (ymin / resize_num)) * 3] = image_coordinate2D[floor(uv_point.u)][floor(uv_point.v) * 3];
                world_coordinate2D[floor(x - (xmin / resize_num))][floor(y - (ymin / resize_num)) * 3 + 1] = image_coordinate2D[floor(uv_point.u)][floor(uv_point.v) * 3 + 1];
                world_coordinate2D[floor(x - (xmin / resize_num))][floor(y - (ymin / resize_num)) * 3 + 2] = image_coordinate2D[floor(uv_point.u)][floor(uv_point.v) * 3 + 2];
            }
        }
    }

    vector<unsigned char>world_coordinate = twoDimension2oneDimension(world_coordinate2D, ceil((ymax - ymin)/ resize_num + 1), ceil((xmax - xmin)/ resize_num + 1));
    //cout << ceil((ymax - ymin) / resize_num + 1) << ", " << ceil((xmax - xmin) / resize_num + 1) << endl;

    return world_coordinate;
}

vector<unsigned char> crop(vector<unsigned char> pixels_input, int width, int height) {
    int crop_height = 1612 - 1512;
    int crop_width = 944 - 840;
    vector<vector<unsigned char>>pixels_input2D = oneDimension2twoDimension(pixels_input, width, height);
    vector<vector<unsigned char>>pixels_crop2D(crop_height, vector<unsigned char>(crop_width * 3));

    for (int row = 0; row < crop_height; row++) {
        for (int col = 0; col < crop_width; col++) {
            pixels_crop2D[row][col * 3] = pixels_input2D[1512 + row][(840 + col) * 3];
            pixels_crop2D[row][col * 3 + 1] = pixels_input2D[1512 + row][(840 + col) * 3 + 1];
            pixels_crop2D[row][col * 3 + 2] = pixels_input2D[1512 + row][(840 + col) * 3 + 2];
        }
    }
    
    vector<unsigned char>pixels_crop = twoDimension2oneDimension(pixels_crop2D, crop_width, crop_height);
    //cout << crop_width << ", " << crop_height << endl;
    return pixels_crop;
}

int main() {
    vector<unsigned char> pixels_road;
    vector<unsigned char> pixels_Binary;
    vector<unsigned char> pixels_Edge;
    vector<unsigned char> pixels_Hough;
    vector<unsigned char> pixels_IPM_normal;
    vector<unsigned char> pixels_IPM_label;
    vector<unsigned char> pixels_crop_normal;
    vector<unsigned char> pixels_crop_label;

    int img_width = 1024, img_height = 768;

    pixels_road = rawRead("road.raw", img_width, img_height);
    pixels_Binary = rgb2binary(pixels_road, img_width, img_height);
    pixels_Edge = EdgeDetection(pixels_Binary, img_width, img_height);
    pixels_Hough = HoughTransform(pixels_road, pixels_Edge, img_width, img_height);
    pixels_IPM_normal = forwardWarping(pixels_road, img_width, img_height);
    pixels_crop_normal = crop(pixels_IPM_normal, 6859, 3120);
    pixels_IPM_label = forwardWarping(pixels_Hough, img_width, img_height); // 6859*3120
    pixels_crop_label = crop(pixels_IPM_label, 6859, 3120); //  104*100
    
    //rawWrite("binary.raw", pixels_Binary, img_width, img_height); // 1024*768
    //rawWrite("Edge.raw", pixels_Edge, img_width, img_height); // 1024*768
    //rawWrite("Hough.raw", pixels_Hough, img_width, img_height); // 1024*768
    //rawWrite("IPM_normal.raw", pixels_IPM_normal, 6859, 3120);
    //rawWrite("IPM_label.raw", pixels_IPM_label, 6859, 3120);
    rawWrite("crop_normal.raw", pixels_crop_normal, 104, 100);
    rawWrite("crop_label.raw", pixels_crop_label, 104, 100);
}