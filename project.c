#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>
#include <limits.h>

#define MAX_CHILDREN 128

// [Funcții deja existente: FileExtension, WriteFormattedToFile, WritePermissionToFile, SafeRead, countLines]

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

int countLines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return -1;
    }

    int count = 0;
    char buffer[1024];
    
    while (fgets(buffer, sizeof(buffer), file)) {
        count++;
    }

    fclose(file);
    return count;
}

void ConvertBMPToGreyscale(const char *inputPath, int bmpFd, int width, int height, int padding) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char pixel[3];
            ssize_t result = read(bmpFd, pixel, 3);
			if (result != 3) {
				if (result == -1) {
					perror("Failed to read pixel data");
				} else if (result == 0) {
					fprintf(stderr, "End of file reached unexpectedly when reading pixel data!\n");					
					} else {
					fprintf(stderr, "Partial pixel data read. Expected 3 bytes, got %zd\n", result);
				}
				if (close(bmpFd) == -1) {
					perror("Error closing the bmp file");
				}
				exit(EXIT_FAILURE);
			}

            unsigned char grey = (unsigned char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
            memset(pixel, grey, sizeof(pixel));

            if (lseek(bmpFd, -3, SEEK_CUR) == (off_t)-1) {
                perror("Failed to seek to pixel position");
                if  (close(bmpFd) == -1) {
					perror("Error closing the bmp file!");
					exit(EXIT_FAILURE);
				}
                exit(EXIT_FAILURE);
            }

            if (write(bmpFd, pixel, 3) != 3) {
                perror("Failed to write grey pixel data");
                if  (close(bmpFd) == -1) {
					perror("Error closing the bmp file!");
					exit(EXIT_FAILURE);
				}
                exit(EXIT_FAILURE);
            }
        }

        if (lseek(bmpFd, padding, SEEK_CUR) == (off_t)-1) {
            perror("Failed to seek to pixel position");
            if  (close(bmpFd) == -1) {
				perror("Error closing the bmp file!");
				exit(EXIT_FAILURE);
			}
            exit(EXIT_FAILURE);
        }
    }
}

