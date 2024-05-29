#include "lodepng.h"
#include "lodepng.c"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int i, j;

typedef struct coordes{
    int x, y;
}coord;


char* load_png_file  (const char *filename, int *width, int *height) {
	unsigned char *image = NULL;
	int error = lodepng_decode32_file(&image, width, height, filename);
	if (error) {
		printf("error %u: %s\n", error, lodepng_error_text(error));
		return NULL;
	}

	return (image);
}

void write_png_file(const char *filename, const unsigned char *image, unsigned width, unsigned height) {
    unsigned char* png;
    size_t size;
    int error = lodepng_encode32(&png, &size, image, width, height);
    if (!error) {
        lodepng_save_file(png, size, filename);
    }
    free(png);
}

void gaussian(unsigned char* image, unsigned w, unsigned h) {
    unsigned x,y;
    int i,j;
    float r,g,b;
    float kernel[3][3] = {
        {1.3, 0.8, 1.3},
        {0.8, 1.5, 0.8},
        {1.3, 0.8, 1.3}
    };
    for (y = 1; y < h - 1; y++) {
        for (x = 1; x < w - 1; x++) {
            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    r = kernel[i + 1][j + 1] * image[4 * w * (y + i) + 4 * (x + j) + 0];
                    g = kernel[i + 1][j + 1] * image[4 * w * (y + i) + 4 * (x + j) + 1];
                    b = kernel[i + 1][j + 1] * image[4 * w * (y + i) + 4 * (x + j) + 2];
                }
            }
            image[4 * w * y + 4 * x + 0] = fmin(r, 255);
            image[4 * w * y + 4 * x + 1] = fmin(g, 255);
            image[4 * w * y + 4 * x + 2] = fmin(b, 255);
            image[4 * w * y + 4 * x + 3] = 255;
        }
    }
    return ;
}

void bord(unsigned char* image, unsigned w, unsigned h) {
    unsigned x,y;
    int i,j;
    float r,g,b;
    float kernel[3][3] = {
        {1.0, 0.3, 1.0},
        {0.3, 2.0, 0.3},
        {1.0, 0.3, 1.0}
    };
    for (y = 1; y < h - 1; y++) {
        for (x = 1; x < w - 1; x++) {
            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    r = kernel[i + 1][j + 1] * image[4 * w * (y + i) + 4 * (x + j) + 0];
                    g = kernel[i + 1][j + 1] * image[4 * w * (y + i) + 4 * (x + j) + 1];
                    b = kernel[i + 1][j + 1] * image[4 * w * (y + i) + 4 * (x + j) + 2];
                }
            }
            image[4 * w * y + 4 * x + 0] = fmin(r, 255);
            image[4 * w * y + 4 * x + 1] = fmin(g, 255);
            image[4 * w * y + 4 * x + 2] = fmin(b, 255);
            image[4 * w * y + 4 * x + 3] = 255;
        }
    }
    return ;
}

void fill(unsigned char* image, int x, int y, int newColor1, int newColor2, int newColor3, int eps, int width, int height) {
    int dx[] = {-1, 0, 1, 0};
    int dy[] = {0, 1, 0, -1};

    coord* stack = malloc(width * height * 4 * sizeof(coord));
    long top = 0;

    stack[top++] = (coord){x, y};

    while(top > 0) {
        coord p = stack[--top];

        if(p.x < 0 || p.x >= width || p.y < 0 || p.y >= height) continue;

        int ind = (p.y * width + p.x) * 4;
        if(image[ind] > eps) continue;

        image[ind] = newColor1;
        image[ind + 1] = newColor2;
        image[ind + 2] = newColor3;

        for(int i = 0; i < 4; i++) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
            if(nx > 0 && nx < width && ny > 0 && ny < height) {
                stack[top++] = (coord){nx, ny};
            }
        }
    }
    free(stack);
}

void average(unsigned char* image, unsigned char* res, int width, int height){
    int gx[3][3] = {{-1, 0, 1},
                    {-1, 0, 1},
                    {-1, 0, 1}};
    int gy[3][3] = {{1,  1,  1},
                    {0,  0,  0},
                    {-1, -1, -1}};

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sumX = 0, sumY = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int index = ((y + dy) * width + (x + dx)) * 4;
                    int gray = (image[index] + image[index + 1] + image[index + 2]) / 3;
                    sumX += gx[dy + 1][dx + 1] * gray;
                    sumY += gy[dy + 1][dx + 1] * gray;
                }
            }
            unsigned char magnitude = sqrt(sumX * sumX + sumY * sumY);

            int resInd = (y * width + x) * 4;
            res[resInd] = magnitude;
            res[resInd + 1] = magnitude;
            res[resInd + 2] = magnitude;
            res[resInd + 3] = image[resInd + 3];
        }
    }
}


void color(unsigned char* image, int width, int height, int epsilon) {
    int color1 = rand() % (255 - epsilon * 2) + epsilon * 2;
    int color2 = rand() % (255 - epsilon * 2) + epsilon * 2;
    int color3 = rand() % (255 - epsilon * 2) + epsilon * 2;
    for(int y = 1; y < height - 1; y++) {
        for(int x = 1; x < width - 1; x++) {
            if(image[4 * (y * width + x)] < epsilon || image[4 * (y * width + x)] > 255-(epsilon*2)) {
                fill(image, x, y, color1, color2, color3, epsilon, width, height);
            }
            color1 = rand() % (255 - epsilon * 2) + epsilon * 2;
            color2 = rand() % (255 - epsilon * 2) + epsilon * 2;
            color3 = rand() % (255 - epsilon * 2) + epsilon * 2;
        }
    }
}

int main() {

	int w = 0, h = 0;
	srand(time(NULL));

	char *filename = "skull.png";
	char *picture = load_png_file(filename, &w, &h);

	if (picture == NULL) {
		printf("I can't read the picture %s. Error.\n", filename);
		return -1;
	}
    unsigned char *fin = malloc(sizeof(char)*w*h*4);

	for (i = 0; i < h * w * 4; i += 4) {
		char R,G,B,A;
		R = picture[i+0];
		G = picture[i+1];
		B = picture[i+2];
		A = picture[i+3];

        int grey = ((R + G + B + A)/4) % 255;
        fin[i]=grey;
        fin[i+1]=grey;
        fin[i+2]=grey;
        fin[i+3]=grey;
	}
	average(picture, fin, w, h);
    gaussian(fin, w, h);
    bord(fin, w, h);
    color(fin,w,h,25);
	free(picture);
    char* img = "finalskull.png";
    write_png_file(img, fin, w, h);
    free(fin);
	return 0;
}
