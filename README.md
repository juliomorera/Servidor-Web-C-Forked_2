Servidor-Web-C-Forked

Estudiantes: Julio Morera 207260848 Estudiantes: Dalia Moran 118850464

Curso: Sistemas Operativos Profesor: Bryan Mena Villalobos

Proyecto: Servidor web en C Forked:

Descripción:

El Servidor-Web-C-Forked es un servidor HTTP básico implementado en C que maneja solicitudes GET para archivos estáticos desde un directorio raíz especificado. El servidor incluye funcionalidades para manejar solicitudes HTTP, registrar logs de solicitudes, y permitir la terminación del servidor desde una interfaz de línea de comandos.

Requerimientos del Proyecto Validación de Argumentos del Puerto:

Verifica que el puerto proporcionado sea un número válido entre 1024 y 65535. Manejo de Solicitudes HTTP GET:

Sirve archivos estáticos desde el directorio serverroot. Responde con un error 404 si el archivo no se encuentra. Ignorar Otros Tipos de Solicitudes:

Responde con un error 405 si el método HTTP no es GET. Logs de las Solicitudes Recibidas:

Imprime en la consola la ruta del archivo solicitado para fines de depuración. Interfaz de Línea de Comandos con Capacidad de Terminación del Servidor:

Permite terminar el servidor mediante la tecla 'q'. Explicación del Código Validación de Argumentos El programa valida el argumento del puerto para asegurar que esté dentro del rango permitido (1024-65535). Si el puerto es inválido, se muestra un mensaje de error y el programa se termina.

Manejo de Solicitudes HTTP handle_request(int client_socket): Lee y analiza la solicitud HTTP del cliente. Si la solicitud es de tipo GET, intenta servir el archivo correspondiente desde el directorio serverroot. Responde con un error 404 si el archivo no se encuentra. Responde con un error 405 para métodos HTTP no permitidos. Logs de Solicitudes Se imprime la ruta del archivo solicitado en la consola para registrar la solicitud recibida y facilitar la depuración.

Interfaz de Línea de Comandos Proceso Hijo: Maneja las solicitudes HTTP y sirve los archivos. Proceso Padre: Espera la entrada del usuario. Cuando se presiona 'q', el proceso padre envía una señal para terminar el servidor. Manejo de Señales Se utiliza la señal SIGUSR1 para terminar el servidor cuando se presiona 'q'.

Cómo Probar el Servidor:

Compilar el Servidor Para compilar el servidor, ejecuta el siguiente comando en la terminal: gcc -o server_forked server_forked.c

Iniciar el Servidor Ejecuta el servidor con un puerto válido, por ejemplo: ./server_forked 8080

Hacer Solicitudes Utiliza un navegador web o una herramienta como curl para enviar solicitudes GET a la dirección del servidor. Ejemplo: http://localhost:8080/index.html 
http://localhost:8080/song.mp3 
http://localhost:8080/matrix.mp4 
http://localhost:8080/proyecto.pdf

Terminar el servidor: Presione la letra Q en la terminal o "CTRL + C"
