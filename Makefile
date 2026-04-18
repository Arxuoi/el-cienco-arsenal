CC = gcc
CFLAGS = -Wall -O3 -pthread
TARGET = elcienco
API_TARGET = elcienco_api

SRC_DIR = src
CORE_DIR = src/core
UTILS_DIR = src/utils
API_DIR = api
INCLUDE_DIR = include

SOURCES = $(SRC_DIR)/main.c \
          $(CORE_DIR)/udp_flood.c \
          $(CORE_DIR)/tcp_syn.c \
          $(CORE_DIR)/http_flood.c \
          $(CORE_DIR)/slowloris.c \
          $(CORE_DIR)/dns_amp.c \
          $(CORE_DIR)/icmp_flood.c \
          $(CORE_DIR)/rudyloris.c \
          $(CORE_DIR)/cloudflare_bypass.c \
          $(UTILS_DIR)/socket_utils.c \
          $(UTILS_DIR)/packet_builder.c \
          $(UTILS_DIR)/ip_spoof.c \
          $(API_DIR)/server.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $(SOURCES) -o $(TARGET)
	@echo "[+] El Cienco compiled successfully"

termux: $(SOURCES)
	clang $(CFLAGS) -I$(INCLUDE_DIR) $(SOURCES) -o $(TARGET)
	@echo "[+] El Cienco compiled for Termux"

clean:
	rm -f $(TARGET)

install:
	cp $(TARGET) /data/data/com.termux/files/usr/bin/
	@echo "[+] El Cienco installed to Termux PATH"

run:
	./$(TARGET)

api:
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $(API_DIR)/server.c $(CORE_DIR)/*.c $(UTILS_DIR)/*.c -o $(API_TARGET)
	./$(API_TARGET)