void ProcessRegularFile(const char *inputPath, int outputFd, const struct stat *entry_stat) {
    const char *fileExt = FileExtension(inputPath);
    if (fileExt != NULL && strcmp(fileExt, ".bmp") == 0) {
        // Tratarea specială a fișierelor BMP
        int bmpFd = open(inputPath, O_RDWR);
        if (bmpFd == -1) {
            perror("Failed to open BMP file for greyscale conversion");
            exit(EXIT_FAILURE);
        }

        unsigned char bmpHeader[54];
        if (read(bmpFd, bmpHeader, sizeof(bmpHeader)) != sizeof(bmpHeader)) {
            perror("Failed to read BMP header");
            if (close(bmpFd) == -1) {
                perror("Error closing the bmp file!");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        }

        int width = *(int *)&bmpHeader[18];
        int height = abs(*(int *)&bmpHeader[22]);
        int padding = (4 - (width * 3) % 4) % 4;

        WriteFormattedToFile(outputFd, "File name: %s\n", inputPath);
        WriteFormattedToFile(outputFd, "File height: %d\n", height);
		WriteFormattedToFile(outputFd, "File width: %d\n", width);
        WriteFormattedToFile(outputFd, "File size: %ld bytes\n", (long)entry_stat->st_size);
        WriteFormattedToFile(outputFd, "User ID: %d\n", entry_stat->st_uid);
        WriteFormattedToFile(outputFd, "Last modified: %s", ctime(&entry_stat->st_mtime));
        WriteFormattedToFile(outputFd, "Link count: %ld\n", (long)entry_stat->st_nlink);

        // Scrierea permisiunilor
        WritePermissionToFile(outputFd, entry_stat->st_mode, S_IRUSR, S_IWUSR, S_IXUSR, "User permissions: ");
        WritePermissionToFile(outputFd, entry_stat->st_mode, S_IRGRP, S_IWGRP, S_IXGRP, "Group permissions: ");
        WritePermissionToFile(outputFd, entry_stat->st_mode, S_IROTH, S_IWOTH, S_IXOTH, "Other permissions: ");

        ConvertBMPToGreyscale(inputPath, bmpFd, width, height, padding);

        if (close(bmpFd) == -1) {
		  perror("Error closing the bmp file!");
		  exit(EXIT_FAILURE);
		}
    } else {
        // Tratarea fișierelor obișnuite
        WriteFormattedToFile(outputFd, "File name: %s\n", inputPath);
        WriteFormattedToFile(outputFd, "File size: %ld bytes\n", (long)entry_stat->st_size);
        WriteFormattedToFile(outputFd, "User ID: %d\n", entry_stat->st_uid);
        WriteFormattedToFile(outputFd, "Last modified: %s", ctime(&entry_stat->st_mtime));
        WriteFormattedToFile(outputFd, "Link count: %ld\n", (long)entry_stat->st_nlink);

        // Scrierea permisiunilor
        WritePermissionToFile(outputFd, entry_stat->st_mode, S_IRUSR, S_IWUSR, S_IXUSR, "User permissions: ");
        WritePermissionToFile(outputFd, entry_stat->st_mode, S_IRGRP, S_IWGRP, S_IXGRP, "Group permissions: ");
        WritePermissionToFile(outputFd, entry_stat->st_mode, S_IROTH, S_IWOTH, S_IXOTH, "Other permissions: ");
    }
}

void ProcessSymbolicLink(const char *inputPath, int outputFd, const struct stat *entry_stat) {
    char targetPath[PATH_MAX];
    ssize_t len = readlink(inputPath, targetPath, sizeof(targetPath) - 1);
    if (len == -1) {
        perror("Failed to read symbolic link target");
        exit(EXIT_FAILURE);
    }
    targetPath[len] = '\0'; 

    // Scrierea informațiilor despre link-ul simbolic
    WriteFormattedToFile(outputFd, "Symbolic link name: %s\n", inputPath);
    WriteFormattedToFile(outputFd, "Symbolic link size: %ld bytes\n", (long)entry_stat->st_size);
    WriteFormattedToFile(outputFd, "Symbolic link target: %s\n", targetPath);

    struct stat targetStat;
    if (lstat(targetPath, &targetStat) == -1) {
        perror("Failed to get statistics of the target file");
        exit(EXIT_FAILURE);
    }

    // Scrierea informațiilor despre fișierul țintă al link-ului simbolic
    WriteFormattedToFile(outputFd, "Target file size: %ld bytes\n", (long)targetStat.st_size);
    WritePermissionToFile(outputFd, targetStat.st_mode, S_IRUSR, S_IWUSR, S_IXUSR, "User permissions for target: ");
    WritePermissionToFile(outputFd, targetStat.st_mode, S_IRGRP, S_IWGRP, S_IXGRP, "Group permissions for target: ");
    WritePermissionToFile(outputFd, targetStat.st_mode, S_IROTH, S_IWOTH, S_IXOTH, "Other permissions for target: ");
}

void ProcessDirectory(const char *inputPath, int outputFd, const struct stat *entry_stat) {
    // Scrierea informațiilor de bază despre director
    WriteFormattedToFile(outputFd, "Directory name: %s\n", inputPath);
    WriteFormattedToFile(outputFd, "User ID: %d\n", entry_stat->st_uid);
    WriteFormattedToFile(outputFd, "Link count: %ld\n", (long)entry_stat->st_nlink);

    // Scrierea permisiunilor pentru director
    WritePermissionToFile(outputFd, entry_stat->st_mode, S_IRUSR, S_IWUSR, S_IXUSR, "User permissions: ");
    WritePermissionToFile(outputFd, entry_stat->st_mode, S_IRGRP, S_IWGRP, S_IXGRP, "Group permissions: ");
    WritePermissionToFile(outputFd, entry_stat->st_mode, S_IROTH, S_IWOTH, S_IXOTH, "Other permissions: ");
}

void ProcessFile(const char *inputPath, const char *outputDir, const char *fileName, const struct stat *entry_stat) {
    // Crearea și deschiderea fișierului de ieșire
    char outputFilePath[PATH_MAX];
    snprintf(outputFilePath, PATH_MAX, "%s/%s_statistica.txt", outputDir, fileName);
    int outputFd = open(outputFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outputFd == -1) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    // Procesarea fișierului în funcție de tip
    if (S_ISREG(entry_stat->st_mode)) {
        ProcessRegularFile(inputPath, outputFd, entry_stat);
    } else if (S_ISLNK(entry_stat->st_mode)) {
        ProcessSymbolicLink(inputPath, outputFd, entry_stat);
    } else if (S_ISDIR(entry_stat->st_mode)) {
        ProcessDirectory(inputPath, outputFd, entry_stat);
    }

    // Închiderea fișierului de ieșire
    if (close(outputFd) == -1) {
        perror("Error closing output file");
        exit(EXIT_FAILURE);
    }
}

void CreateChildProcess(const char *inputPath, const char *outputDir, const char *fileName, const struct stat *entry_stat, int pipes[][2], int childIndex) {
    // Crearea pipe-ului pentru procesul copil
    if (pipe(pipes[childIndex]) == -1) {
        perror("Pipe failure");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // În procesul copil
        if(close(pipes[childIndex][0]) == -1) { // Închiderea capătului de citire
            perror("Error closing the reading end of pipe!");
            exit(EXIT_FAILURE);
        }
        ProcessFile(inputPath, outputDir, fileName, entry_stat);
        // Trimiterea numărului de linii către procesul părinte
        char outputFilePath[PATH_MAX];
        snprintf(outputFilePath, PATH_MAX, "%s/%s_statistica.txt", outputDir, fileName);

        int numLines = countLines(outputFilePath);
        if (numLines == -1) {
            perror("Error at counting lines from files!");
            exit(EXIT_FAILURE);
        }

        if (write(pipes[childIndex][1], &numLines, sizeof(numLines)) != sizeof(numLines)) {
            perror("Error writing to pipe!");
            close(pipes[childIndex][1]);
            exit(EXIT_FAILURE);
        }

        if(close(pipes[childIndex][1]) == -1) {
            perror("Error closing the writing end of pipe!");
            exit(EXIT_FAILURE);
        } // Închiderea capătului de scriere

        exit(0);

    } else {
        // În procesul părinte
        if(close(pipes[childIndex][1]) == -1) {
            perror("Error closing the writing end of pipe!");
            exit(EXIT_FAILURE);
        }
        childIndex++; // Închiderea capătului de scriere
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_directory> <output_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("Failed to open input directory");
        exit(EXIT_FAILURE);
    }

    int pipes[MAX_CHILDREN][2];
    struct dirent *entry;
    struct stat entry_stat;
    int childIndex = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char inputPath[PATH_MAX], outputPath[PATH_MAX];
        snprintf(inputPath, PATH_MAX, "%s/%s", argv[1], entry->d_name);
        snprintf(outputPath, PATH_MAX, "%s/%s", argv[2], entry->d_name);

        if (lstat(inputPath, &entry_stat) == -1) {
            perror("Failed to get entry stats");
            continue;
        }

        CreateChildProcess(inputPath, argv[2], entry->d_name, &entry_stat, pipes, childIndex);
        childIndex++;
    }

    for (int i = 0; i < childIndex; i++) {
        int status;
        pid_t childPid = wait(&status);
        int numLines;

        if (childPid == -1) {
            perror("Wait error");
            continue;
        }

        ssize_t bytesRead = read(pipes[i][0], &numLines, sizeof(numLines));
      	if (bytesRead == sizeof(numLines)) {
        	if (WIFEXITED(status)) {
	  			int exitStatus = WEXITSTATUS(status);
	  			printf("Child process with PID %d exited with status %d. It wrote %d lines.\n", childPid, exitStatus, numLines);
        	} else if (WIFSIGNALED(status)) {
	  			int termSig = WTERMSIG(status);
	  			printf("Child process with PID %d was terminated by signal %d. It wrote %d lines.\n", childPid, termSig, numLines);
        	}
      	} else {
        	if (bytesRead == -1) {
	  			perror("Read error");
        	} else {
	  			fprintf(stderr, "Child process with PID %d did not send the expected number of bytes.\n", childPid);
        	}
      	}
		
        if(close(pipes[i][0]) == -1) {
            perror("Error closing the reading end of pipe!");
            exit(EXIT_FAILURE);
      }
    }

    if (closedir(dir) == -1) {
        perror("Error closing directory");
        exit(EXIT_FAILURE);
    }

    return 0;
}
