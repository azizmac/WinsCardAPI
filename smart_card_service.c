#include "smart_card_service.h"

BOOL InitializeSmartCard(SmartCardContext* context) {
    LONG result;
    
    memset(context, 0, sizeof(SmartCardContext));
    context->isConnected = FALSE;
    
    result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &(context->hContext));
    if (result != SCARD_S_SUCCESS) {
        printf("Ошибка при установке контекста смарт-карты: %X\n", result);
        return FALSE;
    }
    
    return TRUE;
}

BOOL ListReaders(SmartCardContext* context, char* readers, DWORD* readersLength) {
    LONG result;
    
    result = SCardListReaders(context->hContext, NULL, readers, readersLength);
    if (result != SCARD_S_SUCCESS) {
        if (result == SCARD_E_NO_READERS_AVAILABLE) {
            printf("Нет доступных считывателей карт\n");
        } else {
            printf("Ошибка при получении списка считывателей: %X\n", result);
        }
        return FALSE;
    }
    
    return TRUE;
}

BOOL ConnectToCard(SmartCardContext* context, const char* readerName) {
    LONG result;
    
    strncpy(context->readerName, readerName, sizeof(context->readerName) - 1);
    
    result = SCardConnect(context->hContext, 
                          context->readerName,
                          SCARD_SHARE_SHARED, 
                          SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                          &(context->hCard), 
                          &(context->dwActiveProtocol));
    
    if (result != SCARD_S_SUCCESS) {
        printf("Ошибка при подключении к карте: %X\n", result);
        return FALSE;
    }
    
    context->isConnected = TRUE;
    return TRUE;
}

void DisconnectFromCard(SmartCardContext* context) {
    if (context->isConnected) {
        SCardDisconnect(context->hCard, SCARD_LEAVE_CARD);
        context->isConnected = FALSE;
    }
}

void ReleaseSmartCard(SmartCardContext* context) {
    DisconnectFromCard(context);
    
    if (context->hContext) {
        SCardReleaseContext(context->hContext);
        context->hContext = 0;
    }
}

BOOL ReadFromCard(SmartCardContext* context, BYTE* command, DWORD commandLength, 
                  BYTE* response, DWORD* responseLength) {
    LONG result;
    SCARD_IO_REQUEST ioRequest;
    
    if (!context->isConnected) {
        printf("Нет подключения к карте\n");
        return FALSE;
    }
    
    if (context->dwActiveProtocol == SCARD_PROTOCOL_T0) {
        ioRequest = *SCARD_PCI_T0;
    } else if (context->dwActiveProtocol == SCARD_PROTOCOL_T1) {
        ioRequest = *SCARD_PCI_T1;
    } else {
        printf("Неподдерживаемый протокол\n");
        return FALSE;
    }
    
    result = SCardTransmit(context->hCard, &ioRequest, command, commandLength,
                           NULL, response, responseLength);
    
    if (result != SCARD_S_SUCCESS) {
        printf("Ошибка при чтении данных с карты: %X\n", result);
        return FALSE;
    }
    
    return TRUE;
}

BOOL WriteToCard(SmartCardContext* context, BYTE* data, DWORD dataLength, 
                 BYTE* response, DWORD* responseLength) {
    return ReadFromCard(context, data, dataLength, response, responseLength);
}

BOOL RewriteCardData(SmartCardContext* context, BYTE address, 
                     BYTE* data, DWORD dataLength, 
                     BYTE* response, DWORD* responseLength) {
    BYTE* command = (BYTE*)malloc(5 + dataLength);
    if (!command) {
        printf("Ошибка выделения памяти\n");
        return FALSE;
    }
    
    command[0] = 0xFF;  
    command[1] = 0xD6;  
    command[2] = 0x00;  
    command[3] = address;  
    command[4] = (BYTE)dataLength;  
    memcpy(command + 5, data, dataLength);
    
    BOOL result = ReadFromCard(context, command, 5 + dataLength, response, responseLength);
    
    free(command);
    return result;
}

void PrintHexData(BYTE* data, DWORD dataLength) {
    for (DWORD i = 0; i < dataLength; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
} 