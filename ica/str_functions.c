#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

size_t bstrlen(const char *s) {
    size_t len = 0;

    while (s[len] != '\0') {
        len++;
    }
    return len;
}

char *bstrcpy(char *dest, const char *src) {
    int index = 0;
       
    while (src[index] != '\0') {
        dest[index] = src[index];
        index++;
    }
    dest[index] = '\0';

    return dest;
}

char *bstrcat(char *dest, const char *src) {
    size_t len = bstrlen(dest);

    int index = 0;

    while (src[index] != '\0') {
        dest[len] = src[index];
        index++;
        len++;
    }
    dest[len] = '\0';

    return dest;
}

int main(int argc, char *argv[]) {
// bstrlen tests
    {
        char str0[5] = "";
        char str1[] = "123456789";
        char str2[50] = "123456";

        assert(bstrlen(str0) == strlen(str0));
        assert(bstrlen(str1) == strlen(str1));
        assert(bstrlen(str2) == strlen(str2));
        assert(bstrlen("abc") == strlen("abc"));
        printf("bstrlen passed all tests\n");
    }
// bstrcpy tests
    {
        char str0[5] = "";
        char str1[50] = "123456";
        char str2[50] = "123456";

        assert(strcmp(bstrcpy(str1, str0), strcpy(str2, str0)) == 0);
    }

    {
        char str1[] = "123456789";
        char str2[50] = "123456";
        char str3[50] = "123456";

        assert(strcmp(bstrcpy(str2, str1), strcpy(str3, str1)) == 0);
    }

    {
        char str1[5] = "";
        char str2[5] = "";

        assert(strcmp(bstrcpy(str1, "a"), strcpy(str2, "a")) == 0);
    }
    printf("bstrcpy passed all tests\n");
// bstrcat tests
    {
        char str1[] = "123456789";
        char str2[50] = "123456";
        char str3[50] = "123456";

        assert(strcmp(bstrcat(str2, str1), strcat(str3, str1)) == 0);
    }

    {
        char str0[5] = "";
        char str1[50] = "123456";
        char str2[50] = "123456";

        assert(strcmp(bstrcat(str1, str0), strcat(str2, str0)) == 0);
    }

    {
        char str1[50] = "123456";
        char str2[50] = "123456";

        assert(strcmp(bstrcat(str1, "a"), strcat(str2, "a")) == 0);
    }

    {
        char str1[50] = "123456";
        char str2[50] = "123456";

        assert(strcmp(bstrcat(str1, "abcdef"), strcat(str2, "abcdef")) == 0);
    }
    printf("bstrcat passed all tests\n");

    return EXIT_SUCCESS;
}
