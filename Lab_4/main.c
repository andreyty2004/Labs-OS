#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define USER 0
#define GROUP 1
#define OTHER 2

long int octal_form(char* msk)
{
  long int mask = 0;
  for(int i = 0; i < 3; i++)
  {
    mask = mask << 3;
    int num = msk[i] - '0';
    mask = mask | num;
  }
  return mask;
}

long parse_mask(const char* str, const char* path)
{
  int mode[3] = {0};

  struct stat st;
  int r = stat(path, &st);
  if(r == -1)
  {
    fprintf(stderr, "File error\n");
    return -1;
  }

  int initial_mode[3] = {0};
  int initial_bitmap[3][3] = {0};

  if(st.st_mode & S_IRUSR)
    {initial_mode[USER] += 4; initial_bitmap[USER][0] = 1;}
  if(st.st_mode & S_IWUSR)
    {initial_mode[USER] += 2; initial_bitmap[USER][1] = 1;}
  if(st.st_mode & S_IXUSR)
    {initial_mode[USER] += 1; initial_bitmap[USER][2] = 1;}

  if(st.st_mode & S_IRGRP)
    {initial_mode[GROUP] += 4; initial_bitmap[GROUP][0] = 1;}
  if(st.st_mode & S_IWGRP)
    {initial_mode[GROUP] += 2; initial_bitmap[GROUP][1] = 1;}
  if(st.st_mode & S_IXGRP)
    {initial_mode[GROUP] += 1; initial_bitmap[GROUP][2] = 1;}

  if(st.st_mode & S_IROTH)
    {initial_mode[OTHER] += 4; initial_bitmap[OTHER][0] = 1;}
  if(st.st_mode & S_IWOTH)
    {initial_mode[OTHER] += 2; initial_bitmap[OTHER][1] = 1;}
  if(st.st_mode & S_IXOTH)
    {initial_mode[OTHER] += 1; initial_bitmap[OTHER][2] = 1;}

  int permissions = 0;
  int ugoa = 4;
  int read = 0, write = 0, execute = 0;
  char operation;

  for(int i = 0; i < strlen(str); i++)
  {
    if(permissions == 0)
    {
      switch(str[i])
      {
        case 'u':
        {
          ugoa = 0;
          break;
        }
        case 'g':
        {
          ugoa = 1;
          break;
        }
        case 'o':
        {
          ugoa = 2;
          break;
        }
        case 'a':
        {
          ugoa = 3;
          break;
        }
        default:
        {
          if(ugoa == 4)
            ugoa = 3;
          permissions = 1;
          break;
        }
      }
    }
    if(permissions == 1)
    {
      switch(str[i])
      {
        case '+':
        {
          operation = '+';
          break;
        }
        case '-':
        {
          operation = '-';
          break;
        }
        case '=':
        {
          operation = '=';
          break;
        }
        default:
        {
          permissions = 2;
          break;
        }
      }
    }
    if(permissions == 2)
    {
      switch(str[i])
      {
        case 'r':
        {
          read = 1;
          break;
        }
        case 'w':
        {
          write = 1;
          break;
        }
        case 'x':
        {
          execute = 1;
          break;
        }
        default:
        {
          fprintf(stderr, "Incorrect mode (1)\n");
          return -1;
          break;
        }
      }
    }
  }

  if(!operation)
  {
    fprintf(stderr, "Incorrect mode (2)\n");
    return -1;
  }

  int temp = 0;
  if(read)
    temp += 4;
  if(write)
    temp += 2;
  if(execute)
    temp += 1;

  for(int i = 0; i < 3; i++)
    mode[i] = initial_mode[i];

  if(operation == '=')
  {
    for(int i = 0; i < 3; i++)
    {
      if(i == ugoa || ugoa == 3)
      {
        mode[i] = temp;
      }
    }
  }
  else if(operation == '-')
  {
    for(int i = 0; i < 3; i++)
    {
      if(i == ugoa || ugoa == 3)
      {
        if(read && initial_bitmap[i][0])
          mode[i] -= 4;
        if(write && initial_bitmap[i][1])
          mode[i] -= 2;
        if(execute && initial_bitmap[i][2])
          mode[i] -= 1;
      }
    }
  }
  else if(operation == '+')
  {
    for(int i = 0; i < 3; i++)
    {
      if(i == ugoa || ugoa == 3)
      {
        if(read && !initial_bitmap[i][0])
          mode[i] += 4;
        if(write && !initial_bitmap[i][1])
          mode[i] += 2;
        if(execute && !initial_bitmap[i][2])
          mode[i] += 1;
      }
    }
  }

  char str_mode[3];

  for(int i = 0; i < 3; i++)
    str_mode[i] = mode[i] + '0'; 

  return octal_form(str_mode); 
}

int main(int argc, char* argv[])
{
  if(argc < 3)
  {
    fprintf(stderr, "Not enough arguments\n");
    return -1;
  }
  
  long int mode;
  int res;
  char* value = argv[1];
  
  if(value[0] >= '0' && value[0] <= '7')
  {
    if(strlen(value) != 3)
    {
      fprintf(stderr, "incorrect mode (3)\n"); 
    }
    mode = octal_form(argv[1]);
  }
  else
  {
    mode = parse_mask(argv[1], argv[2]);
  }
  
  if(mode != -1)
  {
    res = chmod(argv[2], mode);
  }

  if(res == -1)
  {
    fprintf(stderr, "chmod error\n");
  }
  return 0;
}

// enhance - and +
// add leading zero