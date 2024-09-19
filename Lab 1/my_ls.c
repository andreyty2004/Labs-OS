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
#define ANSI_COLOR_RESET   "x1b[0m"
#define ANSI_COLOR_BLUE    "x1b[34m"
#define ANSI_COLOR_GREEN   "x1b[32m"
#define ANSI_COLOR_CYAN    "x1b[36m"

// Структура для хранения информации о файле
typedef struct {
  char *name;
  struct stat sb;
} file_info;

// Функция для сравнения имен файлов
int compare_names(const void *a, const void *b) {
  return strcmp(((file_info *)a)->name, ((file_info *)b)->name);
}

// Функция для вывода информации о файле
void print_file_info(file_info *file, int long_format) {
  if (long_format) {
    char permissions[11];
    permissions[0] = (file->sb.st_mode & S_IFDIR) ? 'd' : '-';
                    (file->sb.st_mode & S_IFLNK) ? 'l' : '-';
    permissions[1] = (file->sb.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (file->sb.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (file->sb.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (file->sb.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (file->sb.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (file->sb.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (file->sb.st_mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (file->sb.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (file->sb.st_mode & S_IXOTH) ? 'x' : '-';
    permissions[10] = '0';

    // Получаем имя пользователя и группы
    struct passwd *pw = getpwuid(file->sb.st_uid);
    struct group *gr = getgrgid(file->sb.st_gid);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file->sb.st_mtime));

    // printf("%s %4lu %s %s %8lu %s %s\n", 
    //        permissions,
    //        file->sb.st_nlink,
    //        pw ? pw->pw_name : "?",
    //        gr ? gr->gr_name : "?",
    //        file->sb.st_size,
    //        time_str,
    //        file->name);

    // Вывод с цветами
    printf("%s%-10s %s%-5lu %s%-8s %s%-8s %s%-8ld %s%-12s %s%-s%s\n",
           file->sb.st_mode & S_IFDIR ? ANSI_COLOR_BLUE :
           file->sb.st_mode & S_IFLNK ? ANSI_COLOR_CYAN :
           file->sb.st_mode & S_IXUSR ? ANSI_COLOR_GREEN : "",
           permissions, 
           ANSI_COLOR_RESET,
           file->sb.st_nlink, 
           ANSI_COLOR_RESET,
           pw ? pw->pw_name : "?", 
           ANSI_COLOR_RESET,
           gr ? gr->gr_name : "?",
           ANSI_COLOR_RESET,
           file->sb.st_size, 
           ANSI_COLOR_RESET,
           time_str,
           ANSI_COLOR_RESET,
           file->name,
           ANSI_COLOR_RESET);
  } else {
    // printf("%s  ", file->name);

    static int column = 0; // счетчик столбцов

    // Вывод с цветами
    printf("%s%-10s %s",
           file->sb.st_mode & S_IFDIR ? ANSI_COLOR_BLUE :
           file->sb.st_mode & S_IFLNK ? ANSI_COLOR_CYAN :
           file->sb.st_mode & S_IXUSR ? ANSI_COLOR_GREEN : "",
           file->name, ANSI_COLOR_RESET);

    column++;

    // если количество столбцов достигло 3, то переходим на новую строку
    if (column == 3) {
      printf("n");
      column = 0;
    }
  }
}

// Функция для получения информации о файлах в директории
file_info **get_file_info(char *path, int *count, int all) {
  DIR *dir;
  struct dirent *entry;
  file_info **files = NULL;
  *count = 0;

  dir = opendir(path);
  if (dir == NULL) {
    fprintf(stderr, "ls: cannot open directory '%s': %sn", path, strerror(errno));
    return NULL;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (!all && entry->d_name[0] == '.') {
      continue;
    }

    *count += 1;
    files = realloc(files, (*count) * sizeof(file_info *));
    if (files == NULL) {
      fprintf(stderr, "ls: out of memoryn");
      closedir(dir);
      return NULL;
    }

    files[*count - 1] = malloc(sizeof(file_info));
    if (files[*count - 1] == NULL) {
      fprintf(stderr, "ls: out of memoryn");
      closedir(dir);
      return NULL;
    }

    files[*count - 1]->name = strdup(entry->d_name);
    if (files[*count - 1]->name == NULL) {
      fprintf(stderr, "ls: out of memoryn");
      closedir(dir);
      return NULL;
    }

    char full_path[MAX_PATH_LEN];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
    if (lstat(full_path, &(files[*count - 1]->sb)) == -1) {
      fprintf(stderr, "ls: cannot stat '%s': %sn", full_path, strerror(errno));
      free(files[*count - 1]->name);
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

  while ((opt = getopt(argc, argv, "la")) != -1) {
    switch (opt) {
      case 'l':
        long_format = 1;
        break;
      case 'a':
        all_files = 1;
        break;
      default:
        fprintf(stderr, "Usage: ls [-l] [-a] [file ...]n");
        return 1;
    }
  }

  if (optind == argc) {
    // Если аргументов нет, вывести информацию о текущей директории
    file_info **files;
    int count;
    files = get_file_info(".", &count, all_files);
    if (files != NULL) {
      qsort(files, count, sizeof(file_info *), compare_names);
      for (int i = 0; i < count; i++) {
        print_file_info(files[i], long_format);
        free(files[i]->name);
        free(files[i]);
      }
      free(files);
    }
  } else {
    // Обработка переданных файлов и директорий
    for (; optind < argc; optind++) {
      file_info **files;
      int count;
      files = get_file_info(argv[optind], &count, all_files);
      if (files != NULL) {
        qsort(files, count, sizeof(file_info *), compare_names);
        for (int i = 0; i < count; i++) {
          print_file_info(files[i], long_format);
          free(files[i]->name);
          free(files[i]);
        }
        free(files);
      }
    }
  }

  return 0;
}