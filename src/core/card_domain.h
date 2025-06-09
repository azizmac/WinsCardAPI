#ifndef CARD_DOMAIN_H
#define CARD_DOMAIN_H

#include <stdint.h>
#include <stdlib.h>

/**
 * Слой ядра (Core Layer)
 * Определяет основные сущности и интерфейсы для работы со смарт-картами
 */

typedef struct {
    uint8_t* data;
    size_t length;
} CardData;

typedef struct {
    void* context;
} CardContext;

/**
 * Интерфейс репозитория карт (порт)
 * Определяет методы для взаимодействия с физической картой
 */
typedef struct {
    int (*initialize)(CardContext* context);
    int (*list_readers)(CardContext* context, char** readers, size_t* readersCount);
    int (*connect)(CardContext* context, const char* reader);
    int (*disconnect)(CardContext* context);
    int (*release)(CardContext* context);
    int (*transmit)(CardContext* context, const uint8_t* command, size_t commandLength, 
                    uint8_t* response, size_t* responseLength);
} CardRepository;

/**
 * Коды ошибок домена
 */
typedef enum {
    CARD_SUCCESS = 0,
    CARD_ERROR_INIT_FAILED = -1,
    CARD_ERROR_CONNECT_FAILED = -2,
    CARD_ERROR_TRANSMIT_FAILED = -3,
    CARD_ERROR_INVALID_PARAMETER = -4,
    CARD_ERROR_MEMORY_ALLOCATION = -5
} CardError;

/**
 * Функции для создания и освобождения структуры CardData
 */
CardData* card_data_create(size_t length);
void card_data_free(CardData* data);
int card_data_copy(CardData* dest, const CardData* src);

#endif /* CARD_DOMAIN_H */ 