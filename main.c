//
// Created by harun on 11/28/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"
#include <stdio.h>


void shrinkImage(Image *img);
void expandImage(Image *img);
// Function to apply Sobel edge detection
Matrix sobelEdgeDetection(Image img);
// Function to enhance contrast of an image by a given factor
Image enhanceEdges(Image img, double contrastFactor);
// Function to apply a binary threshold to an image based on intensity
void applyEdgeThreshold(Image *img, unsigned char threshold);
// Function to clean up edges using morphological operations (shrink and expand)
void cleanUpEdges(Image *img);

Image averageNeighborPixels(Image img, int size);
Image canny(Image img);

int main(int argc, const char * argv[]) {
    char inputFilename[] = "/Users/sachinkaul/Documents/GitHub/CoralCount/Samples/Sample8.ppm";
    char outputFilename[] = "/Users/sachinkaul/Documents/GitHub/CoralCount/Outputs/SampleOutput.ppm";

    // Step 1: Read the input image
    Image img = readImage(inputFilename);
    if (img.map == NULL) {
        fprintf(stderr, "Error: Could not read image file %s.\n", inputFilename);
        return 1;
    }

    //Smooth the image
    Image smoothed = averageNeighborPixels(img, 2);

    // Step 2: Apply Sobel edge detection
    /*Matrix edgeMatrix = sobelEdgeDetection(img);

    // Convert edge matrix back to image
    Image edgeImage = matrix2Image(edgeMatrix, 0, 1.0);
    deleteMatrix(edgeMatrix);*/

    //Step 2.5: Apply Canny Edge Detector
    Image edgeImage = canny(smoothed);
    writeImage(edgeImage, "/Users/sachinkaul/Documents/GitHub/CoralCount/Outputs/testOutput1.ppm");

    // Step 3: Enhance the contrast of the edge image
    double contrastFactor = 2.0; // Experiment with higher contrast
    Image highContrastImage = enhanceEdges(edgeImage, contrastFactor);

    // Step 4: Apply a lower threshold to make the edges more distinct
    unsigned char threshold = 50; // Lower threshold to capture more edges
    applyEdgeThreshold(&highContrastImage, threshold);

    // Step 5: Clean up the edges using morphological operations
    cleanUpEdges(&highContrastImage);

    // Step 6: Write the processed image to a new file
    writeImage(highContrastImage, outputFilename);

    // Free memory
    deleteImage(img);
    deleteImage(smoothed);
    deleteImage(highContrastImage);

    printf("Program ends ... ");
    return 0;
}

void shrinkImage(Image *img) {
    Image temp = createImage(img->height, img->width);

    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            unsigned char minR = 255, minG = 255, minB = 255;

            // Check all 8 neighbors
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < img->height && nj >= 0 && nj < img->width) {
                        minR = MIN(minR, img->map[ni][nj].r);
                        minG = MIN(minG, img->map[ni][nj].g);
                        minB = MIN(minB, img->map[ni][nj].b);
                    }
                }
            }

            temp.map[i][j].r = minR;
            temp.map[i][j].g = minG;
            temp.map[i][j].b = minB;
            temp.map[i][j].i = (minR + minG + minB) / 3;  // Update intensity
        }
    }

    // Copy back the result
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            img->map[i][j] = temp.map[i][j];
        }
    }

    deleteImage(temp);
}

void expandImage(Image *img) {
    Image temp = createImage(img->height, img->width);

    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            unsigned char maxR = 0, maxG = 0, maxB = 0;

            // Check all 8 neighbors
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < img->height && nj >= 0 && nj < img->width) {
                        maxR = MAX(maxR, img->map[ni][nj].r);
                        maxG = MAX(maxG, img->map[ni][nj].g);
                        maxB = MAX(maxB, img->map[ni][nj].b);
                    }
                }
            }

            temp.map[i][j].r = maxR;
            temp.map[i][j].g = maxG;
            temp.map[i][j].b = maxB;
            temp.map[i][j].i = (maxR + maxG + maxB) / 3;  // Update intensity
        }
    }

    // Copy back the result
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            img->map[i][j] = temp.map[i][j];
        }
    }

    deleteImage(temp);
}

