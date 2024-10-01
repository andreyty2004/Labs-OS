#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>

#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"

// Функция для вывода строки с подсветкой совпадения
void print_with_highlight(char* string, char* pattern) 
{
    regex_t regex;
    regmatch_t match;
    int offset = 0;
    regcomp(&regex, pattern, REG_EXTENDED);
    while(regexec(&regex, string + offset, 1, &match, 0) == 0)
    {
        printf("%.*s", match.rm_so, string + offset);
        if(match.rm_so != -1 && match.rm_eo != -1)
        {
            //printf("%.*s", (int)(match.rm_so), string);
            printf("%s%.*s%s",  ANSI_COLOR_RED,
                                (int)(match.rm_eo - match.rm_so), 
                                string + offset + match.rm_so,
                                ANSI_COLOR_RESET);
            offset += match.rm_eo;
        }
        else
        {
            break;
        }
    }
    if(offset)
        printf("%s", string + offset);
    regfree(&regex);
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        fprintf(stderr, "mygrep: pattern not specified\n");
        return 1;
    }
    regex_t regex;
    if (regcomp(&regex, argv[1], REG_NEWLINE)) 
    {
        fprintf(stderr, "mygrep: error compiling regular expression\n");
        regfree(&regex);
        return 1;
    }
    regfree(&regex);

    if (argc == 2) 
    {
        // Если файлов не задано, читать из стандартного ввода
        char* buffer = NULL;
        size_t len = 0;
        while (getline(&buffer, &len, stdin) != -1) 
        {
            print_with_highlight(buffer, argv[1]);
        }
        if(buffer != NULL)
        {
            free(buffer);
        }
    } 
    else 
    {
        // Обработка переданных файлов
        FILE *fp = fopen(argv[2], "r");
        if (fp == NULL) 
        {
            fprintf(stderr, "mygrep: cannot open '%s': %s\n", argv[2], strerror(errno));
            return 1;
        }

        char* buffer = NULL;
        size_t len = 0;
        while (getline(&buffer, &len, fp) != -1) 
        { 
            print_with_highlight(buffer, argv[1]);
        }
        if(buffer != NULL)
        {
            free(buffer);
        }
        fclose(fp);
    }
    return 0;
}

