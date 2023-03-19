#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>


const int buffer_size = 5000;

int main(int argc, char *argv[]) {
    int fifo1, fifo2;
    pid_t pid1, pid2, pid3;

    char buffer[buffer_size], res[buffer_size];

    char name1[] = "fifo1";
    char name2[] = "fifo2";
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

    // Создаем именованные каналы
    if (mkfifo("fifo1", 0666) == -1 || mkfifo("fifo2", 0666) == -1) {
        perror("Error while creating named pipes");
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

        // Запись содержимого входного файла в буфер
        char input_buffer[buffer_size];
        size_t input_size = fread(input_buffer, 1, buffer_size, input_file);
        fclose(input_file);

        // Запись содержимого входного файла в именованный канал
        fifo1 = open("fifo1", O_WRONLY);
        write(fifo1, input_buffer, input_size);
        close(fifo1);

        // Завершаем работу процесса
        exit(0);
    }

    // Создаём именованные каналы
    mknod(name1, S_IFIFO | 0666, 0);
    mknod(name2, S_IFIFO | 0666, 0);
    // Создаем второй процесс
    pid2 = fork();

    if (pid2 == -1) {
        perror("Error while forking second process");
        return 1;
    }

    if (pid2 == 0) {
        // Дочерний процесс - второй процесс

        // Читаем данные из named pipe
        fifo1 = open("fifo1", O_RDONLY);
        read(fifo1, &buffer, buffer_size);
        close(fifo1);
        // Создаём массивы для обоих строк, в которых будем отмечать вхождение того или иного символа ASCII в строку.
        int ascii1[128] = {0};
        int ascii2[128] = {0};
        int i, count = 0;
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

        // Записываем результат в именнованный канал.
        fifo2 = open("fifo2", O_WRONLY);
        write(fifo2, res, strlen(res) + 1);
        close(fifo2);

        // Завершаем работу процесса
        exit(0);
    }

    // Создаем третий процесс
    pid3 = fork();

    if (pid3 == -1) {
        perror("Error forking third process");
        return 1;
    }

    if (pid3 == 0) {
        // Третий дочерний процесс

        // Читаем данные из named pipe
        fifo2 = open("fifo2", O_RDONLY);
        read(fifo2, res, buffer_size);
        close(fifo2);

        // Записываем результат в файл
        fwrite(res, sizeof(char), strlen(res), output_file);

        // Завершаем работу процесса
        exit(0);
    }

    close(fifo1);
    close(fifo2);

    // Ожидаем завершения всех дочерних процессов
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    // Удаляем именованные каналы
    unlink("fifo1");
    unlink("fifo2");

    return 0;
}
