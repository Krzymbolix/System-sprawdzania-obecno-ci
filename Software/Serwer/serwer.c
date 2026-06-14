#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void handle_client(int client_socket) {
    char buffer[2048] = {0};
    read(client_socket, buffer, sizeof(buffer));
    printf("Odebrano żądanie:\n%s\n", buffer);

    // Prosta analiza: sprawdź czy to GET /get-list/
    char method[8], path[1024];
    sscanf(buffer, "%s %s", method, path);

    if (strncmp(method, "GET", 3) != 0) {
        // Obsługujemy tylko GET
        const char* response =
            "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }

    if (strncmp(path, "/get-list/", 10) == 0) {
        // Wyciągnij ID z URL
        char* id = path + 10;

        // Możesz tu dodać logikę np. połączenia z bazą
        // Dla uproszczenia zwróćmy statyczną odpowiedź
        char body[512];
        snprintf(body, sizeof(body),
                 "{\"list1\": [[\"%s\", \"12345\", \"Jan\", \"Kowalski\"]]}", id);

        char response[1024];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n"
                 "%s",
                 strlen(body), body);

        send(client_socket, response, strlen(response), 0);
    } else {
        const char* response =
            "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    printf("Serwer działa na porcie %d...\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_socket >= 0) {
            handle_client(client_socket);
        }
    }

    return 0;
}
