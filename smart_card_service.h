#ifndef SMART_CARD_SERVICE_H
#define SMART_CARD_SERVICE_H

#include <windows.h>
#include <winscard.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;
    char readerName[256];
    BOOL isConnected;
} SmartCardContext;

BOOL InitializeSmartCard(SmartCardContext* context);
BOOL ListReaders(SmartCardContext* context, char* readers, DWORD* readersLength);
BOOL ConnectToCard(SmartCardContext* context, const char* readerName);
void DisconnectFromCard(SmartCardContext* context);
void ReleaseSmartCard(SmartCardContext* context);
BOOL ReadFromCard(SmartCardContext* context, BYTE* command, DWORD commandLength, 
                  BYTE* response, DWORD* responseLength);
BOOL WriteToCard(SmartCardContext* context, BYTE* data, DWORD dataLength, 
                 BYTE* response, DWORD* responseLength);
BOOL RewriteCardData(SmartCardContext* context, BYTE address, 
                    BYTE* data, DWORD dataLength, 
                    BYTE* response, DWORD* responseLength);
void PrintHexData(BYTE* data, DWORD dataLength);

#endif 