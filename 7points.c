#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

const int buffer_size = 5000;


int main(int argc, char *argv[]) {
    int pipefd[2], pipefd2[2];
    pid_t pid1, pid2;

    char buffer[buffer_size], res[buffer_size];

    // Проверяем, есть ли файлы для чтения и записи
    if (argc != 3) {
        printf("Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }

    // Открываем файлы для чтения и записи
    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        perror("Error while opening input file");
        return 1;
    }

    FILE *output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
        perror("Error while opening output file");
        return 1;
    }

    // Создаем неименованные каналы
    if (pipe(pipefd) == -1 || pipe(pipefd2) == -1) {
        perror("Error while creating pipes");
        return 1;
    }

    // Создаем первый процесс
    pid1 = fork();

    if (pid1 == -1) {
        perror("Error while forking first process");
        return 1;
    }

    if (pid1 == 0) {
        // Дочерний процесс - первый процесс

        // Закрываем неиспользуемые концы каналов
        close(pipefd[0]);
        close(pipefd2[1]);

        // Read the input file into a buffer
        char input_buffer[buffer_size];
        size_t input_size = fread(input_buffer, 1, buffer_size, input_file);
        fclose(input_file);

        // Write the input buffer to the first pipe
        write(pipefd[1], input_buffer, input_size);

        // Read the res from the second pipe
        read(pipefd2[0], res, buffer_size);

        // Write the res to the output file
        fwrite(res, sizeof(char), strlen(res), output_file);

        // Завершаем работу процесса
        exit(0);
    }

    // Создаем второй процесс
    pid2 = fork();

    if (pid2 == -1) {
        perror("Error while forking second process");
        return 1;
    }

    if (pid2 == 0) {
        // Дочерний процесс - второй процесс

        // Закрываем неиспользуемые концы каналов
        close(pipefd[1]);
        close(pipefd2[0]);

        // Read the input buffer from the first pipe
        read(pipefd[0], buffer, buffer_size);

        char str1[buffer_size], str2[buffer_size];
        int ascii1[128] = {0};
        int ascii2[128] = {0};
        int i, count = 0;
        // Запускаем обработку данных
        for (i = 0; i < strlen(buffer); ++i) {
            if (buffer[i] == '\n'){
                break;
            }
            if (buffer[i] != '\n' && ascii1[(int) buffer[i]] == 0) {
                ascii1[(int) buffer[i]] = 1;
            }
        }
        // Считываем символы второй строки и заполняем массив ascii2
        while (i < strlen(buffer)) {
            if (buffer[i] != '\n' && ascii2[(int) buffer[i]] == 0) {
                ascii2[(int) buffer[i]] = 1;
            }
            i++;
        }
        // Записываем символы, которые содержатся в первой строке и не содержатся во второй, в результат
        for (i = 0; i < 128; ++i) {
            if (ascii1[i] == 1 && ascii2[i] == 0 && i != '\n') {
                res[count++] = i;
            }
        }
        // Разделяем строки
        res[count++] = '\n';
        // Записываем символы, которые содержатся вo второй строке и не содержатся в первой, в результат
        for (i = 0; i < 128; ++i) {
            if (ascii1[i] == 0 && ascii2[i] == 1 && i != '\n') {
                res[count++] = i;
            }
        }
        res[count++] = '\0';

        // Write the res to the second pipe
        write(pipefd2[1], res, strlen(res) + 1);

        // Завершаем работу процесса
        exit(0);
    }

    // Закрываем неиспользуемые концы каналов
    close(pipefd[0]);
    close(pipefd[1]);
    close(pipefd2[0]);
    close(pipefd2[1]);

    // Ожидаем завершения всех дочерних процессов
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}