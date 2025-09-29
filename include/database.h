#ifndef DATABASE_H
#define DATABASE_H

#include <libpq-fe.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

namespace ERP {
    
    class Database {
    private:
        PGconn* connection;
        
    public:
        Database(const std::string& conninfo) {
            connection = PQconnectdb(conninfo.c_str());
            if (PQstatus(connection) != CONNECTION_OK) {
                std::cerr << "Error de conexión: " << PQerrorMessage(connection) << std::endl;
                throw std::runtime_error("No se pudo conectar a PostgreSQL");
            }
            std::cout << "✅ Conectado a PostgreSQL" << std::endl;
        }
        
        ~Database() {
            if (connection) PQfinish(connection);
        }
        
        // Ejecutar query sin retorno
        bool execute(const std::string& query) {
            PGresult* res = PQexec(connection, query.c_str());
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                std::cerr << "Error ejecutando query: " << PQerrorMessage(connection) << std::endl;
                PQclear(res);
                return false;
            }
            PQclear(res);
            return true;
        }
        
        // Ejecutar query con retorno
        PGresult* query(const std::string& query) {
            PGresult* res = PQexec(connection, query.c_str());
            if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                std::cerr << "Error en query: " << PQerrorMessage(connection) << std::endl;
                PQclear(res);
                return nullptr;
            }
            return res;
        }
        
        // Inicializar tablas
        bool initialize_tables() {
            std::string create_table = R"(
                CREATE TABLE IF NOT EXISTS clientes (
                    id SERIAL PRIMARY KEY,
                    codigo VARCHAR(20) UNIQUE NOT NULL,
                    razon_social VARCHAR(200) NOT NULL,
                    ruc VARCHAR(11) UNIQUE NOT NULL,
                    direccion TEXT,
                    telefono VARCHAR(20),
                    email VARCHAR(100),
                    activo BOOLEAN DEFAULT true,
                    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    fecha_actualizacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );
                
                CREATE INDEX IF NOT EXISTS idx_clientes_codigo ON clientes(codigo);
                CREATE INDEX IF NOT EXISTS idx_clientes_ruc ON clientes(ruc);
                CREATE INDEX IF NOT EXISTS idx_clientes_activo ON clientes(activo);
            )";
            
            return execute(create_table);
        }
    };
    
} // namespace ERP

#endif
