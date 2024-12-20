#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEST_CASES 10
#define MAX_LINE_LENGTH 1024

int validate_line(const char *line) {
    const char *ptr = line;
    while (*ptr) {
        if (!isprint(*ptr) && !isspace(*ptr)) return 0;
        ptr++;
    }
    return 1;
}

int compress_file(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) return 0;

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        fclose(input_file);
        return 0;
    }

    char current_char, prev_char;
    int count = 1;
    int first_char = 1;
    int compressed_length = 0;

    prev_char = fgetc(input_file);
    if (prev_char == EOF) {
        fclose(input_file);
        fclose(output_file);
        return 0;
    }

    
    while ((current_char = fgetc(input_file)) != EOF) {
        if (current_char == prev_char && current_char != '\n') {
            count++;
        } else {
            if (prev_char != '\n') {
                if (!first_char) fprintf(output_file, " ");
                fprintf(output_file, "%c %d", prev_char, count);
                compressed_length += count;
                first_char = 0;
            } else {
                // Only print newline if not at end of file
                if (current_char != EOF) {
                    fprintf(output_file, "\n");
                    first_char = 1;
                }
            }
            count = 1;
            prev_char = current_char;
        }
    }

    // Handle last character without adding extra newline
    if (prev_char != '\n' && prev_char != EOF) {
        if (!first_char) fprintf(output_file, " ");
        fprintf(output_file, "%c %d", prev_char, count);
        compressed_length += count;
    }

    return compressed_length;
}

int decompress_file(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) return 0;

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        fclose(input_file);
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    int decompressed_length = 0;

    while (fgets(line, sizeof(line), input_file)) {
        char *ptr = line;
        char current_char;
        int count;

        while (*ptr && sscanf(ptr, "%c %d", &current_char, &count) == 2) {
            for (int i = 0; i < count; i++) {
                fputc(current_char, output_file);
                decompressed_length++;
            }
            
            // Move pointer past current character and count
            while (*ptr && *ptr != ' ') ptr++;
            if (*ptr) ptr++;
            while (*ptr && isdigit(*ptr)) ptr++;
            if (*ptr) ptr++;
        }
        
        if (*(ptr-1) != '\n') {
            fputc('\n', output_file);
        }
    }

    fclose(input_file);
    fclose(output_file);
    return decompressed_length;
}


int run_tests(const char *test_file, int is_compression) {
    FILE *fp = fopen(test_file, "r");
    if (!fp) return 0;

    char line[MAX_LINE_LENGTH];
    char temp_input[] = "temp_input.txt";
    char temp_output[] = "temp_output.txt";
    char temp_expected[] = "temp_expected.txt";
    int test_count = 0;
    int passed = 0;
    
    while (fgets(line, sizeof(line), fp) && test_count < MAX_TEST_CASES) {
        if (strncmp(line, "# Test Case", 10) == 0) {
            FILE *temp_in = fopen(temp_input, "w");
            FILE *temp_exp = fopen(temp_expected, "w");
            int input_section = 1;
            
            while (fgets(line, sizeof(line), fp)) {
                // Skip comments and empty lines
                if (line[0] == '#' || line[0] == '\n') continue;
                
                // Check for end of test case
                if (line[0] == '=') break;
                
                // Write to appropriate file
                if (input_section) {
                    fputs(line, temp_in);
                } else {
                    fputs(line, temp_exp);
                }
                
                // Check next line to determine section
                char peek[MAX_LINE_LENGTH];
                long pos = ftell(fp);
                if (fgets(peek, sizeof(peek), fp)) {
                    fseek(fp, pos, SEEK_SET);
                    if (peek[0] != '=' && !isspace(peek[0])) continue;
                }
                input_section = 0;
            }
            
            fclose(temp_in);
            fclose(temp_exp);

            int success = is_compression ? 
                         compress_file(temp_input, temp_output) : 
                         decompress_file(temp_input, temp_output);

            if (success > 0) {
                FILE *out = fopen(temp_output, "r");
                FILE *exp = fopen(temp_expected, "r");
                int match = 1;

                while (fgets(line, sizeof(line), out)) {
                    char expected[MAX_LINE_LENGTH];
                    if (!fgets(expected, sizeof(expected), exp) || 
                        strcmp(line, expected) != 0) {
                        match = 0;
                        break;
                    }
                }

                // Check if expected file has more lines
                if (fgets(line, sizeof(line), exp)) match = 0;

                fclose(out);
                fclose(exp);

                if (match) {
                    passed++;
                    printf("Test case %d: PASSED\n", test_count + 1);
                } else {
                    printf("Test case %d: FAILED\n", test_count + 1);
                    printf("Expected output:\n");
                    FILE *exp = fopen(temp_expected, "r");
                    while (fgets(line, sizeof(line), exp)) printf("%s", line);
                    fclose(exp);
                    
                    printf("\nActual output:\n");
                    FILE *out = fopen(temp_output, "r");
                    while (fgets(line, sizeof(line), out)) printf("%s", line);
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
        if (strcmp(argv[1], "-test-compress") == 0) return run_tests(argv[2], 1) ? 0 : 1;
        if (strcmp(argv[1], "-test-decompress") == 0) return run_tests(argv[2], 0) ? 0 : 1;
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
    } else if (strcmp(argv[1], "-decompress") == 0) {
        int decompressed_length = decompress_file(argv[2], argv[3]);
        if (decompressed_length > 0) {
            printf("Decompression successful\n");
            printf("Decompressed length: %d\n", decompressed_length);
        }
    } else {
        printf("Invalid mode. Use -compress, -decompress, -test-compress, or -test-decompress\n");
        return 1;
    }

    return 0;
}
