#ifndef CLIENTE_H
#define CLIENTE_H

#include "database.h"
#include <string>
#include <vector>

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
        
        // Convertir a JSON string
        std::string to_json() const {
            std::string json = "{";
            json += "\"id\":" + std::to_string(id) + ",";
            json += "\"codigo\":\"" + codigo + "\",";
            json += "\"razon_social\":\"" + escape_json(razon_social) + "\",";
            json += "\"ruc\":\"" + ruc + "\",";
            json += "\"direccion\":\"" + escape_json(direccion) + "\",";
            json += "\"telefono\":\"" + telefono + "\",";
            json += "\"email\":\"" + email + "\",";
            json += "\"activo\":" + std::string(activo ? "true" : "false");
            json += "}";
            return json;
        }
        
    private:
        std::string escape_json(const std::string& str) const {
            std::string result;
            for (char c : str) {
                if (c == '"') result += "\\\"";
                else if (c == '\\') result += "\\\\";
                else if (c == '\n') result += "\\n";
                else if (c == '\r') result += "\\r";
                else if (c == '\t') result += "\\t";
                else result += c;
            }
            return result;
        }
    };
    
    class ClienteDAO {
    private:
        Database& db;
        
    public:
        ClienteDAO(Database& database) : db(database) {}
        
        // Crear cliente
        bool crear(const Cliente& cliente) {
            std::string query = "INSERT INTO clientes (codigo, razon_social, ruc, direccion, telefono, email) "
                              "VALUES ('" + escape_sql(cliente.codigo) + "', "
                              "'" + escape_sql(cliente.razon_social) + "', "
                              "'" + escape_sql(cliente.ruc) + "', "
                              "'" + escape_sql(cliente.direccion) + "', "
                              "'" + escape_sql(cliente.telefono) + "', "
                              "'" + escape_sql(cliente.email) + "')";
            
            return db.execute(query);
        }
        
        // Obtener todos los clientes activos
        std::vector<Cliente> obtener_todos() {
            std::vector<Cliente> clientes;
            std::string query = "SELECT id, codigo, razon_social, ruc, direccion, telefono, email, activo "
                              "FROM clientes WHERE activo = true ORDER BY razon_social";
            
            PGresult* res = db.query(query);
            if (!res) return clientes;
            
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++) {
                Cliente c;
                c.id = std::stoi(PQgetvalue(res, i, 0));
                c.codigo = PQgetvalue(res, i, 1);
                c.razon_social = PQgetvalue(res, i, 2);
                c.ruc = PQgetvalue(res, i, 3);
                c.direccion = PQgetvalue(res, i, 4);
                c.telefono = PQgetvalue(res, i, 5);
                c.email = PQgetvalue(res, i, 6);
                c.activo = (PQgetvalue(res, i, 7)[0] == 't');
                
                clientes.push_back(c);
            }
            
            PQclear(res);
            return clientes;
        }
        
        // Obtener cliente por ID
        Cliente obtener_por_id(int id) {
            Cliente cliente;
            std::string query = "SELECT id, codigo, razon_social, ruc, direccion, telefono, email, activo "
                              "FROM clientes WHERE id = " + std::to_string(id);
            
            PGresult* res = db.query(query);
            if (!res || PQntuples(res) == 0) {
                if (res) PQclear(res);
                return cliente; // Retorna cliente vacío
            }
            
            cliente.id = std::stoi(PQgetvalue(res, 0, 0));
            cliente.codigo = PQgetvalue(res, 0, 1);
            cliente.razon_social = PQgetvalue(res, 0, 2);
            cliente.ruc = PQgetvalue(res, 0, 3);
            cliente.direccion = PQgetvalue(res, 0, 4);
            cliente.telefono = PQgetvalue(res, 0, 5);
            cliente.email = PQgetvalue(res, 0, 6);
            cliente.activo = (PQgetvalue(res, 0, 7)[0] == 't');
            
            PQclear(res);
            return cliente;
        }
        
        // Actualizar cliente
        bool actualizar(const Cliente& cliente) {
            std::string query = "UPDATE clientes SET "
                              "codigo = '" + escape_sql(cliente.codigo) + "', "
                              "razon_social = '" + escape_sql(cliente.razon_social) + "', "
                              "ruc = '" + escape_sql(cliente.ruc) + "', "
                              "direccion = '" + escape_sql(cliente.direccion) + "', "
                              "telefono = '" + escape_sql(cliente.telefono) + "', "
                              "email = '" + escape_sql(cliente.email) + "', "
                              "fecha_actualizacion = CURRENT_TIMESTAMP "
                              "WHERE id = " + std::to_string(cliente.id);
            
            return db.execute(query);
        }
        
        // Eliminación lógica
        bool eliminar(int id) {
            std::string query = "UPDATE clientes SET activo = false, "
                              "fecha_actualizacion = CURRENT_TIMESTAMP "
                              "WHERE id = " + std::to_string(id);
            
            return db.execute(query);
        }
        
    private:
        std::string escape_sql(const std::string& str) {
            std::string result;
            for (char c : str) {
                if (c == '\'') result += "''";
                else result += c;
            }
            return result;
        }
    };
    
} // namespace ERP

#endif