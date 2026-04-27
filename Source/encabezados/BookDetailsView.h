/*
* File Name: BookDetailsView.h
* Description: Panel de wxWidgets para mostrar y editar los detalles principales de un libro.
* Author: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef BOOKDETAILSVIEW_H
#define BOOKDETAILSVIEW_H

#include <wx/wx.h>
#include <wx/statbmp.h>
#include <optional>
#include <string>

// Reenvío de declaración para evitar dependencias circulares
class AppHandler;

class BookDetailsView : public wxPanel {
public:
    /**
     * Inicializa el panel BookDetailsView.
     * @param parent: La ventana o panel padre.
     * @param app_handler: Manejador central de la lógica de negocio.
     */
    BookDetailsView(wxWindow* parent, AppHandler* app_handler);

    /**
     * Carga los detalles de un libro específico en los controles de la vista.
     * @param book_id: El ID del libro, o std::nullopt para limpiar la vista.
     */
    void load_book_details(std::optional<int> book_id);

    /**
     * Guarda los cambios realizados en los detalles del libro actual.
     * @return true si los cambios se guardaron exitosamente.
     */
    bool save_changes();

    /**
     * Verifica si la vista tiene cambios sin guardar.
     */
    bool is_dirty() const;

    /**
     * Habilita o deshabilita todos los controles de entrada de la vista.
     */
    void enable_view(bool enable);

    /**
     * Obtiene la ruta del archivo de la imagen de portada actualmente mostrada.
     */
    std::optional<std::string> get_current_image_path() const;

    /**
     * Establece el estado 'sucio' (modificado) de la vista manualmente.
     */
    void set_view_dirty(bool is_dirty = true);

private:
    // Construcción de la interfaz de usuario
    void _create_controls();
    void _layout_controls();
    void _update_controls_state();

    // Manejadores de eventos
    void on_text_changed(wxCommandEvent& event);
    void on_image_clicked(wxMouseEvent& event);

    // Atributos de lógica de negocio y estado
    AppHandler* app_handler;
    std::optional<int> book_id;
    bool _is_dirty_view;
    bool _loading_data;
    std::optional<std::string> current_cover_image_path;

    // Etiquetas estáticas
    wxStaticText* title_label;
    wxStaticText* author_label;
    wxStaticText* synopsis_label;
    wxStaticText* prologue_label;
    wxStaticText* back_cover_text_label;
    wxStaticText* cover_image_label_text;

    // Campos de entrada de texto
    wxTextCtrl* title_ctrl;
    wxTextCtrl* author_ctrl;
    wxTextCtrl* synopsis_ctrl;
    wxTextCtrl* prologue_ctrl;
    wxTextCtrl* back_cover_text_ctrl;

    // Control para mostrar la imagen de portada
    wxStaticBitmap* cover_image_display;
};

#endif // BOOKDETAILSVIEW_H