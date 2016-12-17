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

/**
 * Is used to read the file's contents from the given file.
 * @param filename The name of the file.
 * @return The file's contents.
 */
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
 * @return Returns the gps sequence.
 */
int *convertFileContentToIntegerArray(char *contents) {
    char *splitFileContents = strtok(contents, " ");
    int *resultSequence = malloc(1023 * sizeof(int));

    int i = 0;
    while (splitFileContents != NULL) {
        resultSequence[i] = atoi(splitFileContents);
        i++;
        splitFileContents = strtok(NULL, " ");
    }
    return resultSequence;
}

/**
 * Returns the bit value of the given bit sequence and the given position.
 * @param bitSequence The bit sequence. It is expected to be 10 bits long.
 * @param position The position of the bit. The left-most bit is 1.
 * @return The bit value at the given position.
 */
int getBitValueAtPosition(int bitSequence, int position) {
    int shifter = 10 - position;
    int filter = 1 << shifter;

    return (bitSequence & filter) >> shifter;
}

/**
 *  Returns the bit value of the given bit sequence for a given satellite (identified by its id). In order to know
 *  whether the first or the second bit should be returned, the positionsIdentifier is needed.
 * @param satelliteID The id of the satellite. This is needed in order to decide which bits to return.
 * @param bitSequence The bit sequence for which the bit should be returned.
 * @param positionIdentifier The position identifier. This is either 0 or 1.
 * @return The bit value at the given position.
 */
int getBitValueAtPositionForSatellite(int satelliteID, int bitSequence, int positionIdentifier) {
    int position = satelliteMap[satelliteID - 1][positionIdentifier];
    return getBitValueAtPosition(bitSequence, position);
}

/**
 * Calculates the result of the below bit sequence. This takes into consideration, from which position those bits should
 * be retrieved.
 * @param satelliteID The satellite id. This is needed in order to figure out, which bits should be used to calculate
 * the result.
 * @param bitSequence The below bit sequence.
 * @return The calculated value from the below bit sequence.
 */
int calcNumbersFromBelow(int satelliteID, int bitSequence) {
    int value1 = getBitValueAtPositionForSatellite(satelliteID, bitSequence, 0);
    int value2 = getBitValueAtPositionForSatellite(satelliteID, bitSequence, 1);

    return value1 ^ value2;
}

/**
 * Calculates the next bit for the result bit sequence. It takes the bit from above and the calculated result from
 * below and calculates the result from those two inputs.
 * @param satelliteId The satellite id. This is needed in order to decide which bits to select from the below bit
 * sequence.
 * @param above The above bit sequence.
 * @param below The below bit sequence.
 * @return The calculated bit.
 */
int nextChipSequenceBit(int satelliteId, int above, int below) {
    //Only take the last bit from the above sequence
    int fromAbove = getBitValueAtPosition(above, 10);
    int fromBelow = calcNumbersFromBelow(satelliteId, below);

    return fromAbove ^ fromBelow;
}

/**
 * Generates the next bit sequence. This is done by shifting the existing bit sequence one bit to the right and then
 * placing the new first bit at the beginning of the bit sequence.
 * @param bitSequence The existing bit sequence.
 * @param firstBit The new first bit, which will be placed at the beginning of the new bit sequence.
 * @return The newly generated bit sequence
 */
int generateSequence(int bitSequence, int firstBit) {

    // Move the sequence one step to the right
    bitSequence = bitSequence >> 1;
    // Place the calculated bit at the beginning of the sequence.
    return bitSequence | firstBit;
}

/**
 * Moves the bit to first position.
 * @param firstBit The bit, which will be moved to the front.
 * @return The value, where the bits has been moved to the front.
 */
int moveBitForward(int firstBit) {
    return firstBit << 9;
}

/**
 * Is used to calculate the bit sequence for the above mother sequence.
 * @param bitSequence The bit sequence of the above sequence.
 * @return The generated bit.
 */
int generateAboveSequence(int bitSequence) {
    int firstBit = getBitValueAtPosition(bitSequence, 3) ^getBitValueAtPosition(bitSequence, 10);
    int firstBitMovedForward = moveBitForward(firstBit);

    return generateSequence(bitSequence, firstBitMovedForward);
}

/**
 * Is used to calculate the bit sequence for the below mother sequence.
 * @param bitSequence The bit sequence of the below sequence.
 * @return The generated bit.
 */
int generateBelowSequence(int bitSequence) {
    int firstBit = getBitValueAtPosition(bitSequence, 2) ^getBitValueAtPosition(bitSequence, 3) ^
                   getBitValueAtPosition(bitSequence, 6) ^getBitValueAtPosition(bitSequence, 8) ^
                   getBitValueAtPosition(bitSequence, 9) ^getBitValueAtPosition(bitSequence, 10);
    int firstBitMovedForward = moveBitForward(firstBit);
    return generateSequence(bitSequence, firstBitMovedForward);
}


