/*
* File Name: AppHandler.h
* Descripción: Gestiona la lógica de negocio y actúa como puente entre la UI y la DB.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef APPHANDLER_H
#define APPHANDLER_H

#include <wx/wx.h>
#include <vector>
#include <map>
#include <string>
#include <optional>
#include <functional>
#include "DBManager.h"

// Reenvío de declaración de la ventana principal para evitar inclusiones circulares
class MainWindow;

class AppHandler {
public:
    /**
     * Método estático para obtener la única instancia de AppHandler (Singleton).
     * @param db_manager Puntero opcional al gestor de base de datos.
     * @param db_name Nombre o ruta del archivo de base de datos.
     * @return Puntero a la instancia única.
     */
    static AppHandler* get_instance(DBManager* db_manager = nullptr, const wxString& db_name = "reinventprose_v2_data.db");

    // Deshabilitar copia y asignación para el Singleton
    AppHandler(const AppHandler&) = delete;
    AppHandler& operator=(const AppHandler&) = delete;

    /**
     * Inicializa la base de datos creando las tablas necesarias.
     */
    void initialize_database();

    /**
     * Establece la referencia a la ventana principal.
     */
    void set_main_window(wxFrame* main_window);

    /**
     * Obtiene la referencia a la ventana principal.
     */
    wxFrame* get_main_window() const;

    // --- GESTIÓN DE LIBROS ---

    std::optional<int> create_new_book(const wxString& title, const wxString& author, const wxString& synopsis,
        const wxString& prologue, const wxString& back_cover_text, const wxString& cover_image_path);

    std::vector<DBRow> get_all_books();

    std::optional<DBRow> get_book_details(int book_id);

    bool update_book_details(int book_id, const wxString& title, const wxString& author, const wxString& synopsis,
        const wxString& prologue, const wxString& back_cover_text, const wxString& cover_image_path);

    // --- GESTIÓN DE CAPÍTULOS ---

    std::vector<DBRow> get_chapters_by_book_id(int book_id);

    std::optional<DBRow> get_chapter_details(int chapter_id);

    std::optional<int> create_new_chapter(int book_id, int chapter_number, const wxString& title,
        const wxString& content = "", const wxString& abstract_idea = "");

    bool delete_chapter(int chapter_id);

    bool update_chapter_title(int chapter_id, const wxString& new_title);

    bool update_chapter_content_via_handler(int chapter_id, const wxString& content);

    bool update_chapter_abstract_idea_via_handler(int chapter_id, const wxString& abstract_idea);

    // --- GESTIÓN DE IDEAS CONCRETAS ---

    std::vector<DBRow> get_concrete_ideas_for_chapter(int chapter_id);

    std::optional<int> add_concrete_idea_for_chapter(int chapter_id, const wxString& idea_text);

    bool update_concrete_idea_text(int concrete_idea_id, const wxString& new_text);

    bool delete_concrete_idea_by_id(int concrete_idea_id);

    // --- ESTADO DE CAMBIOS (DIRTY) ---

    void set_dirty(bool is_dirty = true);

    bool is_application_dirty() const;

    void prompt_save_changes(std::function<void()> on_save, std::function<void()> on_discard, std::function<void()> on_cancel);

private:
    // Constructor privado para asegurar el Singleton
    AppHandler(DBManager* db_manager, const wxString& db_name);

    static AppHandler* _instance;
    DBManager* db_manager;
    wxFrame* main_window;
    bool _is_dirty;
    bool _initialized;
};

#endif // APPHANDLER_H