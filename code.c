#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEST_CASES 10
#define MAX_LINE_LENGTH 1024

// Previous functions remain the same (validate_line, compress_file, decompress_file)

int run_tests(const char *test_file, int is_compression) {
    FILE *fp = fopen(test_file, "r");
    if (!fp) {
        printf("Error: Cannot open test file\n");
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    char temp_input[] = "temp_input.txt";
    char temp_output[] = "temp_output.txt";
    char temp_expected[] = "temp_expected.txt";
    int test_count = 0;
    int passed = 0;

    while (fgets(line, sizeof(line), fp) && test_count < MAX_TEST_CASES) {
        if (line[0] == '#') {
            printf("\nRunning %s Test Case %d:\n", 
                   is_compression ? "Compression" : "Decompression", 
                   test_count + 1);
            
            FILE *temp = fopen(temp_input, "w");
            while (fgets(line, sizeof(line), fp) && line[0] != '-') {
                fputs(line, temp);
            }
            fclose(temp);

            temp = fopen(temp_expected, "w");
            while (fgets(line, sizeof(line), fp) && line[0] != '=') {
                fputs(line, temp);
            }
            fclose(temp);

            int success;
            if (is_compression) {
                success = compress_file(temp_input, temp_output);
            } else {
                success = decompress_file(temp_input, temp_output);
            }

            if (success > 0) {
                FILE *out = fopen(temp_output, "r");
                FILE *exp = fopen(temp_expected, "r");
                int match = 1;

                while (fgets(line, sizeof(line), out)) {
                    char expected[MAX_LINE_LENGTH];
                    if (!fgets(expected, sizeof(expected), exp) || strcmp(line, expected) != 0) {
                        match = 0;
                        break;
                    }
                }

                fclose(out);
                fclose(exp);

                if (match) {
                    passed++;
                    printf("Test case %d: PASSED\n", test_count + 1);
                } else {
                    printf("Test case %d: FAILED\n", test_count + 1);
                    printf("Expected output:\n");
                    FILE *exp = fopen(temp_expected, "r");
                    while (fgets(line, sizeof(line), exp)) {
                        printf("%s", line);
                    }
                    fclose(exp);
                    
                    printf("\nActual output:\n");
                    FILE *out = fopen(temp_output, "r");
                    while (fgets(line, sizeof(line), out)) {
                        printf("%s", line);
                    }
                    fclose(out);
                    printf("\n");
                }
            }
            test_count++;
        }
    }

    fclose(fp);
    remove(temp_input);
    remove(temp_output);
    remove(temp_expected);

    printf("\nTest Results: %d/%d passed\n", passed, test_count);
    return passed == test_count;
}

int main(int argc, char *argv[]) {
    if (argc == 3) {
        if (strcmp(argv[1], "-test-compress") == 0) {
            return run_tests(argv[2], 1) ? 0 : 1;
        }
        if (strcmp(argv[1], "-test-decompress") == 0) {
            return run_tests(argv[2], 0) ? 0 : 1;
        }
    }

    if (argc != 4) {
        printf("Usage: %s <mode> input_file output_file\n", argv[0]);
        printf("Modes: -compress, -decompress, -test-compress, -test-decompress\n");
        return 1;
    }

    if (strcmp(argv[1], "-compress") == 0) {
        int compressed_length = compress_file(argv[2], argv[3]);
        if (compressed_length > 0) {
            printf("Compression successful\n");
            printf("Compressed length: %d\n", compressed_length);
        }
        else
            printf("Error in compression process.\n");
    } else if (strcmp(argv[1], "-decompress") == 0) {
        int decompressed_length = decompress_file(argv[2], argv[3]);
        if (decompressed_length > 0) {
            printf("Decompression successful\n");
            printf("Decompressed length: %d\n", decompressed_length);
        }
        else
            printf("Error in decompression process.\n");
    } else {
        printf("Invalid mode. Use -compress, -decompress, -test-compress, or -test-decompress\n");
        return 1;
    }

    return 0;
}
