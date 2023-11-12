#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>
#include <limits.h>

const char *FileExtension(const char path[]) {
    const char *result;
    int i, n;

    n = strlen(path);
    i = n - 1;
    while ((i > 0) && (path[i] != '.') && (path[i] != '/') && (path[i] != '\\')) {
        i--;
    }
    if ((i > 0) && (i < n - 1) && (path[i] == '.') && (path[i - 1] != '/') && (path[i - 1] != '\\')) {
        result = path + i;
    } else {
      printf("The argument given must be a file!\n");
      exit(1);
    }
    return result;
}

void WriteFormattedToFile(int fileDescriptor, const char *format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (write(fileDescriptor, buffer, strlen(buffer)) == -1) {
      perror("Error writing to file");
      close(fileDescriptor);
      exit(1);
    }
}

void WritePermissionToFile(int fileDescriptor, mode_t mode, mode_t permission1, mode_t permission2, mode_t permission3, char *permType) {
    char permString[4] = "---\n";
    
    permString[0] = (mode & permission1) ? 'R' : '-';
    permString[1] = (mode & permission2) ? 'W' : '-';
    permString[2] = (mode & permission3) ? 'X' : '-';
    
    if (write(fileDescriptor, permType, strlen(permType)) == -1 ||
        write(fileDescriptor, permString, strlen(permString)) == -1) {
      perror("Error writing permissions to file");
      close(fileDescriptor);
      exit(1);
    }
}

ssize_t SafeRead(int fileDescriptor, void *buffer, size_t count) {
    ssize_t bytesRead = read(fileDescriptor, buffer, count);
    if (bytesRead == -1) {
        perror("Error reading from file");
        close(fileDescriptor);
        exit(1);
    }
    return bytesRead;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_directory>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  DIR *dir;
  struct dirent *entry;
  struct stat entry_stat;
  int outputFd;
  char path[PATH_MAX];
  char outputFileName[] = "statistica.txt";
  char unusedBuffer[1024];
  
  dir = opendir(argv[1]);
  if (dir == NULL) {
    perror("Failed to open directory");
    exit(EXIT_FAILURE);
  }
  
  outputFd = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (outputFd == -1) {
    perror("Failed to open output file");
    exit(EXIT_FAILURE);
  }

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    
    snprintf(path, PATH_MAX, "%s/%s", argv[1], entry->d_name);
    if (lstat(path, &entry_stat) == -1) {
      perror("Failed to get entry stats");
      continue;
    }
    
    if (S_ISREG(entry_stat.st_mode)) {
      const char *fileExt = FileExtension(entry->d_name);
      WriteFormattedToFile(outputFd, "nume fisier: %s\n", entry->d_name);
      WriteFormattedToFile(outputFd, "dimensiune: %ld\n", (long)entry_stat.st_size);
      WriteFormattedToFile(outputFd, "identificatorul utilizatorului: %d\n", entry_stat.st_uid);
      WriteFormattedToFile(outputFd, "timpul ultimei modificari: %s", ctime(&entry_stat.st_mtime));
      WriteFormattedToFile(outputFd, "contorul de legaturi: %ld\n", (long)entry_stat.st_nlink);
      WritePermissionToFile(outputFd, entry_stat.st_mode, S_IRUSR, S_IWUSR, S_IXUSR, "drepturi de acces user: ");
      WritePermissionToFile(outputFd, entry_stat.st_mode, S_IRGRP, S_IWGRP, S_IXGRP, "drepturi de acces grup: ");
      WritePermissionToFile(outputFd, entry_stat.st_mode, S_IROTH, S_IWOTH, S_IXOTH, "drepturi de acces altii: ");
      
      if (fileExt != NULL && strcmp(fileExt, ".bmp") == 0) {
	int width, height;
	int bmpFileFd = open(path, O_RDONLY);
	if (bmpFileFd == -1) {
	  perror("Failed to open BMP file");
	  continue;
	}
	SafeRead(bmpFileFd, unusedBuffer, 18);
	SafeRead(bmpFileFd, &width, 4);
	SafeRead(bmpFileFd, &height, 4);
	WriteFormattedToFile(outputFd, "inaltime: %d\n", height);
	WriteFormattedToFile(outputFd, "lungime: %d\n", width);
	close(bmpFileFd);
      }
    }
    else if (S_ISLNK(entry_stat.st_mode)) {
      char targetPath[PATH_MAX];
      ssize_t len = readlink(path, targetPath, sizeof(targetPath) - 1);
      if (len == -1) {
        perror("Failed to read symbolic link target");
        continue;
      }
      targetPath[len] = '\0';
      
      struct stat targetStat;
      if (stat(targetPath, &targetStat) == -1) {
        perror("Failed to stat symbolic link target");
        continue;
      }
      
      WriteFormattedToFile(outputFd, "nume legatura: %s\n", entry->d_name);
      WriteFormattedToFile(outputFd, "cale legatura: %s\n", targetPath);
      WriteFormattedToFile(outputFd, "dimensiune legatura: %ld\n", (long)entry_stat.st_size);
      WriteFormattedToFile(outputFd, "dimensiune fisier: %ld\n", (long)targetStat.st_size);
      WritePermissionToFile(outputFd, targetStat.st_mode, S_IRUSR, S_IWUSR, S_IXUSR, "drepturi de acces user legatura: ");
      WritePermissionToFile(outputFd, targetStat.st_mode, S_IRGRP, S_IWGRP, S_IXGRP, "drepturi de acces grup legatura: ");
      WritePermissionToFile(outputFd, targetStat.st_mode, S_IROTH, S_IWOTH, S_IXOTH, "drepturi de acces altii legatura: ");
    }
    else if (S_ISDIR(entry_stat.st_mode)) {
      WriteFormattedToFile(outputFd, "nume director: %s\n", entry->d_name);
      WriteFormattedToFile(outputFd, "identificatorul utilizatorului: %d\n", entry_stat.st_uid);
      WritePermissionToFile(outputFd, entry_stat.st_mode, S_IRUSR, S_IWUSR, S_IXUSR, "drepturi de acces user: ");
      WritePermissionToFile(outputFd, entry_stat.st_mode, S_IRGRP, S_IWGRP, S_IXGRP, "drepturi de acces grup: ");
      WritePermissionToFile(outputFd, entry_stat.st_mode, S_IROTH, S_IWOTH, S_IXOTH, "drepturi de acces altii: ");
    }
  }

  if (closedir(dir) == -1) {
    perror("Error closing directory");
  }
  
  if (close(outputFd) == -1) {
    perror("Error closing output file");
    exit(EXIT_FAILURE);
  }
  
  return 0;
}
