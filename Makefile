CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lwinscard

CORE_SOURCES = card_domain.c
SERVICE_SOURCES = card_service.c
INFRA_SOURCES = winscard_adapter.c
UI_SOURCES = main.c

SOURCES = $(CORE_SOURCES) $(SERVICE_SOURCES) $(INFRA_SOURCES) $(UI_SOURCES)
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = smart_card_app

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del *.o
	del $(EXECUTABLE).exe

run: $(EXECUTABLE)
	.\$(EXECUTABLE)

.PHONY: all clean run 