/**
 * This recursive method call is used to calculate each bit of the chip sequence and stores the calculated bit in the
 * array.
 * @param satelliteID The ID of the satellite (1 indexed), which is used to select the bits for the calculation.
 * @param above The above sequence. The default configuration should be passed.
 * @param below The below sequence. The default configuration should be passed.
 * @param cycleCount The cycle counter. This method should be called with 0
 * @param resultArray The result array, in which the numbers should be stored.
 * @return
 */
void recursiveChipSequenceCalculation(int satelliteID, int above, int below, int cycleCount, int *resultArray) {
    if (cycleCount >= 1023) {
        //Abort!
    } else {
        int returnValue = nextChipSequenceBit(satelliteID, above, below);
        int nextAbove = generateAboveSequence(above);
        int nextBelow = generateBelowSequence(below);

        resultArray[cycleCount] = returnValue;
        recursiveChipSequenceCalculation(satelliteID, nextAbove, nextBelow, ++cycleCount, resultArray);
    }
}

/**
 * Shifts the bit sequence by the given delta. In order to shift the bits and still generate a 1023 bit long sequence
 * the missing bits (at the end) will be filled up with bits from the beginning. This can be done, because the signal
 * the gps satellite sent is repeated several times.
 * @param sequence The sequence, that will be shifted by the given value.
 * @param delta The delta, by which the sequence will be shifted.
 * @return A newly created sequence, in which the bits are shifted by the given delta.
 */
int *shiftSequenceByDelta(int *sequence, int delta) {
    int *shiftedSequence = malloc(1023 * sizeof(int));
    for (int i = 0; i < 1023; i++) {
        shiftedSequence[i] = sequence[(i + delta) % 1023];
    }

    return shiftedSequence;
}


/**
 * Generates the chip sequence (mother sequence) for a given satellite. This is a recursive method call, which places
 * the generated numbers in the given array.
 * @param satelliteID The ID of the satellite (1 indexed), which is used to select the bits for the calculation.
 * @param above The above sequence. The default configuration should be passed.
 * @param below The below sequence. The default configuration should be passed.
 * @param cycleCount The cycle counter. This method should be called with 0.
 * @param resultArray The result array, in which the numbers should be stored.
  */
void generateChipSequence(int satelliteID, int *resultArray) {
    int defaultConfigAbove = 0b1111111111;
    int defaultConfigBelow = 0b1111111111;
    int cycleStart = 0;

    recursiveChipSequenceCalculation(satelliteID, defaultConfigAbove, defaultConfigBelow, cycleStart, resultArray);
}

int sequenceMatchesResult2(int *sequence, int *resultSequence) {
    for (int i = 0; i < 1023; i++) {

        int resultValue = resultSequence[i];
        int bit = sequence[i];

        if (resultValue == 4 && bit != 0) {
            return 0;
        } else if (resultValue == -4 && bit != 1) {
            return 0;
        }
    }
    return 1;
}

int sequenceMatchesResult(int *sequence, int *resultSequence) {
    for (int i = 0; i < 1023; i++) {

        int resultValue = resultSequence[i];
        int bit = sequence[i];

        if (resultValue == 4 && bit != 1) {
            return 0;
        } else if (resultValue == -4 && bit != 0) {
            return 0;
        }
    }
    return 1;
}

int balbliblu(int *sequence, int *resultSequence) {
    int result1 = sequenceMatchesResult(sequence, resultSequence);
    if (result1) {
        return 1;
    }

    int result2 = sequenceMatchesResult2(sequence, resultSequence);
    if (result2) {
        return 0;
    }

    return -1;

}

int main(int argn, char *argv[]) {

    if (argn <= 1) {
        printf("Program parameter missing: Please add a file, that should be read\n");
        return -1;
    }
    char *fileContents = readFile(argv[1]);
    int *gpsSequence = convertFileContentToIntegerArray(fileContents);

    int satelliteChipSequences[24][1023];

    for (int i = 0; i < 24; i++) {
        generateChipSequence(i + 1, satelliteChipSequences[i]);
    }


    for (int satelliteID = 0; satelliteID < 24; satelliteID++) {
        printf("Satellite %2d: ", satelliteID + 1);
        for (int j = 0; j < 1023; j++) {
            printf("%d", satelliteChipSequences[satelliteID][j]);
        }
        printf("\n");

        for (int delta = 0; delta < 1023; delta++) {


            int *shiftedSequence = shiftSequenceByDelta(satelliteChipSequences[satelliteID], delta);
            int resultbit = balbliblu(shiftedSequence, gpsSequence);
            if (resultbit != -1) {
                printf("FOUND! Delta %d\n", delta);
            }
/*
            printf("\t Shifted: ");
            for (int j = 0; j < 1023; j++) {
                printf("%d", shiftedSequence[j]);
            }
            printf("\n");
            */
        }
    }

}