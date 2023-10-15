# Compiler and flags
CC = gcc
CFLAGS = -Wall

# Header files directory
HEADER_FILES_DIR = .

# Include directories
INCLUDES = -I$(HEADER_FILES_DIR)

# Source files
SRCS = servidor.c cliente.c

# Executable files
EXES = $(SRCS:.c=)

# # Build both executables
all: $(EXES)

# Build executables
$EXES: %.c
	$(CC) $(CFLAGS) $< -o $@

# Clean targets
clean:
	rm -f $(EXES)
