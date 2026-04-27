/*
* File Name: DBManager.h
* Descripción: Clase para gestionar la conexión y operaciones con la base de datos SQLite.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include <stdexcept>

// --- Excepciones Personalizadas (Corregidas para evitar error C2665) ---

class DatabaseError : public std::runtime_error {
public:
    explicit DatabaseError(const std::string& message) : std::runtime_error(std::string(message)) {}
};

class BookError : public DatabaseError {
public:
    explicit BookError(const std::string& message) : DatabaseError(std::string(message)) {}
};

class BookCreationError : public BookError {
public:
    explicit BookCreationError(const std::string& message) : BookError(std::string(message)) {}
};

class BookUpdateError : public BookError {
public:
    explicit BookUpdateError(const std::string& message) : BookError(std::string(message)) {}
};

class BookNotFoundError : public BookError {
public:
    explicit BookNotFoundError(const std::string& message) : BookError(std::string(message)) {}
};

class BookDeletionError : public BookError {
public:
    explicit BookDeletionError(const std::string& message) : BookError(std::string(message)) {}
};

class ChapterError : public DatabaseError {
public:
    explicit ChapterError(const std::string& message) : DatabaseError(std::string(message)) {}
};

class ChapterCreationError : public ChapterError {
public:
    explicit ChapterCreationError(const std::string& message) : ChapterError(std::string(message)) {}
};

class ChapterUpdateError : public ChapterError {
public:
    explicit ChapterUpdateError(const std::string& message) : ChapterError(std::string(message)) {}
};

class ChapterNotFoundError : public ChapterError {
public:
    explicit ChapterNotFoundError(const std::string& message) : ChapterError(std::string(message)) {}
};

class ChapterDeletionError : public ChapterError {
public:
    explicit ChapterDeletionError(const std::string& message) : ChapterError(std::string(message)) {}
};

class ConcreteIdeaError : public DatabaseError {
public:
    explicit ConcreteIdeaError(const std::string& message) : DatabaseError(std::string(message)) {}
};

/**
 * Tipos para representar una fila de la base de datos (Equivalente a sqlite3.Row).
 * Usamos variant para manejar tipos dinámicos de SQLite (Enteros, Texto, Reales).
 */
using DBValue = std::variant<std::monostate, long long, std::string, double>;
using DBRow = std::map<std::string, DBValue>;

class DBManager {
public:
    /**
     * Implementa el patrón Singleton. Retorna la única instancia de DBManager.
     * @param db_path Ruta al archivo de base de datos.
     */
    static DBManager* get_instance(const std::string& db_path);

    // Deshabilitar copia y asignación
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    /**
     * Crea las tablas iniciales si no existen.
     */
    void create_database();

    // --- Operaciones CRUD para LIBROS ---

    int create_book(const std::string& title, const std::string& author, const std::string& synopsis,
        const std::string& prologue, const std::string& back_cover_text, const std::string& cover_image_path);

    std::optional<DBRow> get_book_by_id(int book_id);

    std::vector<DBRow> get_all_books();

    bool update_book(int book_id, const std::string& title, const std::string& author, const std::string& synopsis,
        const std::string& prologue, const std::string& back_cover_text, const std::string& cover_image_path);

    bool delete_book(int book_id);

    // --- Operaciones CRUD para CAPÍTULOS ---

    int create_chapter(int book_id, int chapter_number, const std::string& title,
        const std::string& content = "", const std::string& abstract_idea = "");

    std::optional<DBRow> get_chapter_by_id(int chapter_id);

    std::vector<DBRow> get_chapters_by_book_id(int book_id);

    bool update_chapter(int chapter_id, int chapter_number, const std::string& title,
        const std::string& content, const std::string& abstract_idea);

    bool update_chapter_title(int chapter_id, const std::string& new_title);

    bool delete_chapter(int chapter_id);

    bool update_chapter_abstract_idea(int chapter_id, const std::string& abstract_idea);

    bool update_chapter_content_only(int chapter_id, const std::string& content);

    // --- Operaciones CRUD para IDEAS CONCRETAS ---

    int add_concrete_idea(int chapter_id, const std::string& idea);

    std::vector<DBRow> get_concrete_ideas_by_chapter_id(int chapter_id);

    bool update_concrete_idea(int idea_id, const std::string& idea_text);

    bool delete_concrete_idea(int idea_id);

private:
    // Constructor privado
    explicit DBManager(const std::string& db_path);

    // Destructor privado
    ~DBManager();

    void _connect();
    void _disconnect();

    static DBManager* _instance;
    std::string db_path;
    sqlite3* connection;
    bool _initialized_dbm;
};

#endif // DBMANAGER_H