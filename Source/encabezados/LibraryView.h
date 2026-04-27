/*
* File Name: LibraryView.h
* Descripción: Vista que muestra la colección de libros como tarjetas.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <wx/wrapsizer.h> // Incluido para evitar el error C2061
#include <vector>
#include <string>
#include <functional>
#include <optional>

class AppHandler;

// Estructura simplificada para usar los datos en las tarjetas
struct LibroFicha {
    int id;
    std::string title;
    std::string author;
    std::string cover_image_path;
};

// --- Panel Individual de Tarjeta de Libro ---
class BookCardPanel : public wxPanel {
public:
    // Constantes de apariencia
    static const int CARD_WIDTH = 150;
    static const int IMAGE_WIDTH = 100;
    static const int IMAGE_HEIGHT = 150;

    /**
     * Inicializa la tarjeta de un libro.
     */
    BookCardPanel(wxWindow* parent, const LibroFicha& book_data, AppHandler* app_handler, std::function<void(int)> on_click);

    void set_active_style(bool is_active);
    int get_book_id() const { return m_book.id; }

private:
    void _create_controls();
    void _layout_controls();

    void on_internal_card_click(wxMouseEvent& event);
    void on_paint(wxPaintEvent& event);

    LibroFicha m_book; // Ahora se inicializa correctamente por copia del struct
    AppHandler* m_app_handler;
    std::function<void(int)> m_on_click;
    bool m_is_active_style;

    wxStaticBitmap* m_cover_image_ctrl;
    wxStaticText* m_title_label;
    wxStaticText* m_author_label;

    wxColour ACTIVE_BG_COLOUR;
    wxColour INACTIVE_BG_COLOUR;
    wxColour ACTIVE_BORDER_COLOUR;
    wxColour INACTIVE_BORDER_COLOUR;
    const int ACTIVE_BORDER_WIDTH = 2;
    const int INACTIVE_BORDER_WIDTH = 1;

    wxDECLARE_EVENT_TABLE();
};

// --- Panel Contenedor de la Biblioteca ---
class LibraryView : public wxScrolled<wxPanel> {
public:
    LibraryView(wxWindow* parent, AppHandler* app_handler);
    virtual ~LibraryView();

    void set_on_book_card_selected_callback(std::function<void(int)> callback);
    void set_layout_mode(bool is_sidebar);
    void load_books();
    void clear_view(); // C mayúscula cambiada a minúscula para mantener convención

    const std::vector<BookCardPanel*>& get_book_card_panels() const { return m_book_card_panels; }

private:
    wxPanel* _create_no_books_message_widget();
    void _clear_and_destroy_book_cards();

    AppHandler* m_app_handler;
    std::vector<BookCardPanel*> m_book_card_panels;
    std::function<void(int)> m_on_card_selected_callback;
    bool m_current_is_sidebar_layout;

    wxWrapSizer* m_wrap_sizer;
    wxBoxSizer* m_box_sizer_vertical;
    wxPanel* m_no_books_message_widget;
};

#endif // LIBRARYVIEW_H