// ====================================================================
// HTTPLIB.H - VERSIÓN MÍNIMA PARA WINDOWS
// Alternativa simple cuando no se puede descargar el original
// ====================================================================

#ifndef HTTPLIB_H
#define HTTPLIB_H

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <iostream>

namespace httplib {

    // ================================================================
    // ESTRUCTURAS BÁSICAS
    // ================================================================
    
    struct Request {
        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
        std::map<std::string, std::string> params;
        std::smatch matches;
        
        std::string get_param_value(const std::string& key) const {
            auto it = params.find(key);
            return it != params.end() ? it->second : "";
        }
        
        std::string get_header_value(const std::string& key) const {
            auto it = headers.find(key);
            return it != headers.end() ? it->second : "";
        }
    };

    struct Response {
        int status = 200;
        std::map<std::string, std::string> headers;
        std::string body;
        
        void set_content(const std::string& content, const std::string& content_type) {
            body = content;
            headers["Content-Type"] = content_type;
            headers["Content-Length"] = std::to_string(content.length());
        }
        
        void set_header(const std::string& key, const std::string& value) {
            headers[key] = value;
        }
    };

    // ================================================================
    // TIPOS DE HANDLERS
    // ================================================================
    
    using Handler = std::function<void(const Request&, Response&)>;
    using HandlerResponse = enum { Handled, Unhandled };
    using PreRoutingHandler = std::function<HandlerResponse(const Request&, Response&)>;

    // ================================================================
    // SERVIDOR HTTP BÁSICO
    // ================================================================
    
    class Server {
    private:
        struct Route {
            std::string method;
            std::regex pattern;
            Handler handler;
        };
        
        std::vector<Route> routes_;
        PreRoutingHandler pre_routing_handler_;
        bool is_running_ = false;
        
#ifdef _WIN32
        SOCKET server_socket_ = INVALID_SOCKET;
#else
        int server_socket_ = -1;
#endif
        
        void init_winsock() {
#ifdef _WIN32
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        }
        
        void cleanup_winsock() {
#ifdef _WIN32
            WSACleanup();
#endif
        }
        
        std::map<std::string, std::string> parse_query_string(const std::string& query) {
            std::map<std::string, std::string> params;
            std::istringstream iss(query);
            std::string pair;
            
            while (std::getline(iss, pair, '&')) {
                auto pos = pair.find('=');
                if (pos != std::string::npos) {
                    std::string key = pair.substr(0, pos);
                    std::string value = pair.substr(pos + 1);
                    params[key] = value;
                }
            }
            
            return params;
        }
        
        Request parse_request(const std::string& raw_request) {
            Request req;
            std::istringstream iss(raw_request);
            std::string line;
            
            // Parse request line
            if (std::getline(iss, line)) {
                std::istringstream request_line(line);
                std::string path_and_query;
                request_line >> req.method >> path_and_query;
                
                // Separate path and query
                auto query_pos = path_and_query.find('?');
                if (query_pos != std::string::npos) {
                    req.path = path_and_query.substr(0, query_pos);
                    std::string query = path_and_query.substr(query_pos + 1);
                    req.params = parse_query_string(query);
                } else {
                    req.path = path_and_query;
                }
            }
            
            // Parse headers
            while (std::getline(iss, line) && line != "\r") {
                auto pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    // Trim spaces
                    value.erase(0, value.find_first_not_of(" \t\r\n"));
                    value.erase(value.find_last_not_of(" \t\r\n") + 1);
                    req.headers[key] = value;
                }
            }
            
            // Parse body
            std::ostringstream body_stream;
            body_stream << iss.rdbuf();
            req.body = body_stream.str();
            
            return req;
        }
        
        std::string generate_response(const Response& res) {
            std::ostringstream oss;
            oss << "HTTP/1.1 " << res.status << " OK\r\n";
            
            for (const auto& header : res.headers) {
                oss << header.first << ": " << header.second << "\r\n";
            }
            
            oss << "\r\n" << res.body;
            return oss.str();
        }
        
        void handle_client(int client_socket) {
            char buffer[8192] = {0};
            
#ifdef _WIN32
            int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
#else
            ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);
#endif
            
