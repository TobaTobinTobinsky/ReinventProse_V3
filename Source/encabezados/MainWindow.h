/*
* File Name: MainWindow.h
* Descripción: Ventana principal completa de ReinventProse 3.0.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <map>
#include <vector>
#include <string>
#include <optional>
#include <functional>
#include "DBManager.h"

// Forward declarations
class AppHandler;
class BookDetailsView;
class ChapterContentView;
class AbstractIdeaView;
class ConcreteIdeaView;
class LibraryView;
class ChapterListView;

class MainWindow : public wxFrame {
public:
    MainWindow(wxWindow* parent, const wxString& title, AppHandler* app_handler);
    virtual ~MainWindow();

    // Métodos públicos de comunicación
    void set_dirty_status_in_title(bool is_dirty);
    void on_main_window_chapter_selected(std::optional<int> chapter_id);

private:
    // --- Inicialización de Componentes ---
    void _set_application_icon();
    void _create_menu_bar();
    void _create_toolbar();
    void _create_status_bar();
    void _create_views();
    void _ensure_edit_notebook();
    wxBitmap _get_res_bmp(const unsigned char* data, unsigned int size);

    // --- Lógica de Interfaz y Estados ---
    void _update_toolbar_state(int new_state);
    void _update_library_view_layout(bool is_sidebar);
    void _highlight_active_book_in_library(std::optional<int> active_book_id);
    void _update_notebook_pages_state(bool chapter_is_selected);
    void _load_chapter_data_into_edit_views(std::optional<int> chapter_id);
    void _reevaluate_global_dirty_state();
    void _clear_chapter_views_and_selection();
    bool _confirm_discard_changes();

    // --- Persistencia y Archivos ---
    wxString _get_config_path(const wxString& filename);
    void _save_state();
    void _load_state();
    wxString _get_resource_path(const wxString& file_name);
    wxBitmap _load_tool_icon(const wxString& icon_name, const wxSize& icon_size);

    // --- Manejadores de Eventos de Menú y Herramientas ---
    void on_menu_new_book(wxCommandEvent& event);
    void on_edit_book_tool_click(wxCommandEvent& event);
    void on_save_current_book_tool_click(wxCommandEvent& event);
    void on_back_to_library_tool_click(wxCommandEvent& event);
    void on_undo_tool_click(wxCommandEvent& event);
    void on_redo_tool_click(wxCommandEvent& event);
    void on_menu_exit(wxCommandEvent& event);
    void on_menu_about(wxCommandEvent& event);

    // Nuevos manejadores Editar
    void on_menu_configuraciones(wxCommandEvent& event);
    void on_menu_skin_editor(wxCommandEvent& event);

    // Exportación
    void on_export_txt(wxCommandEvent& event);
    void on_export_docx(wxCommandEvent& event);
    void on_export_pdf(wxCommandEvent& event);

    // Inteligencia de UI
    void on_update_ui_save_button(wxUpdateUIEvent& event);
    void on_update_ui_needs_book(wxUpdateUIEvent& event);

    // Ventana y AUI
    void on_close(wxCloseEvent& event);
    void on_library_book_selected(int selected_book_id);
    void on_show_library_as_center(wxCommandEvent& event, bool force_clean = false);

    // Atributos
    wxString m_app_name;
    AppHandler* m_app_handler;
    wxPanel* m_base_panel;
    wxAuiManager m_aui_manager;

    std::optional<int> m_current_book_id;
    std::optional<int> m_current_chapter_id;
    int m_current_app_state;

    // Vistas principales
    LibraryView* m_library_view;
    BookDetailsView* m_book_details_view;
    ChapterListView* m_chapter_list_view;

    // Notebook de edición
    wxAuiNotebook* m_edit_notebook;
    ChapterContentView* m_chapter_content_view;
    AbstractIdeaView* m_abstract_idea_view;
    ConcreteIdeaView* m_concrete_idea_view;

    // Constantes
    const wxString CONFIG_DIR = ".reinventprose_v3_config";
    const wxString PERSP_FILE = "layout.txt";
    const wxString APP_ICON = "app_icon.ico";

    enum {
        ID_TOOL_EDIT_BOOK = 6001,
        ID_TOOL_BACK_TO_LIBRARY,
        ID_EXPORT_TXT,
        ID_EXPORT_DOCX,
        ID_EXPORT_PDF,
        ID_MENU_CONFIG,
        ID_MENU_SKIN
    };

    enum {
        STATE_LIBRARY = 0,
        STATE_DETAILS = 1,
        STATE_EDIT = 2
    };

    wxDECLARE_EVENT_TABLE();
};

#endif