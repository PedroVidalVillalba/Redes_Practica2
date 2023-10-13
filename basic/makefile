# Opciones de compilación
CC = gcc -Wall

# Carpeta con las cabeceras
HEADER_FILES_DIR = .

# Opciones de compilación, indica donde están las cabeceras
INCLUDES = -I $(HEADER_FILES_DIR)

# Archivos .h
HEADERS = $(HEADER_FILES_DIR)/servidor.h $(HEADER_FILES_DIR)/cliente.h

# Archivos fuente
SRCS = servidor.c cliente.c

# Ficheros objeto
OBJS = $(SRCS:.c=.o)

# Fichero ejecutable de salida
OUT = $(SRCS:.c=)

# Genera el ejecutable, dependencia de los .o
$(OUT): $(OBJS)
	$(CC) -o $@ $<

# Genera los .o necesarios, dependencia de los .c y .h
%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(INCLUDES)

# Borra el ejecutable y todos los .o y de backup
cleanall: clean
	rm -f $(OUT)

# Borra los ficheros .o y los de backup
clean:
	rm -f *.o 