            if (bytes_received > 0) {
                std::string raw_request(buffer, bytes_received);
                Request req = parse_request(raw_request);
                Response res;
                
                // Pre-routing handler
                if (pre_routing_handler_) {
                    if (pre_routing_handler_(req, res) == HandlerResponse::Handled) {
                        std::string response = generate_response(res);
#ifdef _WIN32
                        send(client_socket, response.c_str(), (int)response.length(), 0);
                        closesocket(client_socket);
#else
                        write(client_socket, response.c_str(), response.length());
                        close(client_socket);
#endif
                        return;
                    }
                }
                
                // Find matching route
                bool handled = false;
                for (const auto& route : routes_) {
                    if (route.method == req.method || route.method == "*") {
                        std::smatch matches;
                        if (std::regex_match(req.path, matches, route.pattern)) {
                            req.matches = matches;
                            route.handler(req, res);
                            handled = true;
                            break;
                        }
                    }
                }
                
                if (!handled) {
                    res.status = 404;
                    res.set_content("Not Found", "text/plain");
                }
                
                std::string response = generate_response(res);
#ifdef _WIN32
                send(client_socket, response.c_str(), (int)response.length(), 0);
                closesocket(client_socket);
#else
                write(client_socket, response.c_str(), response.length());
                close(client_socket);
#endif
            }
        }
        
    public:
        Server() {
            init_winsock();
        }
        
        ~Server() {
            cleanup_winsock();
        }
        
        void Get(const std::string& pattern, Handler handler) {
            routes_.push_back({"GET", std::regex(pattern), handler});
        }
        
        void Post(const std::string& pattern, Handler handler) {
            routes_.push_back({"POST", std::regex(pattern), handler});
        }
        
        void Put(const std::string& pattern, Handler handler) {
            routes_.push_back({"PUT", std::regex(pattern), handler});
        }
        
        void Delete(const std::string& pattern, Handler handler) {
            routes_.push_back({"DELETE", std::regex(pattern), handler});
        }
        
        void set_pre_routing_handler(PreRoutingHandler handler) {
            pre_routing_handler_ = handler;
        }
        
        bool listen(const std::string& host, int port) {
            // Create socket
#ifdef _WIN32
            server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
            if (server_socket_ == INVALID_SOCKET) {
                std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
                return false;
            }
#else
            server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
            if (server_socket_ < 0) {
                std::cerr << "Error creating socket" << std::endl;
                return false;
            }
#endif
            
            // Set socket options
            int opt = 1;
#ifdef _WIN32
            setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
            setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
            
            // Bind socket
            struct sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(port);
            
            if (bind(server_socket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
                std::cerr << "Error binding socket to port " << port << std::endl;
#ifdef _WIN32
                closesocket(server_socket_);
#else
                close(server_socket_);
#endif
                return false;
            }
            
            // Listen for connections
            if (listen(server_socket_, 10) < 0) {
                std::cerr << "Error listening on socket" << std::endl;
#ifdef _WIN32
                closesocket(server_socket_);
#else
                close(server_socket_);
#endif
                return false;
            }
            
            std::cout << "Server listening on " << host << ":" << port << std::endl;
            is_running_ = true;
            
            // Accept connections
            while (is_running_) {
                struct sockaddr_in client_address;
#ifdef _WIN32
                int client_len = sizeof(client_address);
                SOCKET client_socket = accept(server_socket_, (struct sockaddr*)&client_address, &client_len);
                if (client_socket != INVALID_SOCKET) {
                    std::thread client_thread(&Server::handle_client, this, (int)client_socket);
                    client_thread.detach();
                }
#else
                socklen_t client_len = sizeof(client_address);
                int client_socket = accept(server_socket_, (struct sockaddr*)&client_address, &client_len);
                if (client_socket >= 0) {
                    std::thread client_thread(&Server::handle_client, this, client_socket);
                    client_thread.detach();
                }
#endif
            }
            
            return true;
        }
        
        void stop() {
            is_running_ = false;
#ifdef _WIN32
            closesocket(server_socket_);
#else
            close(server_socket_);
#endif
        }
    };

} // namespace httplib

#endif // HTTPLIB_H
