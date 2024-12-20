#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEST_CASES 10
#define MAX_LINE_LENGTH 1024

int validate_line(const char *line) {
    const char *ptr = line;
    
    while (*ptr) {
        while (*ptr == ' ') ptr++;
        if (!*ptr || *ptr == '\n') break;
        
        if (!isprint(*ptr)) return 0;
        ptr++;
        
        if (*ptr != ' ') return 0;
        ptr++;
        
        if (!isdigit(*ptr)) return 0;
        while (isdigit(*ptr)) ptr++;
        
        if (*ptr && *ptr != ' ' && *ptr != '\n') return 0;
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
    int first_in_line = 1;
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
                if (!first_in_line) fprintf(output_file, " ");
                fprintf(output_file, "%c %d", prev_char, count);
                compressed_length += 2;
                first_in_line = 0;
            }
            if (current_char == '\n') {
                fprintf(output_file, "\n");
                first_in_line = 1;
            }
            count = 1;
            prev_char = current_char;
        }
    }

    if (prev_char != EOF && prev_char != '\n') {
        if (!first_in_line) fprintf(output_file, " ");
        fprintf(output_file, "%c %d", prev_char, count);
        compressed_length += 2;
    }

    fclose(input_file);
    fclose(output_file);
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
    char current_char;
    int count;
    int decompressed_length = 0;

    while (fgets(line, sizeof(line), input_file)) {
        if (!validate_line(line)) {
            fclose(input_file);
            fclose(output_file);
            return 0;
        }

        char *ptr = line;
        int is_empty_line = 1;

        while (*ptr && sscanf(ptr, " %c %d", &current_char, &count) == 2) {
            is_empty_line = 0;
            for (int i = 0; i < count; i++) {
                fputc(current_char, output_file);
                decompressed_length++;
            }
            
            while (*ptr && *ptr != ' ') ptr++;
            while (*ptr && *ptr == ' ') ptr++;
            while (*ptr && isdigit(*ptr)) ptr++;
        }
        
        if (!is_empty_line) fputc('\n', output_file);
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
            printf("\nRunning Test Case %d:\n", test_count + 1);
            
            FILE *temp_in = fopen(temp_input, "w");
            FILE *temp_exp = fopen(temp_expected, "w");
            int input_phase = 1;
            
            while (fgets(line, sizeof(line), fp)) {
                if (line[0] == '=') break;
                if (strncmp(line, "# Test Case", 10) == 0 || strlen(line) <= 1) continue;
                
                if (input_phase) {
                    fputs(line, temp_in);
                } else {
                    fputs(line, temp_exp);
                }
                
                char next_line[MAX_LINE_LENGTH];
                long pos = ftell(fp);
                if (fgets(next_line, sizeof(next_line), fp)) {
                    fseek(fp, pos, SEEK_SET);
                    if (next_line[0] != '=' && strncmp(next_line, "# Test Case", 10) != 0) continue;
                }
                input_phase = 0;
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
