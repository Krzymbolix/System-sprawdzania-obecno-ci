/*
 * database.h
 *
 *  Created on: Jun 12, 2025
 *      Author: 7gdob
 */

#ifndef INC_DATABASE_H_
#define INC_DATABASE_H_

#include "stm32l4xx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char uuid[9];
    char imie[30];
    char nazwisko[30];
    const uint8_t* zdjecie;
} Nauczyciel;

typedef struct {
    char uuid[9];
    char nr_albumu[7];
    char imie[30];
    char nazwisko[30];
    const uint8_t* zdjecie;
} Student;

typedef struct {
    uint16_t id;
    char nazwa_przedmiotu[6];
    char uuid_nauczyciela[9];
    uint32_t data_start;
    uint32_t data_koniec;
} Terminarz;

char (*findListPointer(char* name))[9];
void init_database(void);
void* findStudentId(char* uuid);
void* findTeacherId(char* uuid);
uint16_t getLessonIds(char* uuid);
uint8_t isStudentOnList(char* uuid, uint16_t id);

#endif /* INC_DATABASE_H_ */
