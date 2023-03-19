# Операционные системы ИДЗ №1
 ### Русанов Андрей БПИ219
 ### Вариант 32
 Разработать программу, которая на основе анализа двух входных ASCII-строк формирует на выходе две другие строки. В первой из выводимых строк содержатся символы, которых нет во второй исходной строке. Во второй выводимой строке содержатся символы, отсутствующие в первой входной строке (разности символов). Каждый символ в соответствующей выходной строке должен встречаться только один раз.

## Формат ввода и вывода
Программа принимает входной и выходной текстовые файлы в качестве параметров коммандной строки. Во входном файле должны содержаться две строки, **разделённые символом '\n'**. Запустить программу можно введя в консоль следующие комманды:
```
gcc 4points.c
./a.out input1.txt output1.txt
```
Программа выводит в файл две строки(разности введённых строк), также разделённые символом '\n'.

## Решение на 4 балла
Решение на 4 балла представлено в файле [4points.c](4points/4points.c). Программа создает три процесса, где первый процесс читает содержимое входного файла и записывает его в неименованный канал, второй процесс считывает данные из канала, вычисляет результат и передаёт его во второй неименованный канал и третий процесс забирает результат  из этого канала и записывет его в файл.

## Решение на 5 баллов
Решение на 5 баллов представлено в файле [5points.c](5points/5points.c). Оно отличается от предыдущего тем, что использует именнованые каналы, которые создаются с помощью mknod().
## Решение на 6 баллов
Решение на 6 баллов представлено в файле [6points.c](6points/6points.c). Программа создает два неименованных процесса. Первый процесс читает данные из входного буфера и записывает их в первый канал, затем читает результат из второго канала и выводит его в файл. Второй процесс читает данные из первого канала, вычисляет результат и записывает его во второй канал.
## Решение на 7 баллов
Решение на 7 баллов представлено в файле [7points.c](7points/7points.c). Оно отличается от предыдущего тем, что использует именнованые каналы, которые создаются с помощью mknod().