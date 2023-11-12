#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>

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
  if(argc < 2) {
    printf("Not enough arguments\n");
    exit(1);
  }
  char *fileName = argv[1];
  int inputFd = open(fileName, O_RDONLY);
  if (inputFd == -1) {
    perror("Failed to open input file");
    exit(1);
  }
  struct stat fileStats;
  if (fstat(inputFd, &fileStats) == -1) {
    perror("Failed to get file stats!\n");
    close(inputFd);
    exit(1);
  }
  const char *fileExtension = FileExtension(fileName);
  if(strcmp(fileExtension, ".bmp") != 0) {
      printf("File must have a BMP extension!\n");
      exit(1);
  }
  
  char outputFileName[] = "statistica.txt";
  int outputFd = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
  if (outputFd == -1) {
    perror("Failed to open output file");
    close(inputFd);
    exit(1);
  }

  WriteFormattedToFile(outputFd, "nume fisier: %s\n", fileName);

  char unusedBuffer[1024];
  int width, height;

  SafeRead(inputFd, unusedBuffer, 18);
  SafeRead(inputFd, &width, 4);
  SafeRead(inputFd, &height, 4);
  
  WriteFormattedToFile(outputFd, "inaltime: %d\n", height);
  WriteFormattedToFile(outputFd, "lungime: %d\n", width);
  WriteFormattedToFile(outputFd, "dimensiune: %ld\n", (long)fileStats.st_size);
  WriteFormattedToFile(outputFd, "identificatorul utilizatorului: %d\n", fileStats.st_uid);
  WriteFormattedToFile(outputFd, "timpul ultimei modificari: %s", ctime(&fileStats.st_mtime));
  WriteFormattedToFile(outputFd, "contorul de legaturi: %ld\n", (long)fileStats.st_nlink);
  
  WritePermissionToFile(outputFd, fileStats.st_mode, S_IRUSR, S_IWUSR, S_IXUSR, "drepturi de acces user: ");
  WritePermissionToFile(outputFd, fileStats.st_mode, S_IRGRP, S_IWGRP, S_IXGRP, "drepturi de acces grup: ");
  WritePermissionToFile(outputFd, fileStats.st_mode, S_IROTH, S_IWOTH, S_IXOTH, "drepturi de acces altii: ");

  if (close(inputFd) == -1) {
    perror("Error closing input file");
    exit(1);
  }
  
  if (close(outputFd) == -1) {
    perror("Error closing output file");
    exit(1);
  }
  
  return 0;
}
