#ifndef CARD_OPERATIONS_H
#define CARD_OPERATIONS_H

#include "card_domain.h"

/**
 * Слой ядра (Core Layer)
 * Определяет основные операции с картой
 */

/**
 * Интерфейс сервиса карт (порт)
 * Определяет бизнес-операции для работы с картой
 */
typedef struct {
    /**
     * Чтение данных с карты по указанному адресу
     * @param context Контекст карты
     * @param address Адрес начала чтения
     * @param length Количество байт для чтения
     * @param data Буфер для сохранения прочитанных данных
     * @return Код ошибки из CardError
     */
    int (*read_data)(void* service, uint8_t address, size_t length, CardData* data);
    
    /**
     * Запись данных на карту по указанному адресу
     * @param context Контекст карты
     * @param address Адрес начала записи
     * @param data Данные для записи
     * @return Код ошибки из CardError
     */
    int (*write_data)(void* service, uint8_t address, const CardData* data);
    
    /**
     * Перезапись данных на карте по указанному адресу
     * @param context Контекст карты
     * @param address Адрес начала перезаписи
     * @param data Данные для перезаписи
     * @return Код ошибки из CardError
     */
    int (*rewrite_data)(void* service, uint8_t address, const CardData* data);
    
    /**
     * Отправка произвольной команды на карту
     * @param context Контекст карты
     * @param command Команда для отправки
     * @param response Буфер для ответа
     * @return Код ошибки из CardError
     */
    int (*execute_command)(void* service, const CardData* command, CardData* response);
} CardOperations;

#endif /* CARD_OPERATIONS_H */ 