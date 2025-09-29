// ====================================================================
// MAIN.CPP SIMPLIFICADO - PASO A PASO
// Version minima para probar la arquitectura
// ====================================================================

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include "../include/httplib.h"
#include "../include/json.hpp"

// Por ahora, simulamos la base de datos con un vector
// En el siguiente paso conectaremos PostgreSQL
namespace ERP {
    
    // ================================================================
    // MODELO BASICO (Paso 5)
    // ================================================================
    struct Cliente {
        int id = 0;
        std::string codigo;
        std::string razon_social;
        std::string ruc;
        std::string direccion;
        std::string telefono;
        std::string email;
        bool activo = true;
        
        nlohmann::json to_json() const {
            return nlohmann::json{
                {"id", id},
                {"codigo", codigo},
                {"razon_social", razon_social},
                {"ruc", ruc},
                {"direccion", direccion},
                {"telefono", telefono},
                {"email", email},
                {"activo", activo}
            };
        }
        
        static Cliente from_json(const nlohmann::json& j) {
            Cliente c;
            if (j.contains("id")) c.id = j["id"];
            if (j.contains("codigo")) c.codigo = j["codigo"];
            if (j.contains("razon_social")) c.razon_social = j["razon_social"];
            if (j.contains("ruc")) c.ruc = j["ruc"];
            if (j.contains("direccion")) c.direccion = j["direccion"];
            if (j.contains("telefono")) c.telefono = j["telefono"];
            if (j.contains("email")) c.email = j["email"];
            if (j.contains("activo")) c.activo = j["activo"];
            return c;
        }
    };
    
    // ================================================================
    // SIMULADOR DE BASE DE DATOS (Paso 5 - Temporal)
    // ================================================================
    class ClienteSimulado {
    private:
        static std::vector<Cliente> clientes;
        static int proximo_id;
        
    public:
        static std::string listar_todos() {
            nlohmann::json respuesta;
            respuesta["exito"] = true;
            respuesta["mensaje"] = "Clientes obtenidos exitosamente";
            respuesta["datos"] = nlohmann::json::array();
            
            for (const auto& cliente : clientes) {
                respuesta["datos"].push_back(cliente.to_json());
            }
            
            return respuesta.dump();
        }
        
        static std::string obtener_por_id(int id) {
            nlohmann::json respuesta;
            
            for (const auto& cliente : clientes) {
                if (cliente.id == id) {
                    respuesta["exito"] = true;
                    respuesta["mensaje"] = "Cliente encontrado";
                    respuesta["datos"] = cliente.to_json();
                    return respuesta.dump();
                }
            }
            
            respuesta["exito"] = false;
            respuesta["mensaje"] = "Cliente no encontrado";
            respuesta["codigo_error"] = 404;
            return respuesta.dump();
        }
        
        static std::string crear(const std::string& cliente_json) {
            nlohmann::json respuesta;
            
            try {
                auto json_data = nlohmann::json::parse(cliente_json);
                Cliente cliente = Cliente::from_json(json_data);
                
                // Validaciones basicas
                if (cliente.codigo.empty()) {
                    respuesta["exito"] = false;
                    respuesta["mensaje"] = "Codigo es requerido";
                    respuesta["codigo_error"] = 400;
                    return respuesta.dump();
                }
                
                if (cliente.razon_social.empty()) {
                    respuesta["exito"] = false;
                    respuesta["mensaje"] = "Razon social es requerida";
                    respuesta["codigo_error"] = 400;
                    return respuesta.dump();
                }
                
                // Verificar código único
                for (const auto& c : clientes) {
                    if (c.codigo == cliente.codigo) {
                        respuesta["exito"] = false;
                        respuesta["mensaje"] = "Ya existe un cliente con ese codigo";
                        respuesta["codigo_error"] = 409;
                        return respuesta.dump();
                    }
                }
                
                // Asignar ID y agregar
                cliente.id = proximo_id++;
                clientes.push_back(cliente);
                
                respuesta["exito"] = true;
                respuesta["mensaje"] = "Cliente creado exitosamente";
                respuesta["datos"] = cliente.id;
                
            } catch (const std::exception& e) {
                respuesta["exito"] = false;
                respuesta["mensaje"] = "JSON invalido: " + std::string(e.what());
                respuesta["codigo_error"] = 400;
            }
            
            return respuesta.dump();
        }
        
        static std::string eliminar(int id) {
            nlohmann::json respuesta;
            
            for (auto& cliente : clientes) {
                if (cliente.id == id) {
                    cliente.activo = false; // Eliminacion logica
                    respuesta["exito"] = true;
                    respuesta["mensaje"] = "Cliente eliminado exitosamente";
                    respuesta["datos"] = true;
                    return respuesta.dump();
                }
            }
            
            respuesta["exito"] = false;
            respuesta["mensaje"] = "Cliente no encontrado";
            respuesta["codigo_error"] = 400;
            return respuesta.dump();
        }
        
        // Inicializar con datos de prueba
        static void inicializar_datos_prueba() {
            clientes.clear();
            proximo_id = 1;
            
            // Cliente 1
            Cliente c1;
            c1.id = proximo_id++;
            c1.codigo = "CLI001";
            c1.razon_social = "Empresa Ejemplo SAC";
            c1.ruc = "20123456789";
            c1.direccion = "Av. Ejemplo 123, Lima";
            c1.telefono = "01-1234567";
            c1.email = "contacto@ejemplo.com";
            clientes.push_back(c1);
            
            // Cliente 2
            Cliente c2;
            c2.id = proximo_id++;
            c2.codigo = "CLI002";
            c2.razon_social = "Comercializadora Los Andes EIRL";
            c2.ruc = "20987654321";
            c2.direccion = "Jr. Los Andes 456, Arequipa";
            c2.telefono = "054-765432";
            c2.email = "ventas@losandes.com";
            clientes.push_back(c2);
            
            std::cout << "Datos de prueba inicializados: " << clientes.size() << " clientes" << std::endl;
        }
    };
    
