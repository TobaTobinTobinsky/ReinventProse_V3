/**
* File Name: DBManager.cpp
* Descripción: Implementación del gestor de base de datos SQLite con soporte BLOB.
*/

#include "../encabezados/DBManager.h"
#include <iostream>

DBManager* DBManager::_instance = nullptr;

DBManager::DBManager(const std::string& path)
    : db_path(path), connection(nullptr), _initialized_dbm(true)
{
}

DBManager::~DBManager()
{
    _disconnect();
}

DBManager* DBManager::get_instance(const std::string& db_path)
{
    if (_instance == nullptr)
    {
        _instance = new DBManager(db_path);
    }
    return _instance;
}

void DBManager::_connect()
{
    if (connection == nullptr)
    {
        int rc = sqlite3_open(db_path.c_str(), &connection);
        if (rc != SQLITE_OK)
        {
            std::string err = sqlite3_errmsg(connection);
            connection = nullptr;
            throw DatabaseError("Error al conectar con la base de datos: " + err);
        }
        sqlite3_exec(connection, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    }
}

void DBManager::_disconnect()
{
    if (connection != nullptr)
    {
        sqlite3_close(connection);
        connection = nullptr;
    }
}

void DBManager::create_database()
{
    _connect();
    char* errMsg = nullptr;

    // ATENCIÓN: cover_image ahora es BLOB
    const char* sql =
        "CREATE TABLE IF NOT EXISTS books ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL UNIQUE,"
        "author TEXT NOT NULL,"
        "synopsis TEXT,"
        "prologue TEXT,"
        "back_cover_text TEXT,"
        "cover_image_data BLOB);"

        "CREATE TABLE IF NOT EXISTS chapters ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "book_id INTEGER NOT NULL,"
        "chapter_number INTEGER NOT NULL,"
        "title TEXT NOT NULL,"
        "content TEXT,"
        "abstract_idea TEXT,"
        "FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,"
        "UNIQUE (book_id, chapter_number));"

        "CREATE TABLE IF NOT EXISTS concrete_ideas ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "chapter_id INTEGER NOT NULL,"
        "idea TEXT NOT NULL,"
        "FOREIGN KEY (chapter_id) REFERENCES chapters(id) ON DELETE CASCADE);";

    int rc = sqlite3_exec(connection, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::string error(errMsg);
        sqlite3_free(errMsg);
        throw DatabaseError("Error al inicializar tablas: " + error);
    }
    _disconnect();
}

int DBManager::create_book(
    const std::string& title,
    const std::string& author,
    const std::string& synopsis,
    const std::string& prologue,
    const std::string& back_cover_text,
    const std::vector<uint8_t>& cover_image_data)
{
    _connect();
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO books (title, author, synopsis, prologue, back_cover_text, cover_image_data) VALUES (?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(connection, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw DatabaseError(sqlite3_errmsg(connection));
    }

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, author.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, synopsis.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, prologue.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, back_cover_text.c_str(), -1, SQLITE_TRANSIENT);

    // Bind del BLOB
    if (!cover_image_data.empty())
    {
        sqlite3_bind_blob(stmt, 6, cover_image_data.data(), (int)cover_image_data.size(), SQLITE_TRANSIENT);
    }
    else
    {
        sqlite3_bind_null(stmt, 6);
    }

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::string err = sqlite3_errmsg(connection);
        sqlite3_finalize(stmt);
        throw BookCreationError("Fallo al insertar libro: " + err);
    }

    int id = (int)sqlite3_last_insert_rowid(connection);
    sqlite3_finalize(stmt);
    _disconnect();

    return id;
}

std::optional<DBRow> DBManager::get_book_by_id(int book_id)
{
    _connect();
    sqlite3_stmt* stmt;
    std::optional<DBRow> result = std::nullopt;
    const char* sql = "SELECT * FROM books WHERE id = ?;";

    if (sqlite3_prepare_v2(connection, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, book_id);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            DBRow row;
            for (int i = 0; i < sqlite3_column_count(stmt); i++)
            {
                std::string name = sqlite3_column_name(stmt, i);
                int type = sqlite3_column_type(stmt, i);

                if (type == SQLITE_INTEGER)
                {
                    row[name] = (long long)sqlite3_column_int64(stmt, i);
                }
                else if (type == SQLITE_FLOAT)
                {
                    row[name] = sqlite3_column_double(stmt, i);
                }
                else if (type == SQLITE_BLOB)
                {
                    const uint8_t* blob = (const uint8_t*)sqlite3_column_blob(stmt, i);
                    int bytes = sqlite3_column_bytes(stmt, i);
                    if (blob && bytes > 0)
                    {
                        row[name] = std::vector<uint8_t>(blob, blob + bytes);
                    }
                    else
                    {
                        row[name] = std::vector<uint8_t>();
                    }
                }
                else
                {
                    const char* text = (const char*)sqlite3_column_text(stmt, i);
                    row[name] = text ? std::string(text) : std::string("");
                }
            }
            result = row;
        }
    }

    sqlite3_finalize(stmt);
    _disconnect();

    if (!result)
    {
        throw BookNotFoundError("Libro ID " + std::to_string(book_id) + " no encontrado.");
    }

    return result;
}

