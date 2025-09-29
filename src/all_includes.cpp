// all_includes.cpp
// Este archivo fuerza la compilación de todas las implementaciones

#include "../include/database.h"
#include "../include/cliente.h"
#include "../include/cliente_controller.h"
#include "../include/miniserver.h"

// Instanciaciones explícitas para forzar la compilación
void compile_all() {
    // Estas variables forzarán la compilación de todos los métodos
    std::string conninfo = "test";
    
    // Forzar compilación del constructor y destructor de Database
    ERP::Database db(conninfo);
    
    // Forzar compilación de ClienteDAO
    ERP::ClienteDAO dao(db);
    
    // Forzar compilación de ClienteController  
    ERP::ClienteController controller(dao);
    
    // Forzar compilación de MiniServer
    MiniServer server(8080);
    
    // Forzar compilación de algunos métodos
    db.execute("TEST");
    db.query("TEST");
    db.initialize_tables();
    
    // Evitar warnings de variables no usadas
    (void)db;
    (void)dao;
    (void)controller;
    (void)server;
}