    // Variables estaticas
    std::vector<Cliente> ClienteSimulado::clientes;
    int ClienteSimulado::proximo_id = 1;
}

// ================================================================
// FUNCIÓN PRINCIPAL
// ================================================================
int main() {
    std::cout << "ERP Sistema - Paso 5: Servidor Basico" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // Inicializar datos de prueba
    ERP::ClienteSimulado::inicializar_datos_prueba();
    
    // Crear servidor HTTP
    httplib::Server server;
    
    // Configurar CORS para permitir acceso desde navegador
    server.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        
        if (req.method == "OPTIONS") {
            res.status = 200;
            return true; // Manejado
        }
        return false; // No manejado, continuar procesamiento normal
    });
    
    // ================================================================
    // RUTAS DE LA API
    // ================================================================
    
    // GET /api/clientes - Listar todos los clientes
    server.Get("/api/clientes", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "GET /api/clientes" << std::endl;
        std::string resultado = ERP::ClienteSimulado::listar_todos();
        res.set_content(resultado, "application/json");
    });
    
    // GET /api/clientes/:id - Obtener cliente por ID
    server.Get(R"(/api/clientes/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        int id = std::stoi(req.matches[1]);
        std::cout << "GET /api/clientes/" << id << std::endl;
        std::string resultado = ERP::ClienteSimulado::obtener_por_id(id);
        res.set_content(resultado, "application/json");
    });
    
    // POST /api/clientes - Crear nuevo cliente
    server.Post("/api/clientes", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "POST /api/clientes" << std::endl;
        std::cout << "   Body: " << req.body << std::endl;
        std::string resultado = ERP::ClienteSimulado::crear(req.body);
        res.set_content(resultado, "application/json");
    });
    
    // DELETE /api/clientes/:id - Eliminar cliente
    server.Delete(R"(/api/clientes/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        int id = std::stoi(req.matches[1]);
        std::cout << "DELETE /api/clientes/" << id << std::endl;
        std::string resultado = ERP::ClienteSimulado::eliminar(id);
        res.set_content(resultado, "application/json");
    });
    
    // Ruta de estado del sistema
    server.Get("/api/status", [](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json estado;
        estado["sistema"] = "ERP";
        estado["version"] = "1.0.0-paso5";
        estado["estado"] = "activo";
        estado["timestamp"] = time(nullptr);
        res.set_content(estado.dump(), "application/json");
    });
    
    // Página de prueba HTML básica - SIMPLIFICADA
    server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ERP Sistema - Prueba</title>
    <meta charset="utf-8">
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .container { max-width: 800px; margin: 0 auto; }
        button { padding: 10px; margin: 5px; }
        .resultado { background: #f0f0f0; padding: 15px; margin: 10px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ERP Sistema - Prueba API</h1>
        <button onclick="listarClientes()">Listar Clientes</button>
        <button onclick="buscarCliente()">Buscar Cliente ID 1</button>
        <button onclick="crearCliente()">Crear Cliente</button>
        <div id="resultado">Resultado aparecera aqui</div>
    </div>

    <script>
        function mostrarResultado(data) {
            document.getElementById("resultado").textContent = JSON.stringify(data, null, 2);
        }
        
        async function listarClientes() {
            try {
                const response = await fetch("/api/clientes");
                const data = await response.json();
                mostrarResultado(data);
            } catch (error) {
                mostrarResultado({error: error.message});
            }
        }
        
        async function buscarCliente() {
            try {
                const response = await fetch("/api/clientes/1");
                const data = await response.json();
                mostrarResultado(data);
            } catch (error) {
                mostrarResultado({error: error.message});
            }
        }
        
        async function crearCliente() {
            const clienteData = {
                "codigo": "CLI003",
                "razon_social": "Nueva Empresa SAC",
                "ruc": "20111222333",
                "direccion": "Av. Nueva 789",
                "telefono": "01-9999999",
                "email": "info@nueva.com"
            };
            try {
                const response = await fetch("/api/clientes", {
                    method: "POST",
                    headers: {"Content-Type": "application/json"},
                    body: JSON.stringify(clienteData)
                });
                const data = await response.json();
                mostrarResultado(data);
            } catch (error) {
                mostrarResultado({error: error.message});
            }
        }
    </script>
</body>
</html>
)";
        res.set_content(html, "text/html");
    });
    
    // ================================================================
    // INICIAR SERVIDOR
    // ================================================================
    
    std::cout << std::endl;
    std::cout << "Servidor iniciando en: http://localhost:8080" << std::endl;
    std::cout << "API disponible en: http://localhost:8080/api/clientes" << std::endl;
    std::cout << "Pagina de prueba: http://localhost:8080/" << std::endl;
    std::cout << "Presiona Ctrl+C para detener" << std::endl;
    std::cout << std::endl;
    
    // Iniciar servidor (bloqueante)
    if (!server.listen("localhost", 8080)) {
        std::cerr << "Error: No se pudo iniciar el servidor en puerto 8080" << std::endl;
        return 1;
    }
    
    return 0;
}