std::vector<DBRow> DBManager::get_all_books()
{
    _connect();
    std::vector<DBRow> books;
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(connection, "SELECT * FROM books ORDER BY id;", -1, &stmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            DBRow row;
            for (int i = 0; i < sqlite3_column_count(stmt); i++)
            {
                std::string name = sqlite3_column_name(stmt, i);
                int type = sqlite3_column_type(stmt, i);

                if (type == SQLITE_INTEGER)
                {
                    row[name] = (long long)sqlite3_column_int64(stmt, i);
                }
                else if (type == SQLITE_FLOAT)
                {
                    row[name] = sqlite3_column_double(stmt, i);
                }
                else if (type == SQLITE_BLOB)
                {
                    const uint8_t* blob = (const uint8_t*)sqlite3_column_blob(stmt, i);
                    int bytes = sqlite3_column_bytes(stmt, i);
                    if (blob && bytes > 0)
                    {
                        row[name] = std::vector<uint8_t>(blob, blob + bytes);
                    }
                    else
                    {
                        row[name] = std::vector<uint8_t>();
                    }
                }
                else
                {
                    const char* text = (const char*)sqlite3_column_text(stmt, i);
                    row[name] = text ? std::string(text) : std::string("");
                }
            }
            books.push_back(row);
        }
    }

    sqlite3_finalize(stmt);
    _disconnect();

    return books;
}

bool DBManager::update_book(
    int book_id,
    const std::string& title,
    const std::string& author,
    const std::string& synopsis,
    const std::string& prologue,
    const std::string& back_cover_text,
    const std::vector<uint8_t>& cover_image_data)
{
    _connect();
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE books SET title=?, author=?, synopsis=?, prologue=?, back_cover_text=?, cover_image_data=? WHERE id=?;";
    sqlite3_prepare_v2(connection, sql, -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, author.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, synopsis.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, prologue.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, back_cover_text.c_str(), -1, SQLITE_TRANSIENT);

    if (!cover_image_data.empty())
    {
        sqlite3_bind_blob(stmt, 6, cover_image_data.data(), (int)cover_image_data.size(), SQLITE_TRANSIENT);
    }
    else
    {
        sqlite3_bind_null(stmt, 6);
    }

    sqlite3_bind_int(stmt, 7, book_id);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    _disconnect();

    return ok;
}

