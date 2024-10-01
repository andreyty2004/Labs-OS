#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define MAX_PATH_LEN 1024
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

// Структура для хранения информации о файле
typedef struct 
{
  char* name;
  struct stat sb;
} file_info;

// Функция для сравнения имен файлов
void sort(file_info** files, int count)
{
  file_info temp;

  for(int i = 0; i < count - 1; i++)
  {
    for(int j = 0; j < count - i - 1; j++)
    {
      if(strcmp(files[j]->name, files[j + 1]->name) > 0)
      {
        memcpy(&temp, files[j], sizeof(file_info));
        memcpy(files[j], files[j + 1], sizeof(file_info));
        memcpy(files[j + 1], &temp, sizeof(file_info));
      }
    }
  }
}

void count_total(file_info** files, int count)
{
  int total = 0;
  for(int i = 0; i < count; i++)
  {
    if(!S_ISLNK(files[i] -> sb.st_mode))
    {
      if(S_ISDIR(files[i] -> sb.st_mode) && files[i] -> sb.st_size < 4096)
        continue;
      if((files[i] -> sb.st_size % 4096) == 0)
      {
        total = total + (files[i] -> sb.st_size / 4096) * 4;
      }
      else
      {
        int j = files[i] -> sb.st_size / 4096 + 1;
        total = total + j * 4;
      }
    }
  }
  printf("total %i\n", total);
}