// Apply Sobel edge detection and scale the results
Matrix sobelEdgeDetection(Image img) {
    int gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}; // X-direction
    int gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};  // Y-direction

    Matrix edgeMatrix = createMatrix(img.height, img.width);

    for (int i = 1; i < img.height - 1; i++) {
        for (int j = 1; j < img.width - 1; j++) {
            double sumX = 0.0, sumY = 0.0;

            // Apply Sobel kernels
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    unsigned char intensity = img.map[i + k][j + l].i;
                    sumX += gx[k + 1][l + 1] * intensity;
                    sumY += gy[k + 1][l + 1] * intensity;
                }
            }

            // Calculate gradient magnitude and apply scaling
            double magnitude = sqrt(sumX * sumX + sumY * sumY);
            edgeMatrix.map[i][j] = (unsigned char) MIN(255, magnitude * 2); // Adjust scaling factor if necessary
        }
    }

    return edgeMatrix;
}

// Function to enhance the contrast of an image by adjusting intensity values
Image enhanceEdges(Image img, double contrastFactor) {
    Matrix intensityMatrix = image2Matrix(img);

    // Adjust contrast for each intensity value
    for (int i = 0; i < intensityMatrix.height; i++) {
        for (int j = 0; j < intensityMatrix.width; j++) {
            double newValue = 128 + contrastFactor * (intensityMatrix.map[i][j] - 128);
            intensityMatrix.map[i][j] = MAX(0, MIN(255, newValue));
        }
    }

    Image contrastedImage = matrix2Image(intensityMatrix, 0, 1.0);

    deleteMatrix(intensityMatrix);

    return contrastedImage;
}

// Apply a lower threshold to make more edges visible
void applyEdgeThreshold(Image *img, unsigned char threshold) {
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            unsigned char intensity = img->map[i][j].i;
            if (intensity >= threshold) {
                img->map[i][j].r = 255;
                img->map[i][j].g = 255;
                img->map[i][j].b = 255;
                img->map[i][j].i = 255;
            } else {
                img->map[i][j].r = 0;
                img->map[i][j].g = 0;
                img->map[i][j].b = 0;
                img->map[i][j].i = 0;
            }
        }
    }
}

void cleanUpEdges(Image *img) {
    shrinkImage(img); // Removes small noise
    expandImage(img); // Expands edges to restore structure
}

Image averageNeighborPixels(Image img, int size) {
    if (size <= 0) return img; // No processing if size is invalid or zero.
    
    int i, j, m, n, count;
    int sumR, sumG, sumB, sumI;
    Image result = createImage(img.height, img.width); // Create a blank image with the same size.

    // Loop through each pixel in the image.
    for (i = 0; i < img.height; i++) {
        for (j = 0; j < img.width; j++) {
            sumR = sumG = sumB = sumI = count = 0;

            // Loop through the neighborhood defined by the size parameter.
            for (m = i - size; m <= i + size; m++) {
                for (n = j - size; n <= j + size; n++) {
                    // Ensure the neighboring pixel is within the image bounds.
                    if (m >= 0 && m < img.height && n >= 0 && n < img.width) {
                        sumR += img.map[m][n].r;
                        sumG += img.map[m][n].g;
                        sumB += img.map[m][n].b;
                        sumI += img.map[m][n].i;
                        count++;
                    }
                }
            }

            // Calculate the average for each color channel and intensity.
            result.map[i][j].r = sumR / count;
            result.map[i][j].g = sumG / count;
            result.map[i][j].b = sumB / count;
            result.map[i][j].i = sumI / count;
        }
    }

    return result;
}

