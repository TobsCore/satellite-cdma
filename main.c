#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int satelliteMap[24][2] = {
        {2, 6},
        {3, 7},
        {4, 8},
        {5, 9},
        {1, 9},
        {2, 10},
        {1, 8},
        {2, 9},
        {3, 10},
        {2, 3},
        {3, 4},
        {5, 6},
        {6, 7},
        {7, 8},
        {8, 9},
        {9, 10},
        {1, 4},
        {2, 5},
        {3, 6},
        {4, 7},
        {5, 8},
        {6, 9},
        {1, 3},
        {4, 6}};

char *readFile(char *filename) {
    FILE *fp;
    size_t lSize;
    char *buffer;

    fp = fopen(filename, "rb");
    if (!fp) perror(filename), exit(1);

    fseek(fp, 0L, SEEK_END);
    lSize = (size_t) ftell(fp);
    rewind(fp);

    /* allocate memory for entire content */
    buffer = calloc(1, lSize + 1);
    if (!buffer) fclose(fp), fputs("memory alloc fails", stderr), exit(1);

    /* copy the file into the buffer */
    if (1 != fread(buffer, lSize, 1, fp))
        fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

    fclose(fp);
    free(buffer);
    return buffer;
}


/**
 * Places the char representation of numbers, that have been read from the file, into an integer array.
 * @param contents The contents of the file.
 * @param gpsSequence The integer array that is used to hold the numbers.
 */
void convertFileContentToIntegerArray(char *contents, int *gpsSequence) {
    char *tokenizedFileContents = strtok(contents, " ");


    int i = 0;
    while (tokenizedFileContents != NULL) {
        gpsSequence[i] = atoi(tokenizedFileContents);
        i++;
        tokenizedFileContents = strtok(NULL, " ");
    }
}

int getBitValueAtPosition(int bitSequence, int position) {
    int shifter = 10 - position;
    int filter = 1 << shifter;

    return (bitSequence & filter) >> shifter;
}

int getBitValueAtPositionForSatellite(int satelliteID, int bitSequence, int positionIdentifier) {
    int position = satelliteMap[satelliteID - 1][positionIdentifier];
    return getBitValueAtPosition(bitSequence, position);
}

int calcNumbersFromBelow(int satelliteID, int below) {
    int value1 = getBitValueAtPositionForSatellite(satelliteID, below, 0);
    int value2 = getBitValueAtPositionForSatellite(satelliteID, below, 1);

    return value1 ^ value2;
}

int nextChipSequenceBit(int satelliteId, int above, int below) {
    //Only take the last bit from the above sequence
    int fromAbove = above & 0b1;
    int fromBelow = calcNumbersFromBelow(satelliteId, below);

    return fromAbove ^ fromBelow;
}

int generateSequence(int bitSequence, int firstBit) {

    // Mover the sequence on step to the right
    bitSequence = bitSequence >> 1;
    // Place the calculated bit at the beginning of the sequence.
    return bitSequence | firstBit;
}

int moveBitForward(int firstbit) {
    return firstbit << 9;
}

int generateAboveSequence(int bitSequence) {
    int firstBit = getBitValueAtPosition(bitSequence, 3) ^ getBitValueAtPosition(bitSequence, 10);
    int firstBitMovedForward = moveBitForward(firstBit);

    return generateSequence(bitSequence, firstBitMovedForward);
}

int generateBelowSequence(int bitSequence) {
    int firstbit = getBitValueAtPosition(bitSequence, 2) ^getBitValueAtPosition(bitSequence, 3) ^
                   getBitValueAtPosition(bitSequence, 6) ^getBitValueAtPosition(bitSequence, 8) ^
                   getBitValueAtPosition(bitSequence, 9) ^getBitValueAtPosition(bitSequence, 10);
    int firstBitMovedForward = moveBitForward(firstbit);
    return generateSequence(bitSequence, firstBitMovedForward);
}

int generateChipSequence(int satelliteID, int above, int below, int cycleCount, int* resultArray) {
    if (cycleCount >= 1023) {
        //Abort!
        return -1;
    }

    int returnValue = nextChipSequenceBit(satelliteID, above, below);
    int nextAbove = generateAboveSequence(above);
    int nextBelow = generateBelowSequence(below);


    resultArray[cycleCount] = returnValue;
    return generateChipSequence(satelliteID, nextAbove, nextBelow, ++cycleCount, resultArray);
}

int main(int argn, char *argv[]) {

    if (argn <= 1) {
        printf("Programm parameter missing: Please add a file, that should be read\n");
        return -1;
    }
    char *fileContents = readFile(argv[1]);
    int gpsSequence[1023];
    convertFileContentToIntegerArray(fileContents, gpsSequence);

    int defaultConfig_1 = 0b1111111111;
    int defaultConfig_2 = 0b1111111111;


    int satelliteChipSequences[24][1023];

    for (int i = 0; i < 24; i++) {
        generateChipSequence(i + 1, defaultConfig_1, defaultConfig_2, 0, satelliteChipSequences[i]);

    }


    for (int i = 0; i < 24; i++) {
        printf("Satellite %2d: ", i + 1);
        for (int j = 0; j < 1023; j++) {
            printf("%d", satelliteChipSequences[i][j]);
        }
        printf("\n");
    }

}