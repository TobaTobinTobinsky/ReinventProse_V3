/*
* File Name: AppHandler.h
* Descripción: Puente lógico entre UI y DB.
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

class MainWindow;

class AppHandler
{
public:
    static AppHandler* get_instance(
        DBManager* db_manager = nullptr,
        const wxString& db_name = "reinventprose_v3_data.db"
    );

    AppHandler(const AppHandler&) = delete;
    AppHandler& operator=(const AppHandler&) = delete;

    void initialize_database();
    void set_main_window(wxFrame* main_window);
    wxFrame* get_main_window() const;

    // Libros
    std::optional<int> create_new_book(const wxString& title, const wxString& author, const wxString& synopsis,
        const wxString& prologue, const wxString& back_cover_text, const wxString& cover_image_path);
    std::vector<DBRow> get_all_books();
    std::optional<DBRow> get_book_details(int book_id);
    bool update_book_details(int book_id, const wxString& title, const wxString& author, const wxString& synopsis,
        const wxString& prologue, const wxString& back_cover_text, const wxString& cover_image_path);
    bool delete_book(int book_id);

    // Capítulos
    std::vector<DBRow> get_chapters_by_book_id(int book_id);
    std::optional<DBRow> get_chapter_details(int chapter_id);
    std::optional<int> create_new_chapter(int book_id, int chapter_number, const wxString& title,
        const wxString& content = "", const wxString& abstract_idea = "");
    bool delete_chapter(int chapter_id);
    bool update_chapter_title(int chapter_id, const wxString& new_title);
    bool update_chapter_content_via_handler(int chapter_id, const wxString& content);
    bool update_chapter_abstract_idea_via_handler(int chapter_id, const wxString& abstract_idea);

    // Ideas
    std::vector<DBRow> get_concrete_ideas_for_chapter(int chapter_id);
    std::optional<int> add_concrete_idea_for_chapter(int chapter_id, const wxString& idea_text);
    bool update_concrete_idea_text(int concrete_idea_id, const wxString& new_text);
    bool delete_concrete_idea_by_id(int concrete_idea_id);
    bool swap_concrete_idea_positions(int id1, int id2);

    // Estados
    void set_dirty(bool is_dirty = true);
    bool is_application_dirty() const;
    void prompt_save_changes(std::function<void()> on_save, std::function<void()> on_discard, std::function<void()> on_cancel);

private:
    AppHandler(DBManager* db_manager, const wxString& db_name);
    static AppHandler* _instance;
    DBManager* db_manager;
    wxFrame* main_window;
    bool _is_dirty;
    bool _initialized;
};

#endif