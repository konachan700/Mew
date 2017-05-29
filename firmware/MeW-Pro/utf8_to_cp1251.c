/* Библиотека на языке C для конвертации строк из кодировки UTF-8 в CP1251
 * Адрес проекта: https://github.com/bravikov/utf8_to_cp1251
 * */

#include "utf8_to_cp1251.h"
#include "bit.h"
#include <stdbool.h>

#ifndef UTF8_TO_CP1251_LINEAR_SEARCH
    #include <stdlib.h>
#endif

/* Таблица преобразования кодов. Отсортирована по возрастанию кодов Unicode
 * для двоичного поиска. */
static const Cp1251 cp1251Table[] = {
 /* {0x98, 0x0098}, */
    {0xA0, 0x00A0}, /* NO-BREAK SPACE */
    {0xA4, 0x00A4}, /* CURRENCY SIGN */
    {0xA6, 0x00A6}, /* BROKEN BAR */
    {0xA7, 0x00A7}, /* SECTION SIGN */
    {0xA9, 0x00A9}, /* COPYRIGHT SIGN */
    {0xAB, 0x00AB}, /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
    {0xAC, 0x00AC}, /* NOT SIGN */
    {0xAD, 0x00AD}, /* SOFT HYPHEN */
    {0xAE, 0x00AE}, /* REGISTERED SIGN */
    {0xB0, 0x00B0}, /* DEGREE SIGN */
    {0xB1, 0x00B1}, /* PLUS-MINUS SIGN */
    {0xB5, 0x00B5}, /* MICRO SIGN */
    {0xB6, 0x00B6}, /* PILCROW SIGN */
    {0xB7, 0x00B7}, /* MIDDLE DOT */
    {0xBB, 0x00BB}, /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
    {0xA8, 0x0401}, /* CYRILLIC CAPITAL LETTER IO */
    {0x80, 0x0402}, /* CYRILLIC CAPITAL LETTER DJE */
    {0x81, 0x0403}, /* CYRILLIC CAPITAL LETTER GJE */
    {0xAA, 0x0404}, /* CYRILLIC CAPITAL LETTER UKRAINIAN IE */
    {0xBD, 0x0405}, /* CYRILLIC CAPITAL LETTER DZE */
    {0xB2, 0x0406}, /* CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */
    {0xAF, 0x0407}, /* CYRILLIC CAPITAL LETTER YI */
    {0xA3, 0x0408}, /* CYRILLIC CAPITAL LETTER JE */
    {0x8A, 0x0409}, /* CYRILLIC CAPITAL LETTER LJE */
    {0x8C, 0x040A}, /* CYRILLIC CAPITAL LETTER NJE */
    {0x8E, 0x040B}, /* CYRILLIC CAPITAL LETTER TSHE */
    {0x8D, 0x040C}, /* CYRILLIC CAPITAL LETTER KJE */
    {0xA1, 0x040E}, /* CYRILLIC CAPITAL LETTER SHORT U */
    {0x8F, 0x040F}, /* CYRILLIC CAPITAL LETTER DZHE */
    {0xB8, 0x0451}, /* CYRILLIC SMALL LETTER IO */
    {0x90, 0x0452}, /* CYRILLIC SMALL LETTER DJE */
    {0x83, 0x0453}, /* CYRILLIC SMALL LETTER GJE */
    {0xBA, 0x0454}, /* CYRILLIC SMALL LETTER UKRAINIAN IE */
    {0xBE, 0x0455}, /* CYRILLIC SMALL LETTER DZE */
    {0xB3, 0x0456}, /* CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */
    {0xBF, 0x0457}, /* CYRILLIC SMALL LETTER YI */
    {0xBC, 0x0458}, /* CYRILLIC SMALL LETTER JE */
    {0x9A, 0x0459}, /* CYRILLIC SMALL LETTER LJE */
    {0x9C, 0x045A}, /* CYRILLIC SMALL LETTER NJE */
    {0x9E, 0x045B}, /* CYRILLIC SMALL LETTER TSHE */
    {0x9D, 0x045C}, /* CYRILLIC SMALL LETTER KJE */
    {0xA2, 0x045E}, /* CYRILLIC SMALL LETTER SHORT U */
    {0x9F, 0x045F}, /* CYRILLIC SMALL LETTER DZHE */
    {0xA5, 0x0490}, /* CYRILLIC CAPITAL LETTER GHE WITH UPTURN */
    {0xB4, 0x0491}, /* CYRILLIC SMALL LETTER GHE WITH UPTURN */
    {0x96, 0x2013}, /* EN DASH */
    {0x97, 0x2014}, /* EM DASH */
    {0x91, 0x2018}, /* LEFT SINGLE QUOTATION MARK */
    {0x92, 0x2019}, /* RIGHT SINGLE QUOTATION MARK */
    {0x82, 0x201A}, /* SINGLE LOW-9 QUOTATION MARK */
    {0x93, 0x201C}, /* LEFT DOUBLE QUOTATION MARK */
    {0x94, 0x201D}, /* RIGHT DOUBLE QUOTATION MARK */
    {0x84, 0x201E}, /* DOUBLE LOW-9 QUOTATION MARK */
    {0x86, 0x2020}, /* DAGGER */
    {0x87, 0x2021}, /* DOUBLE DAGGER */
    {0x95, 0x2022}, /* BULLET */
    {0x85, 0x2026}, /* HORIZONTAL ELLIPSIS */
    {0x89, 0x2030}, /* PER MILLE SIGN */
    {0x8B, 0x2039}, /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
    {0x9B, 0x203A}, /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
    {0x88, 0x20AC}, /* EURO SIGN */
    {0xB9, 0x2116}, /* NUMERO SIGN */
    {0x99, 0x2122}  /* TRADE MARK SIGN */
};

