/*
* File Name: ChapterListView.h
* Description: Vista que muestra la lista de capítulos de un libro seleccionado.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef CHAPTERLISTVIEW_H
#define CHAPTERLISTVIEW_H

#include <wx/wx.h>
#include <vector>
#include <string>
#include <functional>
#include <optional>
#include "DBManager.h" // Para usar el tipo estricto DBRow

class AppHandler;

class ChapterListView : public wxPanel {
public:
    /**
     * Inicializa el panel de la lista de capítulos.
     */
    ChapterListView(wxWindow* parent, AppHandler* app_handler);

    /**
     * Establece el callback para notificar al MainWindow cuando se selecciona un capítulo.
     */
    void set_on_chapter_selected_callback(std::function<void(std::optional<int>)> callback);

    /**
     * Carga todos los capítulos asociados a un book_id.
     */
    void load_chapters(std::optional<int> book_id);

    /**
     * Selecciona visualmente un capítulo en la lista usando su ID.
     */
    void select_chapter_by_id(std::optional<int> chapter_id);

    /**
     * Obtiene el ID del capítulo actualmente seleccionado en la interfaz.
     */
    std::optional<int> get_selected_chapter_id() const;

private:
    // Construcción de la UI
    void _create_controls();
    void _layout_controls();
    void _update_button_states();

    // Manejadores de Eventos (Se deben declarar en la tabla de eventos del CPP)
    void on_listbox_select(wxCommandEvent& event);
    void on_listbox_dclick(wxCommandEvent& event);
    void on_add_chapter(wxCommandEvent& event);
    void on_edit_chapter(wxCommandEvent& event);
    void on_delete_chapter(wxCommandEvent& event);

    // Atributos de estado y negocio
    AppHandler* app_handler;
    std::optional<int> book_id;

    // Lista en caché con los datos de los capítulos
    std::vector<DBRow> chapters_data;

    // Función delegada a la ventana principal
    std::function<void(std::optional<int>)> on_chapter_selected_callback;

    // Controles visuales (Widgets)
    wxStaticText* list_label;
    wxListBox* chapter_list_ctrl;
    wxButton* add_chapter_button;
    wxButton* edit_chapter_button;
    wxButton* delete_chapter_button;

    // Enumeración de IDs para los botones locales
    enum {
        ID_ADD_CHAPTER = 3001,
        ID_EDIT_CHAPTER,
        ID_DELETE_CHAPTER
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // CHAPTERLISTVIEW_H