#include "winscard_adapter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static WinScardContext* get_winscard_context(CardContext* context) {
    if (!context || !context->context) {
        return NULL;
    }
    return (WinScardContext*)context->context;
}

int winscard_initialize(CardContext* context) {
    if (!context || !context->context) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    WinScardContext* winscardContext = get_winscard_context(context);
    
    // Очистка структуры контекста
    memset(winscardContext, 0, sizeof(WinScardContext));
    winscardContext->isConnected = 0;
    
    // Установка контекста ресурса смарт-карты
    LONG result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &(winscardContext->hContext));
    if (result != SCARD_S_SUCCESS) {
        printf("Ошибка при установке контекста смарт-карты: %X\n", (unsigned int)result);
        return CARD_ERROR_INIT_FAILED;
    }
    
    return CARD_SUCCESS;
}

int winscard_list_readers(CardContext* context, char*** readers, size_t* readersCount) {
    if (!context || !context->context || !readers || !readersCount) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    WinScardContext* winscardContext = get_winscard_context(context);
    
    DWORD readersBufferSize = 0;
    LONG result;
    
    // Получаем размер буфера для списка считывателей
    result = SCardListReaders(winscardContext->hContext, NULL, NULL, &readersBufferSize);
    if (result != SCARD_S_SUCCESS) {
        if (result == SCARD_E_NO_READERS_AVAILABLE) {
            *readersCount = 0;
            *readers = NULL;
            return CARD_SUCCESS;
        }
        printf("Ошибка при получении размера буфера считывателей: %X\n", (unsigned int)result);
        return CARD_ERROR_INIT_FAILED;
    }
    
    // Выделяем память для буфера
    char* readersBuffer = (char*)malloc(readersBufferSize);
    if (!readersBuffer) {
        return CARD_ERROR_MEMORY_ALLOCATION;
    }
    
    // Получаем список считывателей
    result = SCardListReaders(winscardContext->hContext, NULL, readersBuffer, &readersBufferSize);
    if (result != SCARD_S_SUCCESS) {
        free(readersBuffer);
        printf("Ошибка при получении списка считывателей: %X\n", (unsigned int)result);
        return CARD_ERROR_INIT_FAILED;
    }
    
    // Подсчет количества считывателей (разделены нулевыми байтами)
    size_t count = 0;
    char* currentReader = readersBuffer;
    while (*currentReader != '\0') {
        count++;
        currentReader += strlen(currentReader) + 1;
    }
    
    *readersCount = count;
    if (count == 0) {
        free(readersBuffer);
        *readers = NULL;
        return CARD_SUCCESS;
    }
    
    // Выделяем память для массива указателей на имена считывателей
    *readers = (char**)malloc(count * sizeof(char*));
    if (!*readers) {
        free(readersBuffer);
        return CARD_ERROR_MEMORY_ALLOCATION;
    }
    
    // Заполняем массив указателей
    currentReader = readersBuffer;
    for (size_t i = 0; i < count; i++) {
        size_t len = strlen(currentReader) + 1;
        (*readers)[i] = (char*)malloc(len);
        if (!(*readers)[i]) {
            // Освобождаем ранее выделенную память
            for (size_t j = 0; j < i; j++) {
                free((*readers)[j]);
            }
            free(*readers);
            free(readersBuffer);
            return CARD_ERROR_MEMORY_ALLOCATION;
        }
        strcpy((*readers)[i], currentReader);
        currentReader += len;
    }
    
    free(readersBuffer);
    return CARD_SUCCESS;
}

void winscard_free_readers(char** readers, size_t readersCount) {
    if (!readers) {
        return;
    }
    
    for (size_t i = 0; i < readersCount; i++) {
        free(readers[i]);
    }
    free(readers);
}

