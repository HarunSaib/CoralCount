//
// Created by harun on 11/28/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"
#include <stdio.h>

void increaseContrast(char *inputFilename, char *outputFilename, double contrastFactor);

int main(int argc, const char * argv[]) {
    char inputFilename[] = "C:/Users/harun/CLionProjects/CoralCount/CoralCount/Samples/Sample8.ppm";
    char outputFilename[] = "C:/Users/harun/CLionProjects/CoralCount/CoralCount/Outputs/SampleOutput.ppm";
    double contrastFactor = 1.5;  // Increase contrast by 50%

    increaseContrast(inputFilename, outputFilename, contrastFactor);

    printf("Program ends ... ");
    return 0;
}

void increaseContrast(char *inputFilename, char *outputFilename, double contrastFactor) {
    // Step 1: Read the image
    Image img = readImage(inputFilename);
    if (img.map == NULL) {
        fprintf(stderr, "Error: Could not read image file %s.\n", inputFilename);
        return;
    }

    // Step 2: Convert the image to a matrix
    Matrix intensityMatrix = image2Matrix(img);

    // Step 3: Apply contrast adjustment
    for (int i = 0; i < intensityMatrix.height; i++) {
        for (int j = 0; j < intensityMatrix.width; j++) {
            // Adjust contrast
            double newValue = 128 + contrastFactor * (intensityMatrix.map[i][j] - 128);
            // Clamp values to [0, 255]
            intensityMatrix.map[i][j] = MAX(0, MIN(255, newValue));
        }
    }

    // Step 4: Convert the modified matrix back to an image
    Image contrastedImage = matrix2Image(intensityMatrix, 0, 1.0);

    // Step 5: Write the new image to a file
    writeImage(contrastedImage, outputFilename);

    // Step 6: Free memory
    deleteImage(img);
    deleteMatrix(intensityMatrix);
    deleteImage(contrastedImage);
}