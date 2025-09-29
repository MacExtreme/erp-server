#ifndef CLIENTE_CONTROLLER_H
#define CLIENTE_CONTROLLER_H

#include "cliente.h"
#include <string>

namespace ERP {
    
    class ClienteController {
    private:
        ClienteDAO& dao;
        
    public:
        ClienteController(ClienteDAO& cliente_dao) : dao(cliente_dao) {}
        
        // Listar todos los clientes
        std::string listar_todos() {
            auto clientes = dao.obtener_todos();
            
            std::string json = "{\"exito\":true,\"mensaje\":\"Clientes obtenidos exitosamente\",\"datos\":[";
            for (size_t i = 0; i < clientes.size(); i++) {
                json += clientes[i].to_json();
                if (i < clientes.size() - 1) json += ",";
            }
            json += "]}";
            
            return json;
        }
        
        // Obtener cliente por ID
        std::string obtener_por_id(int id) {
            auto cliente = dao.obtener_por_id(id);
            
            if (cliente.id == 0) {
                return "{\"exito\":false,\"mensaje\":\"Cliente no encontrado\",\"codigo_error\":404}";
            }
            
            return "{\"exito\":true,\"mensaje\":\"Cliente encontrado\",\"datos\":" + cliente.to_json() + "}";
        }
        
        // Crear nuevo cliente
        std::string crear(const std::string& cliente_json) {
            // Parseo simple del JSON (implementar parser más robusto después)
            Cliente cliente = parse_json(cliente_json);
            
            if (cliente.codigo.empty() || cliente.razon_social.empty()) {
                return "{\"exito\":false,\"mensaje\":\"Código y razón social son requeridos\",\"codigo_error\":400}";
            }
            
            if (dao.crear(cliente)) {
                return "{\"exito\":true,\"mensaje\":\"Cliente creado exitosamente\"}";
            } else {
                return "{\"exito\":false,\"mensaje\":\"Error al crear cliente\",\"codigo_error\":500}";
            }
        }
        
        // Eliminar cliente
        std::string eliminar(int id) {
            if (dao.eliminar(id)) {
                return "{\"exito\":true,\"mensaje\":\"Cliente eliminado exitosamente\"}";
            } else {
                return "{\"exito\":false,\"mensaje\":\"Error al eliminar cliente\",\"codigo_error\":500}";
            }
        }
        
    private:
        Cliente parse_json(const std::string& json) {
            Cliente cliente;
            // Implementación básica - mejorar después
            // Por ahora, asumimos formato simple: {"codigo":"CLI001","razon_social":"Empresa"...}
            size_t pos;
            
            if ((pos = json.find("\"codigo\":\"")) != std::string::npos) {
                size_t end = json.find("\"", pos + 10);
                cliente.codigo = json.substr(pos + 10, end - pos - 10);
            }
            
            if ((pos = json.find("\"razon_social\":\"")) != std::string::npos) {
                size_t end = json.find("\"", pos + 16);
                cliente.razon_social = json.substr(pos + 16, end - pos - 16);
            }
            
            // ... similar para otros campos
            
            return cliente;
        }
    };
    
} // namespace ERP

#endif