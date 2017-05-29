/* Библиотека на языке C для конвертации строк из кодировки UTF-8 в CP1251
 * Адрес проекта: https://github.com/bravikov/utf8_to_cp1251
 * */

#pragma once
#ifndef UTF8_TO_CP1251_H_20170402_195220
#define UTF8_TO_CP1251_H_20170402_195220

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
    extern "C" {
#endif

/* Если символу Unicode не найдено соответсвие в CP1251, то символ пропускается.
 *
 * Строка utf8 должна кончаться нуль-символом '\0'.
*/
extern int convertUtf8ToCp1251(const char * utf8, char * cp1251);

typedef struct {
    uint8_t cp1251;
    uint32_t unicode;
} Cp1251;

extern void setCustomCp1251Table(const Cp1251 * table, const size_t count);

/* Раскоменируйте строчку ниже, чтобы для конвертации использовать линейный
 * (последовательный) поиск вместо двочного. Поиск требуется для сопоставления
 * кодов символов по таблице. Двоичный поиск быстрее и исползует функию bsearch
 * из stdlib.h. Использовать линейный поиск имеет смысл если есть ограничение
 * на использование стандартной библиотеки. */

/* #define UTF8_TO_CP1251_LINEAR_SEARCH */

#ifdef __cplusplus
    }
#endif

#endif /* #ifndef UTF8_TO_CP1251_H_20170402_195220 */
