# Redes. Práctica 2

## Autores
- Carlos Cao López, <carlos.cao@rai.usc.es>
- Pedro Vidal Villalba, <pedro.vidal.villalba@rai.usc.es>

Proyecto disponible en [GitHub](https://github.com/PedroVidalVillalba/Redes_Practica2).

## Descripción

Esta práctica consiste en la creación de un programa servidor y un programa cliente en TCP para entender su funcionamiento.
Existen dos versiones de los programas:

- **Cliente/Servidor básicos**: en la carpeta `basic/`.
    - **Servidor**: `servidor`. Escucha conexiones de clientes TCP por un puerto especificado y les envía un mensaje.
    - **Cliente**: `cliente`. Se conecta a `servidor` en la IP y puerto especificados, recibe el mensaje y lo imprime.
- **Cliente/Servidor de mayúsculas**: en la carpeta `mayus/`.
    - **Servidor**: `servidormay`. Escucha conexiones de clientes TCP por un puerto especificado y pasa a mayúsculas las líneas de texto que le envía el cliente.
    - **Cliente**: `clientemay`. Se conecta a `servidormay` en la IP y puerto especificados, lee un archivo línea a línea y se lo envía al servidor para que lo pase a mayúsculas, para después escribir el contenido en mayúsculas en un nuevo archivo.
    
## Estructura del proyecto

El código se encuentra distribuido en tres carpetas diferentes:

- **`server`**: tiene el código común a las dos versiones, con la definición de las estructuras de datos y las funciones de creación, conexión y cerrado de servidor y cliente. Estas funcionalidades se encuentran, en `server` y `client`, respectivamente. Hay además algunos archivos más con funciones auxiliares que no imprescindibles, como obtener la IP (en `getip`) o funciones para que el servidor escriba en el log y macros para imprimir mensajes (en `loging`). Todos estos programas tienen sus correspondientes cabeceras `.h` e implementaciones `.c`.
- **`basic`**: tiene los programas de la versión básica: `servidor.c` y `cliente.c`.
- **`mayus`**: tiene los programas de la versión de mayúsculas: `servidormay.c` y `clientemay.c`.

En el nivel superior se encuentran estas carpetas, este `README.md`, un archivo `Makefile` y el resto de entregables, que son el informe de la práctica y una captura de pantalla en la que se ve que el cliente y servidor de mayúsculas pueden ejecutarse en ordenadores distintos.

## Creación de los ejecutables

Para compilar los ejecutables, ejecutar el `Makefile` desde el directorio base:
```
make
```
Esta opción compila todo el proyecto. Las siguientes opciones también están disponibles:
```
make basic
make mayus
make server
make client
```
Compilan solo los el servidor y cliente básicos; el servidor y cliente de mayúsculas; el servidor básico y el servidor de mayúsculas; o el cliente básico y el cliente de mayúsculas, respectivamente.

Para limpiar, ejecutar:
```
make cleanobj
make clean
```
para limpiar los ficheros objeto resultado de la compilación o estos más los ejecutables, respectivamente.
 
