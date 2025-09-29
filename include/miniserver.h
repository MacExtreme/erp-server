#ifndef MINI_SERVER_H
#define MINI_SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

class MiniServer {
private:
    SOCKET server_socket;
    int port;
    bool running;
    std::map<std::string, std::function<std::string(const std::string&, const std::string&)>> routes;

    void handle_client(SOCKET client_socket) {
        char buffer[4096];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::string request(buffer);
            
            // Parsear método y ruta
            std::istringstream iss(request);
            std::string method, path, version;
            iss >> method >> path >> version;
            
            // Buscar cuerpo del request
            std::string body;
            size_t body_pos = request.find("\r\n\r\n");
            if (body_pos != std::string::npos) {
                body = request.substr(body_pos + 4);
            }
            
            std::string response_content = "{\"error\":\"Ruta no encontrada\"}";
            std::string content_type = "application/json";
            
            // Buscar ruta
            for (const auto& route : routes) {
                if (path.find(route.first) == 0) {
                    response_content = route.second(method, body);
                    break;
                }
            }
            
            // Construir respuesta HTTP
            std::string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: " + content_type + "\r\n";
            response += "Access-Control-Allow-Origin: *\r\n";
            response += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
            response += "Access-Control-Allow-Headers: Content-Type\r\n";
            response += "Content-Length: " + std::to_string(response_content.length()) + "\r\n";
            response += "Connection: close\r\n\r\n";
            response += response_content;
            
            send(client_socket, response.c_str(), response.length(), 0);
        }
        
        closesocket(client_socket);
    }

public:
    MiniServer(int port = 8080) : port(port), running(false), server_socket(INVALID_SOCKET) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Error inicializando Winsock" << std::endl;
        }
    }
    
    ~MiniServer() {
        stop();
        WSACleanup();
    }
    
    void get(const std::string& path, std::function<std::string(const std::string&, const std::string&)> handler) {
        routes[path] = handler;
    }
    
    void post(const std::string& path, std::function<std::string(const std::string&, const std::string&)> handler) {
        routes[path] = handler;
    }
    
    void del(const std::string& path, std::function<std::string(const std::string&, const std::string&)> handler) {
        routes[path] = handler;
    }
    
    bool start() {
        server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket == INVALID_SOCKET) {
            std::cerr << "Error creando socket: " << WSAGetLastError() << std::endl;
            return false;
        }
        
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "Error en bind: " << WSAGetLastError() << std::endl;
            closesocket(server_socket);
            return false;
        }
        
        if (listen(server_socket, 5) == SOCKET_ERROR) {
            std::cerr << "Error en listen: " << WSAGetLastError() << std::endl;
            closesocket(server_socket);
            return false;
        }
        
        running = true;
        std::cout << "Servidor iniciado en http://localhost:" << port << std::endl;
        std::cout << "Presiona Ctrl+C para detener el servidor" << std::endl;
        
        // Bucle principal SIN hilos - maneja una conexión a la vez
        while (running) {
            sockaddr_in client_addr;
            int client_addr_size = sizeof(client_addr);
            
            // Usar select para no bloquear indefinidamente
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(server_socket, &readfds);
            
            timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int activity = select(0, &readfds, NULL, NULL, &timeout);
            
            if (activity == SOCKET_ERROR) {
                std::cerr << "Error en select: " << WSAGetLastError() << std::endl;
                break;
            }
            
            if (activity > 0 && FD_ISSET(server_socket, &readfds)) {
                SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
                
                if (client_socket != INVALID_SOCKET) {
                    // Manejar cliente en el mismo hilo (sin crear nuevo hilo)
                    handle_client(client_socket);
                }
            }
        }
        
        return true;
    }
    
    void stop() {
        running = false;
        if (server_socket != INVALID_SOCKET) {
            closesocket(server_socket);
            server_socket = INVALID_SOCKET;
        }
    }
};

#endif