bool DBManager::delete_book(int book_id)
{
    _connect();
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(connection, "DELETE FROM books WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, book_id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    _disconnect();

    return ok;
}

int DBManager::create_chapter(int book_id, int chapter_number, const std::string& title, const std::string& content, const std::string& abstract_idea)
{
    _connect();
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO chapters (book_id, chapter_number, title, content, abstract_idea) VALUES (?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(connection, sql, -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, book_id);
    sqlite3_bind_int(stmt, 2, chapter_number);
    sqlite3_bind_text(stmt, 3, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, abstract_idea.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        throw ChapterCreationError("Error al crear capítulo.");
    }

    int id = (int)sqlite3_last_insert_rowid(connection);
    sqlite3_finalize(stmt);
    _disconnect();

    return id;
}

std::vector<DBRow> DBManager::get_chapters_by_book_id(int book_id)
{
    _connect();
    std::vector<DBRow> chapters;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(connection, "SELECT * FROM chapters WHERE book_id = ? ORDER BY chapter_number;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, book_id);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        DBRow row;
        for (int i = 0; i < sqlite3_column_count(stmt); i++)
        {
            std::string name = sqlite3_column_name(stmt, i);
            int type = sqlite3_column_type(stmt, i);

            if (type == SQLITE_INTEGER)
            {
                row[name] = (long long)sqlite3_column_int64(stmt, i);
            }
            else if (type == SQLITE_FLOAT)
            {
                row[name] = sqlite3_column_double(stmt, i);
            }
            else
            {
                const char* text = (const char*)sqlite3_column_text(stmt, i);
                row[name] = text ? std::string(text) : std::string("");
            }
        }
        chapters.push_back(row);
    }

    sqlite3_finalize(stmt);
    _disconnect();

    return chapters;
}

std::optional<DBRow> DBManager::get_chapter_by_id(int chapter_id)
{
    _connect();
    sqlite3_stmt* stmt;
    std::optional<DBRow> result = std::nullopt;
    sqlite3_prepare_v2(connection, "SELECT * FROM chapters WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, chapter_id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        DBRow row;
        for (int i = 0; i < sqlite3_column_count(stmt); i++)
        {
            std::string name = sqlite3_column_name(stmt, i);
            int type = sqlite3_column_type(stmt, i);

            if (type == SQLITE_INTEGER)
            {
                row[name] = (long long)sqlite3_column_int64(stmt, i);
            }
            else if (type == SQLITE_FLOAT)
            {
                row[name] = sqlite3_column_double(stmt, i);
            }
            else
            {
                const char* text = (const char*)sqlite3_column_text(stmt, i);
                row[name] = text ? std::string(text) : std::string("");
            }
        }
        result = row;
    }

    sqlite3_finalize(stmt);
    _disconnect();

    return result;
}

bool DBManager::update_chapter_title(int chapter_id, const std::string& new_title)
{
    _connect();
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(connection, "UPDATE chapters SET title = ? WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, new_title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, chapter_id);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    _disconnect();

    return ok;
}

bool DBManager::delete_chapter(int chapter_id)
{
    _connect();
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(connection, "DELETE FROM chapters WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, chapter_id);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    _disconnect();

    return ok;
}

bool DBManager::update_chapter_abstract_idea(int chapter_id, const std::string& abstract_idea)
{
    _connect();
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(connection, "UPDATE chapters SET abstract_idea = ? WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, abstract_idea.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, chapter_id);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    _disconnect();

    return ok;
}

bool DBManager::update_chapter_content_only(int chapter_id, const std::string& content)
{
    _connect();
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(connection, "UPDATE chapters SET content = ? WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, chapter_id);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    _disconnect();

    return ok;
}

int DBManager::add_concrete_idea(int chapter_id, const std::string& idea)
{
    _connect();
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(connection, "INSERT INTO concrete_ideas (chapter_id, idea) VALUES (?, ?);", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, chapter_id);
    sqlite3_bind_text(stmt, 2, idea.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);

    int id = (int)sqlite3_last_insert_rowid(connection);
    sqlite3_finalize(stmt);
    _disconnect();

    return id;
}

std::vector<DBRow> DBManager::get_concrete_ideas_by_chapter_id(int chapter_id)
{
    _connect();
    std::vector<DBRow> ideas;
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(connection, "SELECT * FROM concrete_ideas WHERE chapter_id = ? ORDER BY id ASC;", -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, chapter_id);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            DBRow row;
            row["id"] = (long long)sqlite3_column_int64(stmt, 0);
            const char* text = (const char*)sqlite3_column_text(stmt, 2);
            row["idea"] = text ? std::string(text) : std::string("");
            ideas.push_back(row);
        }
    }

    sqlite3_finalize(stmt);
    _disconnect();

    return ideas;
}

bool DBManager::update_concrete_idea(int idea_id, const std::string& idea_text)
{
    _connect();
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(connection, "UPDATE concrete_ideas SET idea = ? WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, idea_text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, idea_id);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    _disconnect();

    return ok;
}

bool DBManager::delete_concrete_idea(int idea_id)
{
    _connect();
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(connection, "DELETE FROM concrete_ideas WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, idea_id);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    _disconnect();

    return ok;
}

bool DBManager::swap_concrete_idea_ids(int id_source, int id_target)
{
    _connect();
    sqlite3_exec(connection, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    sqlite3_stmt* stmt;

    // 1. Mover Target a ID 0
    sqlite3_prepare_v2(connection, "UPDATE concrete_ideas SET id = 0 WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id_target);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // 2. Mover Source a ID Target
    sqlite3_prepare_v2(connection, "UPDATE concrete_ideas SET id = ? WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id_target);
    sqlite3_bind_int(stmt, 2, id_source);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // 3. Mover ID 0 a ID Source
    sqlite3_prepare_v2(connection, "UPDATE concrete_ideas SET id = ? WHERE id = 0;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id_source);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    bool ok = (sqlite3_exec(connection, "COMMIT;", nullptr, nullptr, nullptr) == SQLITE_OK);
    _disconnect();

    return ok;
}