#include <iostream>
#include <string>

#include "database.h"
#include "cliente.h"
#include "cliente_controller.h"

// Incluir httplib o tu MiniServer
#ifdef USE_HTTPLIB
    #include "httplib.h"
#else
    #include "miniserver.h"
#endif

using namespace std::string_literals; // AGREGA ESTA LINEA

int main() {
    std::cout << " ERP Sistema - Modulo Clientes con PostgreSQL" << std::endl;
    std::cout << "================================================" << std::endl;
    
    try {
        // Configuracion de conexion PostgreSQL
        std::string conninfo = "host=localhost port=5432 dbname=erp_db user=erp_user password=erp_pass";
        
        // Crear base de datos y tablas
        ERP::Database db(conninfo);
        db.initialize_tables();
        
        // Crear DAO y Controller
        ERP::ClienteDAO cliente_dao(db);
        ERP::ClienteController cliente_controller(cliente_dao);
        
        // Configurar servidor HTTP
        #ifdef USE_HTTPLIB
            httplib::Server server;
            
            server.Get("/api/clientes", [&](const httplib::Request& req, httplib::Response& res) {
                std::cout << "GET /api/clientes" << std::endl;
                res.set_content(cliente_controller.listar_todos(), "application/json");
            });
            
            server.Get("/api/clientes/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
                int id = std::stoi(req.matches[1]);
                std::cout << "GET /api/clientes/" << id << std::endl;
                res.set_content(cliente_controller.obtener_por_id(id), "application/json");
            });
            
            server.Post("/api/clientes", [&](const httplib::Request& req, httplib::Response& res) {
                std::cout << "POST /api/clientes" << std::endl;
                res.set_content(cliente_controller.crear(req.body), "application/json");
            });
            
            server.Delete("/api/clientes/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
                int id = std::stoi(req.matches[1]);
                std::cout << "DELETE /api/clientes/" << id << std::endl;
                res.set_content(cliente_controller.eliminar(id), "application/json");
            });
            
            std::cout << "Servidor iniciado en http://localhost:8080" << std::endl;
            server.listen("localhost", 8080);
            
        #else
            MiniServer server(8080);
            
            server.get("/api/clientes", [&](const std::string& method, const std::string& body) -> std::string {
                std::cout << "GET /api/clientes" << std::endl;
                return cliente_controller.listar_todos();
            });
            
            server.get("/api/clientes/", [&](const std::string& method, const std::string& body) -> std::string {
                try {
                    size_t last_slash = method.find_last_of('/');
                    if (last_slash != std::string::npos) {
                        int id = std::stoi(method.substr(last_slash + 1));
                        std::cout << "GET /api/clientes/" << id << std::endl;
                        return cliente_controller.obtener_por_id(id);
                    }
                } catch (...) {}
                return "{\"error\":\"ID invalido\"}"s; // Usa "s" literal
            });
            
            server.post("/api/clientes", [&](const std::string& method, const std::string& body) -> std::string {
                std::cout << "POST /api/clientes" << std::endl;
                return cliente_controller.crear(body);
            });
            
            server.del("/api/clientes/", [&](const std::string& method, const std::string& body) -> std::string {
                try {
                    size_t last_slash = method.find_last_of('/');
                    if (last_slash != std::string::npos) {
                        int id = std::stoi(method.substr(last_slash + 1));
                        std::cout << "DELETE /api/clientes/" << id << std::endl;
                        return cliente_controller.eliminar(id);
                    }
                } catch (...) {}
                return "{\"error\":\"ID invalido\"}"s; // Usa "s" literal
            });
            
            std::cout << "Servidor iniciado en http://localhost:8080" << std::endl;
            server.start();
        #endif
        
    } catch (const std::exception& e) {
        std::cerr << " Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}