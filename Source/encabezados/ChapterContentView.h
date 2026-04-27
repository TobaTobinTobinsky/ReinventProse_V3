/*
* Archivo: ChapterContentView.h
* Descripción: Panel para visualizar y editar el contenido de un capítulo en texto plano (UTF-8).
*              Optimizado para español y caracteres latinoamericanos.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef CHAPTERCONTENTVIEW_H
#define CHAPTERCONTENTVIEW_H

#include <wx/wx.h>
#include <optional>
#include <string>

class AppHandler;

class ChapterContentView : public wxPanel {
public:
    ChapterContentView(wxWindow* parent, AppHandler* app_handler);

    // Métodos principales
    void load_content(std::optional<int> chapter_id);
    bool save_changes();

    // Consultas de estado
    bool is_editable() const;
    bool is_dirty() const;
    void enable_view(bool enable);
    bool force_save_if_dirty();
    void set_view_dirty(bool is_dirty = true);

private:
    // Configuración de la interfaz
    void _create_controls();
    void _layout_controls();
    void _update_edit_mode_ui();

    // Manejadores de eventos
    void on_text_changed(wxCommandEvent& event);
    void on_edit_button_click(wxCommandEvent& event);

    // Atributos de lógica
    AppHandler* app_handler;
    std::optional<int> chapter_id;
    bool _is_dirty_view;
    bool _loading_data;
    bool _is_in_edit_mode;

    // Controles (Widgets)
    wxStaticText* content_label;
    wxTextCtrl* content_ctrl;
    wxToolBar* edit_mode_toolbar;

    // IDs de controles
    enum {
        ID_EDIT_MODE_TOOL = 2001
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // CHAPTERCONTENTVIEW_H