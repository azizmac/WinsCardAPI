#include "card_service.h"
#include <string.h>

static int service_read_data_callback(void* service_ptr, uint8_t address, size_t length, CardData* data);
static int service_write_data_callback(void* service_ptr, uint8_t address, const CardData* data);
static int service_rewrite_data_callback(void* service_ptr, uint8_t address, const CardData* data);
static int service_execute_command_callback(void* service_ptr, const CardData* command, CardData* response);

int card_service_initialize(CardService* service, CardRepository* repository, CardContext* context) {
    if (!service || !repository || !context) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    service->repository = repository;
    service->context = context;
    
    return repository->initialize(context);
}

int card_service_connect(CardService* service, const char* readerName) {
    if (!service || !service->repository || !service->context || !readerName) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    return service->repository->connect(service->context, readerName);
}

int card_service_disconnect(CardService* service) {
    if (!service || !service->repository || !service->context) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    return service->repository->disconnect(service->context);
}

int card_service_release(CardService* service) {
    if (!service || !service->repository || !service->context) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    return service->repository->release(service->context);
}

int card_service_read_data(CardService* service, uint8_t address, size_t length, CardData* data) {
    if (!service || !service->repository || !service->context || !data) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    // Формирование APDU команды для чтения данных
    uint8_t commandData[5] = { 0xFF, 0xB0, 0x00, address, (uint8_t)length };
    CardData command = { commandData, 5 };
    
    return card_service_execute_command(service, &command, data);
}

int card_service_write_data(CardService* service, uint8_t address, const CardData* data) {
    if (!service || !service->repository || !service->context || !data || !data->data) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    // Формирование APDU команды для записи данных
    size_t commandLength = 5 + data->length;
    uint8_t* commandData = (uint8_t*)malloc(commandLength);
    if (!commandData) {
        return CARD_ERROR_MEMORY_ALLOCATION;
    }
    
    commandData[0] = 0xFF;    // CLA
    commandData[1] = 0xD0;    // INS (запись)
    commandData[2] = 0x00;    // P1
    commandData[3] = address; // P2 (адрес)
    commandData[4] = (uint8_t)data->length; // Lc (длина данных)
    memcpy(commandData + 5, data->data, data->length);
    
    CardData command = { commandData, commandLength };
    CardData response;
    response.data = (uint8_t*)malloc(258); // Максимальный размер ответа
    response.length = 258;
    
    int result = card_service_execute_command(service, &command, &response);
    
    free(commandData);
    free(response.data);
    
    return result;
}

int card_service_rewrite_data(CardService* service, uint8_t address, const CardData* data) {
    if (!service || !service->repository || !service->context || !data || !data->data) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    // Формирование APDU команды для перезаписи данных
    size_t commandLength = 5 + data->length;
    uint8_t* commandData = (uint8_t*)malloc(commandLength);
    if (!commandData) {
        return CARD_ERROR_MEMORY_ALLOCATION;
    }
    
    commandData[0] = 0xFF;    // CLA
    commandData[1] = 0xD6;    // INS (UPDATE BINARY)
    commandData[2] = 0x00;    // P1
    commandData[3] = address; // P2 (адрес)
    commandData[4] = (uint8_t)data->length; // Lc (длина данных)
    memcpy(commandData + 5, data->data, data->length);
    
    CardData command = { commandData, commandLength };
    CardData response;
    response.data = (uint8_t*)malloc(258); // Максимальный размер ответа
    response.length = 258;
    
    int result = card_service_execute_command(service, &command, &response);
    
    free(commandData);
    free(response.data);
    
    return result;
}

int card_service_execute_command(CardService* service, const CardData* command, CardData* response) {
    if (!service || !service->repository || !service->context || !command || !command->data || !response) {
        return CARD_ERROR_INVALID_PARAMETER;
    }
    
    // Инициализация буфера для ответа, если он не инициализирован
    if (!response->data) {
        response->data = (uint8_t*)malloc(258); // Максимальный размер ответа
        if (!response->data) {
            return CARD_ERROR_MEMORY_ALLOCATION;
        }
        response->length = 258;
    }
    
    size_t responseLength = response->length;
    int result = service->repository->transmit(
        service->context, 
        command->data, 
        command->length, 
        response->data, 
        &responseLength
    );
    
    response->length = responseLength;
    return result;
}

CardOperations card_service_get_operations(CardService* service) {
    CardOperations operations = {
        .read_data = service_read_data_callback,
        .write_data = service_write_data_callback,
        .rewrite_data = service_rewrite_data_callback,
        .execute_command = service_execute_command_callback
    };
    
    return operations;
}

static int service_read_data_callback(void* service_ptr, uint8_t address, size_t length, CardData* data) {
    return card_service_read_data((CardService*)service_ptr, address, length, data);
}

static int service_write_data_callback(void* service_ptr, uint8_t address, const CardData* data) {
    return card_service_write_data((CardService*)service_ptr, address, data);
}

static int service_rewrite_data_callback(void* service_ptr, uint8_t address, const CardData* data) {
    return card_service_rewrite_data((CardService*)service_ptr, address, data);
}

static int service_execute_command_callback(void* service_ptr, const CardData* command, CardData* response) {
    return card_service_execute_command((CardService*)service_ptr, command, response);
} 