int winscard_connect(CardContext* context, const char* readerName) {
    if (!context || !context->context || !readerName) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    WinScardContext* winscardContext = get_winscard_context(context);
    
    // Сохраняем имя считывателя
    strncpy(winscardContext->readerName, readerName, sizeof(winscardContext->readerName) - 1);
    
    // Подключаемся к карте в указанном считывателе
    LONG result = SCardConnect(winscardContext->hContext, 
                           winscardContext->readerName,
                           SCARD_SHARE_SHARED, 
                           SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                           &(winscardContext->hCard), 
                           &(winscardContext->dwActiveProtocol));
    
    if (result != SCARD_S_SUCCESS) {
        printf("Ошибка при подключении к карте: %X\n", (unsigned int)result);
        return CARD_ERROR_CONNECT_FAILED;
    }
    
    winscardContext->isConnected = 1;
    return CARD_SUCCESS;
}

int winscard_disconnect(CardContext* context) {
    if (!context || !context->context) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    WinScardContext* winscardContext = get_winscard_context(context);
    
    if (winscardContext->isConnected) {
        LONG result = SCardDisconnect(winscardContext->hCard, SCARD_LEAVE_CARD);
        if (result != SCARD_S_SUCCESS) {
            printf("Ошибка при отключении от карты: %X\n", (unsigned int)result);
            return CARD_ERROR_CONNECT_FAILED;
        }
        winscardContext->isConnected = 0;
    }
    
    return CARD_SUCCESS;
}

int winscard_release(CardContext* context) {
    if (!context || !context->context) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    WinScardContext* winscardContext = get_winscard_context(context);
    
    // Отключаемся от карты, если подключены
    if (winscardContext->isConnected) {
        winscard_disconnect(context);
    }
    
    // Освобождаем контекст
    if (winscardContext->hContext) {
        LONG result = SCardReleaseContext(winscardContext->hContext);
        if (result != SCARD_S_SUCCESS) {
            printf("Ошибка при освобождении контекста: %X\n", (unsigned int)result);
            return CARD_ERROR_INIT_FAILED;
        }
        winscardContext->hContext = 0;
    }
    
    return CARD_SUCCESS;
}

int winscard_transmit(CardContext* context, const uint8_t* command, size_t commandLength, 
                     uint8_t* response, size_t* responseLength) {
    if (!context || !context->context || !command || !response || !responseLength) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    WinScardContext* winscardContext = get_winscard_context(context);
    
    if (!winscardContext->isConnected) {
        printf("Нет подключения к карте\n");
        return CARD_ERROR_CONNECT_FAILED;
    }
    
    SCARD_IO_REQUEST ioRequest;
    
    // Определяем структуру ввода-вывода в зависимости от активного протокола
    if (winscardContext->dwActiveProtocol == SCARD_PROTOCOL_T0) {
        ioRequest = *SCARD_PCI_T0;
    } else if (winscardContext->dwActiveProtocol == SCARD_PROTOCOL_T1) {
        ioRequest = *SCARD_PCI_T1;
    } else {
        printf("Неподдерживаемый протокол\n");
        return CARD_ERROR_TRANSMIT_FAILED;
    }
    
    DWORD dwResponseLength = (DWORD)*responseLength;
    
    // Отправляем команду на карту и получаем ответ
    LONG result = SCardTransmit(winscardContext->hCard, &ioRequest, command, (DWORD)commandLength,
                          NULL, response, &dwResponseLength);
    
    if (result != SCARD_S_SUCCESS) {
        printf("Ошибка при передаче данных карте: %X\n", (unsigned int)result);
        return CARD_ERROR_TRANSMIT_FAILED;
    }
    
    *responseLength = dwResponseLength;
    return CARD_SUCCESS;
}

CardRepository winscard_create_repository() {
    CardRepository repository = {
        .initialize = winscard_initialize,
        .list_readers = winscard_list_readers,
        .connect = winscard_connect,
        .disconnect = winscard_disconnect,
        .release = winscard_release,
        .transmit = winscard_transmit
    };
    
    return repository;
} 