static const Cp1251 * customCp1251Table = 0;
static size_t customCp1251TableSize = 0;

void setCustomCp1251Table(const Cp1251 * table, const size_t count) {
    customCp1251Table = table;
    customCp1251TableSize = count;
}

#ifndef UTF8_TO_CP1251_LINEAR_SEARCH
/* Функция сравнения для двоичного поиска. */
static int compareConversion (const void * s1, const void * s2) {
    return ((Cp1251 *)s1)->unicode - ((Cp1251 *)s2)->unicode;
}
#endif

#define MAX_NUMBER_OF_UTF8_OCTETS (6)

int convertUtf8ToCp1251(const char * utf8, char * cp1251)
{
    const uint8_t firstOctetMask[MAX_NUMBER_OF_UTF8_OCTETS] = {
        b1000_0000,
        b1110_0000,
        b1111_0000,
        b1111_1000,
        b1111_1100,
        b1111_1110
    };
    const uint8_t firstOctetTemplate[MAX_NUMBER_OF_UTF8_OCTETS] = {
        b0000_0000,
        b1100_0000,
        b1110_0000,
        b1111_0000,
        b1111_1000,
        b1111_1100
    };
    const uint8_t secondOctetMask = b1100_0000;
    const uint8_t secondOctetTemplate = b1000_0000;

    size_t numberOfRemainingOctets = 0;
    uint32_t unicode = 0;
    size_t i = 0;
    size_t cp1251_i = 0;
    
    while (utf8[i] != '\0') {
        const uint8_t octet = utf8[i++];
        
        if (numberOfRemainingOctets == 0) {
            bool error = true;
            size_t j;

            for (j = 0; j < MAX_NUMBER_OF_UTF8_OCTETS; j++) {
                const uint8_t octetMask = firstOctetMask[j];
                const uint8_t octetTemplate = firstOctetTemplate[j];

                if ((octet & octetMask) == octetTemplate) {
                    unicode = octet & ~octetMask;

                    if (j == 0) {
                        /* Обнаружен символ US-ASCII */
                        cp1251[cp1251_i++] = unicode;
                    }

                    numberOfRemainingOctets = j;
                    error = false;
                    break;
                }
            }

            if (error) {
                /* Ошибка UTF-8. */
                return -1;
            }
        }
        else {
            if ((octet & secondOctetMask) == secondOctetTemplate) {
                unicode <<= 6;
                unicode |= octet & ~secondOctetMask;
                numberOfRemainingOctets--;
                
                if (numberOfRemainingOctets == 0) {
                    if (0x410 <= unicode && unicode <= 0x44F) {
                        cp1251[cp1251_i++] = 0xC0 + unicode - 0x410;
                    }
                    else {
                        const Cp1251 * tables[] = {
                            customCp1251Table,
                            cp1251Table
                        };
                        const size_t tableSizes[] = {
                            customCp1251TableSize,
                            sizeof(cp1251Table) / sizeof(Cp1251),
                        };

                        size_t t;

#                       ifdef UTF8_TO_CP1251_LINEAR_SEARCH
                        bool found = false;
#                       endif

                        for (t = 0; t < 2; t++) {
#                           ifdef UTF8_TO_CP1251_LINEAR_SEARCH
                            /* Линейный (последовательный) поиск */
                            size_t u;
                            for (u = 0; u < tableSizes[t]; u++) {
                                if (unicode == tables[t][u].unicode) {
                                    cp1251[cp1251_i++] =
                                        tables[t][u].cp1251;
                                    found = true;
                                    break;
                                }
                            }

                            if (found) {
                                break;
                            }

#                           else
                            /* Двоичный поиск */
                            const Cp1251 key = {0, unicode};

                            const Cp1251 * conversion = bsearch(
                                &key,
                                tables[t],
                                tableSizes[t],
                                sizeof(Cp1251),
                                compareConversion
                            );

                            if (conversion) {
                                cp1251[cp1251_i++] = conversion->cp1251;
                                break;
                            }
#                           endif
                        }
                    }
                }
            }
            else {
                /* Ошибка UTF-8. */
                return -1;
            }
        }
    }

    cp1251[cp1251_i] = '\0';
    
    return cp1251_i;
}
