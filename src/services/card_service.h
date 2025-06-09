#ifndef CARD_SERVICE_H
#define CARD_SERVICE_H

#include "card_domain.h"
#include "card_operations.h"

/**
 * Слой сервисов (Service Layer)
 * Реализует бизнес-логику работы с картой
 */

typedef struct {
    CardRepository* repository;
    CardContext* context;
} CardService;

/**
 * Инициализация сервиса карт
 * @param service Указатель на структуру сервиса
 * @param repository Репозиторий для работы с картой
 * @param context Контекст карты
 * @return Код ошибки из CardError
 */
int card_service_initialize(CardService* service, CardRepository* repository, CardContext* context);

/**
 * Подключение к считывателю карт
 * @param service Указатель на структуру сервиса
 * @param readerName Имя считывателя
 * @return Код ошибки из CardError
 */
int card_service_connect(CardService* service, const char* readerName);

/**
 * Отключение от считывателя карт
 * @param service Указатель на структуру сервиса
 * @return Код ошибки из CardError
 */
int card_service_disconnect(CardService* service);

/**
 * Освобождение ресурсов сервиса
 * @param service Указатель на структуру сервиса
 * @return Код ошибки из CardError
 */
int card_service_release(CardService* service);

/**
 * Чтение данных с карты
 * @param service Указатель на структуру сервиса
 * @param address Адрес для чтения
 * @param length Количество байт для чтения
 * @param data Буфер для прочитанных данных
 * @return Код ошибки из CardError
 */
int card_service_read_data(CardService* service, uint8_t address, size_t length, CardData* data);

/**
 * Запись данных на карту
 * @param service Указатель на структуру сервиса
 * @param address Адрес для записи
 * @param data Данные для записи
 * @return Код ошибки из CardError
 */
int card_service_write_data(CardService* service, uint8_t address, const CardData* data);

/**
 * Перезапись данных на карте
 * @param service Указатель на структуру сервиса
 * @param address Адрес для перезаписи
 * @param data Данные для перезаписи
 * @return Код ошибки из CardError
 */
int card_service_rewrite_data(CardService* service, uint8_t address, const CardData* data);

/**
 * Отправка произвольной команды на карту
 * @param service Указатель на структуру сервиса
 * @param command Команда для отправки
 * @param response Буфер для ответа
 * @return Код ошибки из CardError
 */
int card_service_execute_command(CardService* service, const CardData* command, CardData* response);

/**
 * Получение операций карты (реализация интерфейса CardOperations)
 * @param service Указатель на структуру сервиса
 * @return Структура с операциями
 */
CardOperations card_service_get_operations(CardService* service);

#endif /* CARD_SERVICE_H */ 