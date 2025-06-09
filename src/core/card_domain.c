#include "card_domain.h"
#include <string.h>

CardData* card_data_create(size_t length) {
    if (length == 0) {
        return NULL;
    }
    
    CardData* data = (CardData*)malloc(sizeof(CardData));
    if (!data) {
        return NULL;
    }
    
    data->data = (uint8_t*)malloc(length);
    if (!data->data) {
        free(data);
        return NULL;
    }
    
    data->length = length;
    memset(data->data, 0, length);
    
    return data;
}

void card_data_free(CardData* data) {
    if (!data) {
        return;
    }
    
    if (data->data) {
        free(data->data);
        data->data = NULL;
    }
    
    free(data);
}

int card_data_copy(CardData* dest, const CardData* src) {
    if (!dest || !src || !src->data) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    // Если данные назначения не инициализированы или размер не соответствует
    if (!dest->data || dest->length < src->length) {
        // Освобождаем старый буфер, если он есть
        if (dest->data) {
            free(dest->data);
        }
        
        // Выделяем новый буфер
        dest->data = (uint8_t*)malloc(src->length);
        if (!dest->data) {
            dest->length = 0;
            return CARD_ERROR_MEMORY_ALLOCATION;
        }
        
        dest->length = src->length;
    }
    
    // Копируем данные
    memcpy(dest->data, src->data, src->length);
    
    return CARD_SUCCESS;
} 