#include <stdio.h>
#include <stdlib.h>
#include "card_domain.h"
#include "card_operations.h"
#include "card_service.h"
#include "winscard_adapter.h"

void print_hex_data(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

void display_readers(char** readers, size_t readersCount) {
    printf("\nДоступные считыватели карт:\n");
    for (size_t i = 0; i < readersCount; i++) {
        printf("%zu. %s\n", i + 1, readers[i]);
    }
}

int select_reader(CardService* service, CardContext* context) {
    char** readers = NULL;
    size_t readersCount = 0;
    
    // Получаем список считывателей
    int result = winscard_list_readers(context, &readers, &readersCount);
    if (result != CARD_SUCCESS) {
        printf("Ошибка при получении списка считывателей: %d\n", result);
        return result;
    }
    
    if (readersCount == 0) {
        printf("Считыватели не найдены.\n");
        return CARD_ERROR_INIT_FAILED;
    }
    
    if (readersCount == 1) {
        printf("Найден один считыватель: %s. Используем его.\n", readers[0]);
        result = card_service_connect(service, readers[0]);
        winscard_free_readers(readers, readersCount);
        return result;
    }
    
    // Вывод считывателей для выбора
    display_readers(readers, readersCount);
    
    size_t selectedIndex;
    printf("Выберите считыватель (1-%zu): ", readersCount);
    scanf("%zu", &selectedIndex);
    
    if (selectedIndex < 1 || selectedIndex > readersCount) {
        printf("Неверный выбор.\n");
        winscard_free_readers(readers, readersCount);
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    result = card_service_connect(service, readers[selectedIndex - 1]);
    winscard_free_readers(readers, readersCount);
    return result;
}

void read_card_example(CardService* service) {
    uint8_t address = 0x00;
    size_t length = 16;
    CardData data = { NULL, 0 };
    
    printf("\nОтправка команды чтения на карту...\n");
    
    int result = card_service_read_data(service, address, length, &data);
    if (result == CARD_SUCCESS) {
        printf("Получены данные (длина: %zu байт):\n", data.length - 2);
        print_hex_data(data.data, data.length);
        
        if (data.length >= 2) {
            uint8_t sw1 = data.data[data.length - 2];
            uint8_t sw2 = data.data[data.length - 1];
            
            if (sw1 == 0x90 && sw2 == 0x00) {
                printf("Операция успешно завершена (SW1=90, SW2=00)\n");
            } else {
                printf("Статус выполнения команды: SW1=%02X, SW2=%02X\n", sw1, sw2);
            }
        }
    } else {
        printf("Ошибка при чтении данных: %d\n", result);
    }
    
    if (data.data) {
        free(data.data);
    }
}

void write_card_example(CardService* service) {
    uint8_t dataToWrite[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t address = 0x10;
    
    CardData data = { NULL, 0 };
    data.data = (uint8_t*)malloc(sizeof(dataToWrite));
    if (!data.data) {
        printf("Ошибка выделения памяти\n");
        return;
    }
    
    memcpy(data.data, dataToWrite, sizeof(dataToWrite));
    data.length = sizeof(dataToWrite);
    
    printf("\nЗапись данных на карту по адресу %02X...\n", address);
    printf("Данные для записи:\n");
    print_hex_data(data.data, data.length);
    
    int result = card_service_rewrite_data(service, address, &data);
    if (result == CARD_SUCCESS) {
        printf("Запись успешно выполнена\n");
        
        printf("\nЧтение данных для проверки записи...\n");
        CardData readData = { NULL, 0 };
        
        result = card_service_read_data(service, address, data.length, &readData);
        if (result == CARD_SUCCESS) {
            printf("Прочитанные данные:\n");
            print_hex_data(readData.data, readData.length);
            free(readData.data);
        } else {
            printf("Ошибка при чтении данных: %d\n", result);
        }
    } else {
        printf("Ошибка при записи данных: %d\n", result);
    }
    
    free(data.data);
}

int main() {
    printf("Сервис работы со смарт-картами (Луковая архитектура)\n");
    printf("===================================================\n");
    
    // Инициализация адаптера для WinSCard
    WinScardContext winscardContext;
    CardContext cardContext = { &winscardContext };
    
    // Инициализация репозитория
    CardRepository repository = winscard_create_repository();
    
    // Инициализация сервиса
    CardService service;
    int result = card_service_initialize(&service, &repository, &cardContext);
    if (result != CARD_SUCCESS) {
        printf("Не удалось инициализировать сервис смарт-карт: %d\n", result);
        return 1;
    }
    
    // Выбор считывателя и подключение к карте
    result = select_reader(&service, &cardContext);
    if (result != CARD_SUCCESS) {
        printf("Не удалось подключиться к считывателю карт: %d\n", result);
        card_service_release(&service);
        return 1;
    }
    
    printf("\nПодключение к карте успешно установлено\n");
    
    // Главное меню
    int choice;
    do {
        printf("\nМеню:\n");
        printf("1. Прочитать данные с карты\n");
        printf("2. Записать данные на карту\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                read_card_example(&service);
                break;
            case 2:
                write_card_example(&service);
                break;
            case 0:
                printf("Выход из программы\n");
                break;
            default:
                printf("Неверный выбор. Пожалуйста, попробуйте снова.\n");
        }
    } while (choice != 0);
    
    // Освобождение ресурсов
    card_service_release(&service);
    
    return 0;
} 