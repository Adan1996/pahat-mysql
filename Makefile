# ==============================================================================
# Cross-Platform Makefile untuk MySQL Plugin Driver (Windows, Linux, & macOS)
# ==============================================================================

# 1. Deteksi OS Otomatis
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
else
    DETECTED_OS := $(shell uname -s)
endif

# 2. Konfigurasi Compiler & Nama Output
CC := gcc
CFLAGS := -Wall -Wextra -O2 -fPIC
TARGET_NAME := mysql_pahat
SRC := src/mysql_driver.c
MODULES_DIR := modules

# 3. Setting Konfigurasi Spesifik Per OS
ifeq ($(DETECTED_OS),Windows)
    # --- KONFIGURASI WINDOWS ---
    TARGET := $(MODULES_DIR)/$(TARGET_NAME).dll
    
    # Path SDK MySQL di Windows
    MYSQL_DEV_DIR ?= C:/mysql_dev
    
    INCLUDES := -I"$(MYSQL_DEV_DIR)/include"
    LIBS := "$(MYSQL_DEV_DIR)/lib/libmysql.dll"
    
    MKDIR := if not exist $(MODULES_DIR) mkdir $(MODULES_DIR)
    RM := del /Q /F 2>NUL || true
else
    # --- KONFIGURASI LINUX / MACOS ---
    ifeq ($(DETECTED_OS),Darwin)
        TARGET := $(MODULES_DIR)/$(TARGET_NAME).dylib
    else
        TARGET := $(MODULES_DIR)/$(TARGET_NAME).so
    endif
    
    # Deteksi Flag Kompilasi (pkg-config -> mysql_config -> mariadb_config -> default fallback)
    MYSQL_CFLAGS := $(shell pkg-config --cflags mariadb 2>/dev/null || \
                            pkg-config --cflags mysqlclient 2>/dev/null || \
                            mysql_config --cflags 2>/dev/null || \
                            mariadb_config --cflags 2>/dev/null || \
                            echo "-I/usr/include/mysql")

    MYSQL_LIBS := $(shell pkg-config --libs mariadb 2>/dev/null || \
                          pkg-config --libs mysqlclient 2>/dev/null || \
                          mysql_config --libs 2>/dev/null || \
                          mariadb_config --libs 2>/dev/null || \
                          echo "-lmysqlclient")
    
    INCLUDES := $(MYSQL_CFLAGS)
    LIBS := $(MYSQL_LIBS)
    
    MKDIR := mkdir -p $(MODULES_DIR)
    RM := rm -f
endif

# ==============================================================================
# TARGET BUILD
# ==============================================================================

.PHONY: all clean info

all: info $(TARGET)

$(TARGET): $(SRC)
	$(MKDIR)
	$(CC) -shared $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS)

info:
	@echo "----------------------------------------"
	@echo "Building for OS : $(DETECTED_OS)"
	@echo "Target Output   : $(TARGET)"
	@echo "INCLUDES        : $(INCLUDES)"
	@echo "LIBS            : $(LIBS)"
	@echo "----------------------------------------"

clean:
	-$(RM) $(TARGET)