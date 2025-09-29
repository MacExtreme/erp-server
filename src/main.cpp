#include <iostream>
#include <string>
#include <vector>
#include "../include/miniserver.h"

// JSON simple para Windows
namespace simple_json {
    std::string escape(const std::string& str) {
        std::string result;
        for (char c : str) {
            if (c == '"') result += "\\\"";
            else if (c == '\\') result += "\\\\";
            else result += c;
        }
        return result;
    }
    
    std::string create_object(const std::vector<std::pair<std::string, std::string>>& pairs) {
        std::string result = "{";
        for (size_t i = 0; i < pairs.size(); ++i) {
            result += "\"" + pairs[i].first + "\":\"" + escape(pairs[i].second) + "\"";
            if (i < pairs.size() - 1) result += ",";
        }
        result += "}";
        return result;
    }
    
    std::string create_array(const std::vector<std::string>& items) {
        std::string result = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            result += items[i];
            if (i < items.size() - 1) result += ",";
        }
        result += "]";
        return result;
    }
}

namespace ERP {
    struct Cliente {
        int id = 0;
        std::string codigo;
        std::string razon_social;
        std::string ruc;
        std::string direccion;
        std::string telefono;
        std::string email;
        bool activo = true;
        
        std::string to_json() const {
            return simple_json::create_object({
                {"id", std::to_string(id)},
                {"codigo", codigo},
                {"razon_social", razon_social},
                {"ruc", ruc},
                {"direccion", direccion},
                {"telefono", telefono},
                {"email", email},
                {"activo", activo ? "true" : "false"}
            });
        }
    };
    
    class ClienteManager {
    private:
        std::vector<Cliente> clientes;
        int proximo_id = 1;
        
    public:
        ClienteManager() {
            // Datos de prueba
            agregar_cliente_ejemplo("CLI001", "Empresa Ejemplo SAC", "20123456789");
            agregar_cliente_ejemplo("CLI002", "Comercializadora Los Andes EIRL", "20987654321");
            std::cout << "Datos de prueba inicializados: " << clientes.size() << " clientes" << std::endl;
        }
        
        void agregar_cliente_ejemplo(const std::string& codigo, const std::string& razon_social, const std::string& ruc) {
            Cliente c;
            c.id = proximo_id++;
            c.codigo = codigo;
            c.razon_social = razon_social;
            c.ruc = ruc;
            c.direccion = "Direccion de " + razon_social;
            c.telefono = "01-1234567";
            c.email = "info@" + codigo + ".com";
            clientes.push_back(c);
        }
        
        std::string listar_todos() {
            std::vector<std::string> clientes_json;
            for (const auto& cliente : clientes) {
                if (cliente.activo) {
                    clientes_json.push_back(cliente.to_json());
                }
            }
            
            return simple_json::create_object({
                {"exito", "true"},
                {"mensaje", "Clientes obtenidos exitosamente"},
                {"datos", simple_json::create_array(clientes_json)}
            });
        }
        
        std::string obtener_por_id(int id) {
            for (const auto& cliente : clientes) {
                if (cliente.id == id && cliente.activo) {
                    return simple_json::create_object({
                        {"exito", "true"},
                        {"mensaje", "Cliente encontrado"},
                        {"datos", cliente.to_json()}
                    });
                }
            }
            
            return simple_json::create_object({
                {"exito", "false"},
                {"mensaje", "Cliente no encontrado"}
            });
        }
        
        std::string eliminar(int id) {
            for (auto& cliente : clientes) {
                if (cliente.id == id) {
                    cliente.activo = false;
                    return simple_json::create_object({
                        {"exito", "true"},
                        {"mensaje", "Cliente eliminado exitosamente"}
                    });
                }
            }
            
            return simple_json::create_object({
                {"exito", "false"},
                {"mensaje", "Cliente no encontrado"}
            });
        }
    };
}

int main() {
    std::cout << "===========================================" << std::endl;
    std::cout << "ERP Sistema - Servidor Windows" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    ERP::ClienteManager manager;
    MiniServer server(8080);
    
    // Configurar rutas
    server.get("/api/clientes", [&](const std::string& method, const std::string& body) {
        std::cout << "GET /api/clientes" << std::endl;
        return manager.listar_todos();
    });
    
    server.get("/api/clientes/", [&](const std::string& method, const std::string& body) {
        try {
            size_t last_slash = method.find_last_of('/');
            if (last_slash != std::string::npos) {
                std::string id_str = method.substr(last_slash + 1);
                int id = std::stoi(id_str);
                std::cout << "GET /api/clientes/" << id << std::endl;
                return manager.obtener_por_id(id);
            }
        } catch (...) {}
        
        return simple_json::create_object({{"error", "ID invalido"}});
    });
    
    server.post("/api/clientes", [&](const std::string& method, const std::string& body) {
        std::cout << "POST /api/clientes - Body: " << (body.empty() ? "vacio" : "con datos") << std::endl;
        // Simular creación exitosa
        return simple_json::create_object({
            {"exito", "true"},
            {"mensaje", "Cliente creado exitosamente"},
            {"id", "999"}
        });
    });
    
    server.del("/api/clientes/", [&](const std::string& method, const std::string& body) {
        try {
            size_t last_slash = method.find_last_of('/');
            if (last_slash != std::string::npos) {
                std::string id_str = method.substr(last_slash + 1);
                int id = std::stoi(id_str);
                std::cout << "DELETE /api/clientes/" << id << std::endl;
                return manager.eliminar(id);
            }
        } catch (...) {}
        
        return simple_json::create_object({{"error", "ID invalido"}});
    });
    
    // Ruta de estado
    server.get("/api/status", [](const std::string& method, const std::string& body) {
        return simple_json::create_object({
            {"sistema", "ERP"},
            {"version", "1.0.0"},
            {"estado", "activo"},
            {"plataforma", "Windows"}
        });
    });
    
    // Ruta raíz
    server.get("/", [](const std::string& method, const std::string& body) {
        std::string html = R"(<html><head><title>ERP Sistema</title></head>
<body><h1>ERP Sistema Funcionando</h1>
<p>API disponible en:</p>
<ul>
<li><a href="/api/clientes">/api/clientes</a> - Listar clientes</li>
<li><a href="/api/status">/api/status</a> - Estado del sistema</li>
</ul>
</body></html>)";
        return html;
    });
    
    std::cout << "Iniciando servidor..." << std::endl;
    std::cout << "URL: http://localhost:8080" << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  GET  /api/clientes    - Listar clientes" << std::endl;
    std::cout << "  GET  /api/clientes/1  - Obtener cliente por ID" << std::endl;
    std::cout << "  POST /api/clientes    - Crear cliente" << std::endl;
    std::cout << "  DEL  /api/clientes/1  - Eliminar cliente" << std::endl;
    std::cout << "  GET  /api/status      - Estado del sistema" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    if (!server.start()) {
        std::cerr << "Error: No se pudo iniciar el servidor" << std::endl;
        return 1;
    }
    
    return 0;
}