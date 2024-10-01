#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

int main(int argc, char* argv[])
{
    int n = 0;
    int b = 0;
    int E = 0;
    int opt;

    while ((opt = getopt(argc, argv, "nbE")) != -1)
    {
        switch(opt)
        {
            case 'n':
                n = 1;
                break;
            case 'b':
                b = 1;
                break;  
            case 'E':
                E = 1;
                break;
            default:
                fprintf(stderr, "Usage: mycat [-n] [-b] [-E] [file ...]\n");
                return 1;
        }
    }

    if (optind == argc) 
    {
        // Print usage information if no filenames are provided
        printf("Usage: %s filename1 [filename2 ...]\n",
               argv[0]);
        return 1;
    }

    for (; optind < argc; optind++) 
    {
        FILE* file = fopen(argv[optind], "r");
        if (file == NULL) 
        {
            fprintf(stderr, "mycat: cannot open '%s': %s\n", argv[optind], strerror(errno));
            continue;
        }

        char* buffer = NULL;
        size_t len = 0;
        int line_count = 1;
        int nonempty_line_count = 1;

        while(getline(&buffer, &len, file) != -1)
        {
            if(n && !b)
            {
                printf("%6d ", line_count++);
            }
            else if(b  && strlen(buffer) > 1)
            {
                printf("%6d ", nonempty_line_count++);
            }

            if(E)
            {
                for(int i = 0; i <= strlen(buffer); i++)
                {
                    if(buffer[i] == '\n')
                    {
                        buffer[i] = '$';
                        buffer[i + 1] = '\n';
                        buffer[i + 2] = '\0';
                        break;
                    }

                }
            }
            printf("%s", buffer);
        }
        free(buffer);
        fclose(file);
    }
    return 0;
}