// Функция для вывода информации о файле
void print_file_info(char *path, file_info *file, int long_format) 
{
  if (long_format) 
  {
    char permissions[11];
    permissions[0] = (S_ISDIR(file->sb.st_mode)) ? 'd' :
                    (S_ISLNK(file->sb.st_mode)) ? 'l' : '-';
    permissions[1] = (file->sb.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (file->sb.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (file->sb.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (file->sb.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (file->sb.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (file->sb.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (file->sb.st_mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (file->sb.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (file->sb.st_mode & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0';

    // Получаем имя пользователя и группы
    struct passwd *pw = getpwuid(file->sb.st_uid);
    struct group *gr = getgrgid(file->sb.st_gid);

    // Время создания документа
    char time_str[14];
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file->sb.st_mtime));

    if(pw && gr)
    {

    	printf("%-10s %-5lu %-8s %-8s %-8ld %-12s %s%-s%s",
           permissions, 
           file -> sb.st_nlink, 
           pw -> pw_name, 
           gr -> gr_name,
           file -> sb.st_size, //!
           time_str,
           S_ISDIR(file->sb.st_mode) ? ANSI_COLOR_BLUE :
           S_ISLNK(file->sb.st_mode)  ? ANSI_COLOR_CYAN :
           file -> sb.st_mode & (S_IXUSR || S_IXGRP || S_IROTH) ? ANSI_COLOR_GREEN : "",
           file -> name,
           ANSI_COLOR_RESET);

    }
    else
    {
   	 printf("%-10s %-5lu %-8li %-8li %-8ld %-12s %s%-s%s",
           permissions, 
           file -> sb.st_nlink, 
           (long)(file -> sb.st_uid), 
           (long)(file -> sb.st_gid),
           file -> sb.st_size, //!
           time_str,
           S_ISDIR(file->sb.st_mode) ? ANSI_COLOR_BLUE :
           S_ISLNK(file->sb.st_mode)  ? ANSI_COLOR_CYAN :
           file -> sb.st_mode & (S_IXUSR || S_IXGRP || S_IROTH) ? ANSI_COLOR_GREEN : "",
           file -> name,
           ANSI_COLOR_RESET);

    }
    // Вывод с цветами
    // Куда указывает ссылка
    if(S_ISLNK(file->sb.st_mode))
    {
      printf(" -> ");
      char link_path[1024];
      char target[1024];
      snprintf(link_path, sizeof(link_path), "%s/%s", path, file -> name);
      int len = readlink(link_path, target, sizeof(target));
      if(len == -1)
      {
        perror("readlink");
        printf("%s", file -> name);
        return;
      }
      target[len] = '\0';
      printf("%s", target);
    }
    printf("\n");
  } 
  else 
  {
    static int column = 0; // счетчик столбцов

    // Вывод с цветами
    printf("%s%-8s %s",
            S_ISDIR(file->sb.st_mode) ? ANSI_COLOR_BLUE :
            S_ISLNK(file->sb.st_mode)  ? ANSI_COLOR_CYAN :
            file -> sb.st_mode & (S_IXUSR || S_IXGRP || S_IROTH) ? ANSI_COLOR_GREEN : "",
            file -> name, 
            ANSI_COLOR_RESET);

    column++;

    if (column == 8) 
    {
      printf("\n");
      column = 0;
    }
  }
}

// Функция для получения информации о файлах в директории
file_info **get_file_info(char *path, int *count, int all_files) 
{
    DIR *dir;
    struct dirent *entry;
    file_info **files = NULL;
    *count = 0;

    dir = opendir(path);
    if (dir == NULL) 
    {
      fprintf(stderr, "ls: cannot open directory '%s': %s\n", path, strerror(errno));
      return NULL;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
      if (!all_files && entry -> d_name[0] == '.') 
      {
        continue;
      }

      *count += 1;
      files = realloc(files, (*count) * sizeof(file_info *));
      if (files == NULL) 
      {
        fprintf(stderr, "ls: out of memory\n");
        closedir(dir);
        return NULL;
      }

      files[*count - 1] = malloc(sizeof(file_info));
      if (files[*count - 1] == NULL) 
      {
        fprintf(stderr, "ls: out of memory\n");
        closedir(dir);
        return NULL;
      }

      files[*count - 1] -> name = strdup(entry -> d_name);
      if (files[*count - 1] -> name == NULL) 
      {
        fprintf(stderr, "ls: out of memory\n");
        closedir(dir);
        return NULL;
      }

      char full_path[MAX_PATH_LEN];
      snprintf(full_path, sizeof(full_path), "%s/%s", path, entry -> d_name);
      if (lstat(full_path, &(files[*count - 1] -> sb)) == -1) {
        fprintf(stderr, "ls: cannot stat '%s': %s\n", full_path, strerror(errno));
        free(files[*count - 1] -> name);
        free(files[*count - 1]);
        files[*count - 1] = NULL;
        *count -= 1;
      }
  }

  closedir(dir);
  return files;
}

int main(int argc, char **argv) {
  int long_format = 0;
  int all_files = 0;
  int opt;

  while ((opt = getopt(argc, argv, "la")) != -1) 
  {
      switch (opt) 
      {
        case 'l':
          long_format = 1;
          break;
        case 'a':
          all_files = 1;
          break;
        default:
          fprintf(stderr, "Usage: ls [-l] [-a] [file ...]\n");
          return 1;
      }
  }

  if (optind == argc) 
  {
    // Если аргументов нет, вывести информацию о текущей директории
    file_info **files;
    int count;
    files = get_file_info(".", &count, all_files);
    if (files != NULL) 
    {
      sort(files, count);
      if(long_format)
        count_total(files, count);
      for (int i = 0; i < count; i++) 
      {
        print_file_info(".", files[i], long_format);
        free(files[i] -> name);
        free(files[i]);
      }
      if(!long_format)
        printf("\n");
      free(files);
    }
  } 
  else 
  {
    // Обработка переданных файлов и директорий
    for (; optind < argc; optind++) 
    {
      file_info** files;
      int count;
      files = get_file_info(argv[optind], &count, all_files);
      if (files != NULL) 
      {
        sort(files, count);
        if(long_format)
          count_total(files, count);
        for (int i = 0; i < count; i++) 
        {
          print_file_info(argv[optind], files[i], long_format);
          free(files[i] -> name);
          free(files[i]);
        }
        free(files);
      }
    }
  }
  return 0;
}
