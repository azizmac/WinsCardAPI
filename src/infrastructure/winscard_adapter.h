#ifndef WINSCARD_ADAPTER_H
#define WINSCARD_ADAPTER_H

#include <windows.h>
#include <winscard.h>
#include "card_domain.h"

/**
 * Слой инфраструктуры (Infrastructure Layer)
 * Адаптер для работы с WinSCard API
 */

typedef struct {
    SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;
    char readerName[256];
    int isConnected;
} WinScardContext;

/**
 * Инициализация контекста WinSCard
 * @param context Контекст карты с WinScardContext внутри
 * @return Код ошибки из CardError
 */
int winscard_initialize(CardContext* context);

/**
 * Получение списка доступных считывателей
 * @param context Контекст карты с WinScardContext внутри
 * @param readers Указатель на массив строк с именами считывателей (будет выделена память)
 * @param readersCount Количество найденных считывателей
 * @return Код ошибки из CardError
 */
int winscard_list_readers(CardContext* context, char*** readers, size_t* readersCount);

/**
 * Освобождение памяти, выделенной для списка считывателей
 * @param readers Массив строк с именами считывателей
 * @param readersCount Количество считывателей
 */
void winscard_free_readers(char** readers, size_t readersCount);

/**
 * Подключение к считывателю карт
 * @param context Контекст карты с WinScardContext внутри
 * @param readerName Имя считывателя
 * @return Код ошибки из CardError
 */
int winscard_connect(CardContext* context, const char* readerName);

/**
 * Отключение от считывателя карт
 * @param context Контекст карты с WinScardContext внутри
 * @return Код ошибки из CardError
 */
int winscard_disconnect(CardContext* context);

/**
 * Освобождение ресурсов WinSCard
 * @param context Контекст карты с WinScardContext внутри
 * @return Код ошибки из CardError
 */
int winscard_release(CardContext* context);

/**
 * Отправка команды на карту и получение ответа
 * @param context Контекст карты с WinScardContext внутри
 * @param command Команда для отправки
 * @param commandLength Длина команды
 * @param response Буфер для ответа
 * @param responseLength Указатель на переменную с длиной буфера ответа
 * @return Код ошибки из CardError
 */
int winscard_transmit(CardContext* context, const uint8_t* command, size_t commandLength, 
                      uint8_t* response, size_t* responseLength);

/**
 * Создание репозитория карт, использующего WinSCard
 * @return Структура репозитория с функциями WinSCard
 */
CardRepository winscard_create_repository();

#endif /* WINSCARD_ADAPTER_H */ 