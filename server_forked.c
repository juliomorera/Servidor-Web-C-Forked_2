#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>  // Para estructuras y constantes relacionadas con sockets
#include <netinet/in.h> // Para estructuras y constantes relacionadas con la red

#define BUFFER_SIZE 1024          // Tamaño del buffer para leer solicitudes
#define SERVER_ROOT "serverroot"  // Directorio donde se almacenan los archivos del servidor

/**
 * @brief Obtiene el tipo MIME basado en la extensión del archivo.
 * 
 * Esta función devuelve el tipo MIME adecuado para la extensión del archivo proporcionado.
 * Si la extensión del archivo no es reconocida, devuelve "application/octet-stream" como tipo MIME por defecto.
 * 
 * @param path Ruta del archivo solicitado.
 * @return Tipo MIME del archivo.
 */
const char* get_mime_type(const char* path) {
    // Verificar extensiones comunes y retornar el tipo MIME correspondiente
    if (strstr(path, ".html") || strstr(path, ".htm")) return "text/html";
    if (strstr(path, ".css")) return "text/css";
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".jpg") || strstr(path, ".jpeg")) return "image/jpeg";
    if (strstr(path, ".png")) return "image/png";
    if (strstr(path, ".gif")) return "image/gif";
    if (strstr(path, ".pdf")) return "application/pdf";
    if (strstr(path, ".mp3")) return "audio/mpeg";
    if (strstr(path, ".mp4")) return "video/mp4";
    return "application/octet-stream"; // Tipo MIME por defecto para archivos desconocidos
}

/**
 * @brief Maneja la solicitud HTTP del cliente.
 * 
 * Esta función lee la solicitud del cliente, maneja solicitudes GET, y envía archivos desde el directorio 
 * especificado por `SERVER_ROOT`. Si el archivo solicitado no se encuentra, se envía un mensaje de error 404.
 * Si la solicitud no es un GET, se envía un mensaje de error 405. También imprime la ruta del archivo solicitado 
 * para fines de depuración.
 * 
 * @param client_socket Descriptor del socket del cliente.
 */
void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];       // Buffer para leer la solicitud del cliente
    char file_path[BUFFER_SIZE];     // Ruta del archivo solicitado
    int file_fd;                     // Descriptor de archivo
    ssize_t bytes_read;             // Número de bytes leídos del archivo
    struct stat file_stat;           // Información sobre el archivo
    const char *mime_type;           // Tipo MIME del archivo

    // Leer la solicitud del cliente
    read(client_socket, buffer, BUFFER_SIZE);

    // Verificar si la solicitud es un GET
    if (strncmp(buffer, "GET ", 4) == 0) {
        // Extraer la ruta del archivo de la solicitud GET
        char *request_path = buffer + 4;      // Obtener la parte de la solicitud después de "GET "
        char *end_path = strchr(request_path, ' '); // Encontrar el primer espacio
        if (end_path) *end_path = '\0'; // Terminar la cadena en el primer espacio

        // Construir la ruta completa del archivo solicitada
        snprintf(file_path, sizeof(file_path), "%s%s", SERVER_ROOT, request_path);

        // Imprimir en la consola la ruta del archivo solicitado para fines de depuración
        printf("Requested file: %s\n", file_path);

        // Abrir el archivo solicitado
        file_fd = open(file_path, O_RDONLY);
        if (file_fd == -1) {
            // Enviar respuesta de error 404 si el archivo no se encuentra
            write(client_socket, "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nFile not found", 60);
        } else {
            // Obtener el tamaño del archivo
            fstat(file_fd, &file_stat);

            // Obtener el tipo MIME del archivo
            mime_type = get_mime_type(file_path);

            // Enviar respuesta HTTP con el tipo MIME y el tamaño del archivo
            dprintf(client_socket, "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %ld\n\n", mime_type, file_stat.st_size);

            // Enviar el contenido del archivo al cliente
            while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
                write(client_socket, buffer, bytes_read);
            }

            close(file_fd); // Cerrar el descriptor de archivo después de enviar el contenido
        }
    } else {
        // Enviar respuesta de error 405 para solicitudes que no sean GET
        write(client_socket, "HTTP/1.1 405 Method Not Allowed\nContent-Type: text/plain\n\nMethod not allowed", 76);
    }

    close(client_socket); // Cerrar el socket del cliente
}

/**
 * @brief Manejador de señales para terminar el servidor.
 * 
 * Esta función maneja la señal SIGUSR1 para terminar el proceso actual. Es utilizada para finalizar 
 * el servidor cuando se recibe la señal.
 * 
 * @param signal Señal recibida.
 */
void signal_handler(int signal) {
    if (signal == SIGUSR1) {
        _exit(0); // Terminar el proceso actual
    }
}

/**
 * @brief Espera a que el usuario presione 'q' y luego termina el servidor.
 * 
 * Esta función permite al usuario terminar el servidor presionando la tecla 'q'. Envia una señal SIGUSR1
 * a todos los procesos del grupo para finalizar el servidor.
 */
void key_listener() {
    int ch = 0;
    printf("Press 'q' to kill all processes and terminate server\n");

    // Esperar a que se presione la tecla 'q'
    while (ch != 'q') {
        ch = getchar();
    }

    // Enviar señal SIGUSR1 para terminar todos los procesos en el grupo
    kill(0, SIGUSR1);
}

int main(int argc, char *argv[]) {
    int server_socket, client_socket, port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pid_t pid;

    // Validar argumentos de línea de comandos
    // El puerto debe ser pasado como argumento
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Convertir el puerto de cadena a entero y validar
    port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number. Must be between 1 and 65535.\n");
        exit(EXIT_FAILURE);
    }

    // Crear socket del servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Aceptar conexiones de cualquier dirección IP
    server_addr.sin_port = htons(port);         // Convertir el número de puerto a red

    // Enlazar el socket con la dirección del servidor
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_socket, 10) < 0) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Configurar el manejador de señales
    signal(SIGUSR1, signal_handler);

    // Crear un proceso hijo para manejar las solicitudes
    if ((pid = fork()) == 0) {
        // Proceso hijo: Maneja las solicitudes del cliente
        while (1) {
            // Aceptar una conexión del cliente
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_socket < 0) {
                perror("accept");
                continue;
            }

            // Crear un nuevo proceso para manejar la solicitud del cliente
            if (fork() == 0) {
                // Proceso hijo: Maneja la solicitud del cliente
                close(server_socket); // Cerrar el socket del servidor en el proceso hijo
                handle_request(client_socket); // Manejar la solicitud del cliente
                exit(0); // Terminar el proceso hijo después de manejar la solicitud
            } else {
                // Proceso padre: Cierra el socket del cliente
                close(client_socket);
            }
        }
    } else {
        // Proceso padre: Espera la tecla 'q' para terminar el servidor
        key_listener(); // Esperar a que el usuario presione 'q'
        close(server_socket); // Cerrar el socket del servidor
        // Esperar a que el proceso hijo termine
        wait(NULL);
    }

    return 0;
}
