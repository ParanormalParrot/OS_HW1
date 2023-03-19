#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

const int buffer_size = 5000;

int main(int argc, char *argv[]) {
    // Дескрипторы для pipe.
    int fd1[2], fd2[2], fd3[2];
    // Для хранения результатов fork().
    pid_t pid1, pid2, pid3;

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

    // Создаем каналы
    if (pipe(fd1) == -1 || pipe(fd2) == -1 || pipe(fd3) == -1) {
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
        // Первый дочерний процесс

        // Закрываем неиспользуемые концы каналов
        close(fd1[0]);
        close(fd2[0]);
        close(fd2[1]);
        close(fd3[0]);
        close(fd3[1]);

        // Записываем содержимое входного файла в буфер
        char input_buffer[buffer_size];
        size_t input_size = fread(input_buffer, 1, buffer_size, input_file);
        fclose(input_file);

        // Записываем содержимое буфера в канал.
        write(fd1[1], input_buffer, input_size);

        // Закрываем используемый конец канала
        close(fd1[1]);

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
        // Второй дочерний процесс

        // Закрываем неиспользуемые концы каналов
        close(fd1[1]);
        close(fd2[0]);
        close(fd3[0]);
        close(fd3[1]);

        // Читаем данные из канала
        read(fd1[0], &buffer, buffer_size);

        // Закрываем неиспользуемый конец канала
        close(fd1[0]);
        // Создаём массивы для обоих строк, в которых будем отмечать вхождение того или иного символа ASCII в строку.
        int ascii1[128] = {0};
        int ascii2[128] = {0};
        int i, count = -1;
        // Считываем символы первой строки (до символа '\n') и заполняем массив ascii1
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
        // Записываем символы, которые содержатся в первой строке и не содержатся во второй, в результат.
        for (i = 0; i < 128; ++i) {
            if (ascii1[i] == 1 && ascii2[i] == 0) {
                res[count++] = i;
            }
        }
        // Разделяем строки.
        res[count++] = '\n';
        // Записываем символы, которые содержатся вo второй строке и не содержатся в первой, в результат.
        for (i = 0; i < 128; ++i) {
            if (ascii1[i] == 0 && ascii2[i] == 1) {
                res[count++] = i;
            }
        }

        res[count++] = '\0';

        // Записываем результат в канал
        write(fd2[1], res, strlen(res) + 1);

        // Закрываем используемый конец канала
        close(fd2[1]);

        // Завершаем работу процесса
        exit(0);
    }

    // Создаем третий процесс
    pid3 = fork();

    if (pid3 == -1) {
        perror("Error while forking third process");
        return 1;
    }

    if (pid3 == 0) {
        // Третий дочерний процесс

        // Закрываем неиспользуемые концы каналов
        close(fd1[0]);
        close(fd1[1]);
        close(fd2[1]);
        close(fd3[0]);

        // Читаем данные из канала
        read(fd2[0], res, buffer_size);

        // Записываем результат в файл
        fwrite(res, sizeof(char), strlen(res), output_file);

        // Закрываем используемые файл и каналы
        fclose(output_file);
        close(fd2[0]);

        // Завершаем работу процесса
        exit(0);
    }

    // Закрываем неиспользуемые концы каналов
    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);
    close(fd3[0]);
    close(fd3[1]);

    // Ожидаем завершения всех дочерних процессов
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return 0;

}