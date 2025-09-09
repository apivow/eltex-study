#include <stdio.h>
#include <locale.h>

int main() {
    setlocale(LC_ALL, "rus");
    const char* file_name = "output.txt";
    const char* text = "String from file";

    FILE *file = fopen(file_name, "w");
    if (file == NULL) {
        perror("Не удалось открыть файл для записи");
        return 1;
    }
    if (fputs(text, file) == EOF) {
        perror("Не удалось записать строку");
        fclose(file);
        return 1;
    }

    fclose(file);

    file = fopen(file_name, "rb");
    if (file == NULL) {
        perror("Не удалось открыть для записи");
        return 1;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Не удалость попасть в конец файла");
        fclose(file);
        return 1;
    }
    long f_size = ftell(file);
    if(f_size < 0) {
        perror("Не удалось получить размер файла");
        return 1;
    }
    
    for (long i = f_size - 1; i >= 0; --i) {
        if (fseek(file, i, SEEK_SET) != 0) {
            perror("Не удалось сдвинуть указатель чтения");
            fclose(file);
            return 1;
        }
        int ch = fgetc(file);
        if(ch == EOF) {
            perror("Не удалось прочитать символ");
            fclose(file);
            return 1;
        }
        putchar(ch);
    }
    putchar('\n');

    fclose(file);
    return 0;
}