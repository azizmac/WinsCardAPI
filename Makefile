CC = gcc
CFLAGS = -Wall -Wextra -Isrc/core -Isrc/services -Isrc/infrastructure
LDFLAGS = -lwinscard

# Пути к исходным файлам по слоям
CORE_DIR = src/core
SERVICES_DIR = src/services
INFRA_DIR = src/infrastructure
UI_DIR = src/ui

# Исходные файлы по слоям
CORE_SOURCES = $(CORE_DIR)/card_domain.c
SERVICE_SOURCES = $(SERVICES_DIR)/card_service.c
INFRA_SOURCES = $(INFRA_DIR)/winscard_adapter.c
UI_SOURCES = $(UI_DIR)/main.c

# Все исходные файлы
SOURCES = $(CORE_SOURCES) $(SERVICE_SOURCES) $(INFRA_SOURCES) $(UI_SOURCES)
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = smart_card_app

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del $(CORE_DIR)\*.o
	del $(SERVICES_DIR)\*.o
	del $(INFRA_DIR)\*.o
	del $(UI_DIR)\*.o
	del $(EXECUTABLE).exe

run: $(EXECUTABLE)
	.\$(EXECUTABLE)

.PHONY: all clean run 