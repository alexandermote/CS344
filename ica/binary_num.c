#include <stdio.h>
#include <string.h>
#include <stdint.h>

char *show_bin(int32_t);

char *show_bin(int32_t input_value) {
    static char bin_val[100] = {0};
    uint32_t mask = 0x0;
    uint32_t format = 0;
    char *ch = NULL;

    memset(bin_val, 0, sizeof(bin_val));
    for (ch = bin_val, mask = (0x1 << 31), format = 0
             ; mask != 0
             ; mask = mask >> 1, format++, ch++) {
        if((format % 4) == 0  && (format != 0)) {
            *ch = ' ';
            ch++;
        }
        if (!(input_value & mask)) *ch = '0';
        else *ch = '1';
    }
    return(bin_val);
}

int main(int argc, char *argv[]) {
    int32_t input_value = -1;

    while(1) {
        printf("\nInput value to convert: ");
        scanf("%d", &input_value);
        if(input_value < 0) {
            printf("Negative number detected, exiting\n");
            break;
        }
        printf("The value is %s\n", (input_value % 2) ? "odd" : "even");
        printf("Decimal:\t%40d\n"
               "Octal:\t\t%40o\n"
               "Hexadecimal:\t%40x\n"
               , input_value
               , input_value
               , input_value);
        printf("Binary:\t\t%40s\n", show_bin(input_value));
    }

    return(0);
}