int getMidpoint(int size){
    if(size % 2 == 0){
        return size / 2 - 1;
    } else {
        return size / 2;
    }
}

Matrix convolve(Matrix m1, Matrix m2){
    int anchorHeight = getMidpoint(m2.height);
    int anchorWidth = getMidpoint(m2.width);
    Matrix convolve = createMatrix(m1.height, m1.width);
    for(int x = 0; x < m1.width; x++){
        for (int y = 0; y < m1.height; y++) {
            if(x < anchorWidth || y < anchorHeight || x > m1.width - anchorWidth - 2 || y > m1.height - anchorHeight - 2){
                convolve.map[y][x] = 0;
            } else {
                //Perform Matrix calculation
                double sum = 0;
                int countwidth = 0;
                for(int i = x - anchorWidth; i < x - anchorWidth + m2.width; i++){
                    int countheight = 0;
                    for (int j = y - anchorHeight; j < y - anchorHeight + m2.height; j++) {
                        sum += m1.map[j][i] * m2.map[countheight][countwidth];
                        countheight++;
                    }
                    countwidth++;
                }
                convolve.map[y][x] = sum;
            }
        }
    }
    return convolve;
}

Image canny(Image img){
    Matrix input = image2Matrix(img);

    Matrix pfilter = createMatrix(2, 2);
    pfilter.map[0][0] = -0.5;
    pfilter.map[0][1] = -0.5;
    pfilter.map[1][0] = 0.5;
    pfilter.map[1][1] = 0.5;

    Matrix qfilter = createMatrix(2, 2);
    qfilter.map[0][0] = -0.5;
    qfilter.map[0][1] = 0.5;
    qfilter.map[1][0] = -0.5;
    qfilter.map[1][1] = 0.5;

    Matrix p = convolve(input, pfilter);
    Matrix q = convolve(input, qfilter);

    Matrix m = createMatrix(p.height, p.width);
    for(int y = 0; y < m.height; y++){
        for(int x = 0; x < m.width; x++){
            m.map[y][x] = sqrt((p.map[y][x] * p.map[y][x]) + (q.map[y][x] * q.map[y][x]));
        }     
    }

    Matrix a = createMatrix(p.height, p.width);
    for(int y = 0; y < m.height; y++){
        for(int x = 0; x < m.width; x++){
            a.map[y][x] = atan2(q.map[y][x], p.map[y][x]);
        }     
    }

    Matrix e = createMatrix(input.height, input.width);

    for(int y = 0; y < m.height; y++){
        for(int x = 0; x < m.width; x++){
            // Find direction
            int direction = 0;
            if(!(a.map[y][x] >= 337.5)){
                double approxAngle = a.map[y][x] / 45;
                approxAngle = fmod(approxAngle, 4.0);
                direction = round(approxAngle);
            }

            // Non maxima suppression
            e.map[y][x] = m.map[y][x];
            switch(direction){
                case 0: 
                    if(y > 0 && m.map[y][x] < m.map[y-1][x]){
                        e.map[y][x] = 0;
                    }
                    if(y < m.height - 1 && m.map[y][x] < m.map[y+1][x]){
                        e.map[y][x] = 0;
                    }
                    break;
                case 1:
                    if(y > 0 && x > 0 && m.map[y][x] < m.map[y-1][x-1]){
                        e.map[y][x] = 0;
                    }
                    if(y < m.height - 1 && x < m.width - 1 && m.map[y][x] < m.map[y+1][x+1]){
                        e.map[y][x] = 0;
                    }
                    break;
                case 2:
                    if(x > 0 && m.map[y][x] < m.map[y][x-1]){
                        e.map[y][x] = 0;
                    }
                    if(x < m.width - 1 && m.map[y][x] < m.map[y][x+1]){
                        e.map[y][x] = 0;
                    }
                    break;
                case 3:
                    if(y < m.height - 1 && x > 0 && m.map[y][x] < m.map[y+1][x-1]){
                        e.map[y][x] = 0;
                    }
                    if(y > 0 && x < m.width - 1 && m.map[y][x] < m.map[y-1][x+1]){
                        e.map[y][x] = 0;
                    }
                    break;
                default:
                    printf("Unknown value reached, NMS.\n");
                    break; 
            }
        }     
    }

    Matrix hysterisis = createMatrix(e.height, e.width);

    for(int y = 0; y < m.height; y++){
        for(int x = 0; x < m.width; x++){
            if(e.map[y][x] < 5){
                hysterisis.map[y][x] = 0;
            } else if(e.map[y][x] >= 5 && e.map[y][x] < 10){
                hysterisis.map[y][x] = 75;
            } else {
                hysterisis.map[y][x] = 255;
            }
        }
    }

    for(int y = 0; y < m.height; y++){
        for(int x = 0; x < m.width; x++){
            if(hysterisis.map[y][x] == 75){
                if(x > 0 && hysterisis.map[y][x-1] == 255){
                    hysterisis.map[y][x] = 255;
                }
                if(x > 0 && y < hysterisis.height - 1 && hysterisis.map[y+1][x-1] == 255){
                    hysterisis.map[y][x] = 255;
                }
                if(x > 0 && y > 0 && hysterisis.map[y-1][x-1] == 255){
                    hysterisis.map[y][x] = 255;
                }
                if(y < hysterisis.height - 1 && hysterisis.map[y+1][x] == 255){
                    hysterisis.map[y][x] = 255;
                }
                if(y > 0 && hysterisis.map[y-1][x] == 255){
                    hysterisis.map[y][x] = 255;
                }
                if(x < hysterisis.width - 1 && hysterisis.map[y][x+1] == 255){
                    hysterisis.map[y][x] = 255;
                }
                if(x < hysterisis.width - 1 && y > 0 && hysterisis.map[y-1][x+1] == 255){
                    hysterisis.map[y][x] = 255;
                }
                if(x < hysterisis.width - 1 && y < hysterisis.height - 1 && hysterisis.map[y+1][x+1] == 255){
                    hysterisis.map[y][x] = 255;
                }
            }
        }
    }

    for(int y = 0; y < m.height; y++){
        for(int x = 0; x < m.width; x++){
            int count = 0;
            if(hysterisis.map[y][x] == 75){
                if(x > 0 && hysterisis.map[y][x-1] == 255){
                    count++;
                }
                if(x > 0 && y < hysterisis.height - 1 && hysterisis.map[y+1][x-1] == 255){
                    count++;
                }
                if(x > 0 && y > 0 && hysterisis.map[y-1][x-1] == 255){
                    count++;
                }
                if(y < hysterisis.height - 1 && hysterisis.map[y+1][x] == 255){
                    count++;
                }
                if(y > 0 && hysterisis.map[y-1][x] == 255){
                    count++;
                }
                if(x < hysterisis.width - 1 && hysterisis.map[y][x+1] == 255){
                    count++;
                }
                if(x < hysterisis.width - 1 && y > 0 && hysterisis.map[y-1][x+1] == 255){
                    count++;
                }
                if(x < hysterisis.width - 1 && y < hysterisis.height - 1 && hysterisis.map[y+1][x+1] == 255){
                    count++;
                }
                if(count > 0){
                    hysterisis.map[y][x] = 255;
                } else {
                    hysterisis.map[y][x] = 0;
                }
            }
        }
    }

    Image finalEdges = matrix2Image(hysterisis, 0, 1.0);

    deleteMatrix(pfilter);
    deleteMatrix(qfilter);
    deleteMatrix(p);
    deleteMatrix(q);
    deleteMatrix(m);
    deleteMatrix(a);
    deleteMatrix(e);
    deleteMatrix(hysterisis);
    return finalEdges;
}
