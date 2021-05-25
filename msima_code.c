#include <stdio.h>

/**
 * Adapted from Mihai Sima's slides.
 *
 * Note: since we are working with 32-bit integers, but only care
 * about 13 bits, we can't simply check if `sample < 0`, since the
 * parity bit is at position 13, not 31.
 */
int signum(int sample) {
    if ((sample & 0x2000) == 0) {
        return 0;
    } else {
        return 1;
    }
    /*
    if (sample < 0)
        return 0; // sign is 0 for negative samples
    else
        return 1; // sign is 1 for positive samples
    */
}

/**
 * Adapted from Mihai Sima's slides.
 * 
 * Note: since we are working with 32-bits, but only care about
 * 13 bits, we can't simply `return -sample` as per slides.
 *
 * SH: I may be circumventing 2's-complement here though.. (2021-05-25)
 */
int magnitude (int sample) {
    return sample & 0x1FFF;
    /*
    if (sample < 0) {
        return -sample;
    } else {
        return sample;
    }
    */
}

/**
 * Adapted from Mihai Sima's slides.
 *
 * Input: sample_magnitude (integer with 13 relevant bits)
 *        sign (integer with 1 relevant bit - 0 = positive)
 * Output: char describing the compressed input as per mu-law encoding.
 */
char codeword_compression (unsigned int sample_magnitude, int sign) {
    int chord, step;
    int codeword_tmp;

    if (sample_magnitude & (1 << 12)) {
        chord = 0x7;
        step = (sample_magnitude >> 8) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;

        return (char) codeword_tmp;
    }
    if (sample_magnitude & (1 << 11)) {
        chord = 0x6;
        step = (sample_magnitude >> 7) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;

        return (char) codeword_tmp;
    }
    if (sample_magnitude & (1 << 10)) {
        chord = 0x5;
        step = (sample_magnitude >> 6) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;

        return (char) codeword_tmp;
    }
    if (sample_magnitude & (1 << 9)) {
        chord = 0x4;
        step = (sample_magnitude >> 5) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;

        return (char) codeword_tmp;
    }
    if (sample_magnitude & (1 << 8)) {
        chord = 0x3;
        step = (sample_magnitude >> 4) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;

        return (char) codeword_tmp;
    }
    if (sample_magnitude & (1 << 7)) {
        chord = 0x2;
        step = (sample_magnitude >> 3) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;

        return (char) codeword_tmp;
    }
    if (sample_magnitude & (1 << 6)) {
        chord = 0x1;
        step = (sample_magnitude >> 2) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;

        return (char) codeword_tmp;
    }
    if (sample_magnitude & (1 << 5)) {
        chord = 0x0;
        step = (sample_magnitude >> 1) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;

        return (char) codeword_tmp;
    }
}

/**
 * Simple assert function to aid the test() in printing test detail table.
 */
char* assert(int calculated, int expected) {
    if (calculated == expected) {
        return "PASS";
    } else {
        return "FAIL";
    }
}

/**
 * Run test cases on decomposing input values into sign, chord, step, discarded.
 * Results are verified and displayed in a detail table.
 *
 * Results are printed to stdout, with a detail table printed to stderr.
 */
void test() {               // test value    , sign, chord,   step, discarded
                            // 32109876543210
    int test_values[][5] = {{0b10000000100011,    1,     0, 0b0001, 0b1},
                            {0b00000001001011,    0,     1, 0b0010, 0b11},
                            {0b00000010011111,    0,     2, 0b0011, 0b111},
                            {0b10000101001111,    1,     3, 0b0100, 0b1111},
                            {0b10001010111111,    1,     4, 0b0101, 0b11111},
                            {0b10010110111111,    1,     5, 0b0110, 0b111111},
                            {0b00101111111111,    0,     6, 0b0111, 0b1111111},
                            {0b11100011111111,    1,     7, 0b1000, 0b11111111}
                           };
    int test_size = 8;
                         
    int test_val, expect_sign, expect_chord, expect_step, expect_discarded, expect_magnitude;
    int calculated_sign, calculated_magnitude, calculated_chord, calculated_step, calculated_discarded;
    char calculated_codeword;
    for (int i = 0; i < test_size; i++) {
        test_val         = test_values[i][0];
        expect_sign      = test_values[i][1];
        expect_chord     = test_values[i][2];
        expect_step      = test_values[i][3];
        expect_discarded = test_values[i][4];
        expect_magnitude = test_val & 0x1FFF;

        calculated_sign = signum(test_val);
        calculated_magnitude = magnitude(test_val);
        calculated_codeword = codeword_compression (calculated_magnitude, calculated_sign);
        calculated_step = calculated_codeword & 0xF;
        calculated_chord = (calculated_codeword >> 4) & 0x7;
        calculated_discarded = test_val - 
            ((calculated_sign << 13) | 
             (1 << (5 + calculated_chord)) | 
             (calculated_step << (1 + calculated_chord)));

        fprintf (stderr, "===Test %d. test_val = %#x===\n", i+1, test_val);
        fprintf (stderr, "    Field | Calc value | Expect Value | PASS?\n");
        fprintf (stderr, "----------|------------|--------------|------\n");
        fprintf (stderr, "     Sign | %10d | %12d | %s\n", calculated_sign, expect_sign, assert(calculated_sign, expect_sign));
        fprintf (stderr, "Magnitude | %#10x | %#12x | %s\n", calculated_magnitude, expect_magnitude, assert(calculated_magnitude, expect_magnitude));
        fprintf (stderr, "    Chord | %10d | %12d | %s\n", calculated_chord, expect_chord, assert(calculated_chord, expect_chord));
        fprintf (stderr, "     Step | %#10x | %#12x | %s\n", calculated_step, expect_step, assert(calculated_step, expect_step));
        fprintf (stderr, "Discarded | %#10x | %#12x | %s\n", calculated_discarded, expect_discarded, assert(calculated_discarded, expect_discarded));

        if (calculated_codeword ==
                (char) ((expect_sign << 7) | (expect_chord << 4) | expect_step)) {
            fprintf(stdout, "Test %d passed.\n", i+1);
        }
    }
}

int main() {
    test();
}
