/*
* File Name: LibraryView.h
* Descripción: Vista que muestra la colección de libros como tarjetas de lujo (Renderizado Procedural).
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 3.1.0
*/

#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <wx/wrapsizer.h>
#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <cstdint>

class AppHandler;

// Estructura simplificada con BLOB
struct LibroFicha
{
    int id;
    std::string title;
    std::string author;
    std::vector<uint8_t> cover_image_data;
};

// --- Panel Individual de Tarjeta de Libro (Ficha de Lujo) ---
class BookCardPanel : public wxPanel
{
public:
    static const int CARD_WIDTH = 160;
    static const int CARD_HEIGHT = 270;
    static const int IMAGE_WIDTH = 100;
    static const int IMAGE_HEIGHT = 150;

    BookCardPanel(
        wxWindow* parent,
        const LibroFicha& book_data,
        AppHandler* app_handler,
        std::function<void(int)> on_details_click,
        std::function<void(int)> on_read_click,
        std::function<void(int)> on_delete_click
    );

    void set_active_style(bool is_active);
    int get_book_id() const { return m_book.id; }

private:
    // Eventos de dibujado y clics nativos
    void on_internal_card_click(wxMouseEvent& event);
    void on_paint(wxPaintEvent& event);

    LibroFicha m_book;
    AppHandler* m_app_handler;

    // Callbacks con inteligencia de contexto
    std::function<void(int)> m_on_details_click;
    std::function<void(int)> m_on_read_click;
    std::function<void(int)> m_on_delete_click;

    bool m_is_active_style;

    // Almacenamos el bitmap en memoria para no decodificar el BLOB en cada frame
    wxBitmap m_cover_bitmap;

    // Colores de estado
    wxColour ACTIVE_BG_COLOUR;
    wxColour INACTIVE_BG_COLOUR;

    wxDECLARE_EVENT_TABLE();
};

// --- Panel Contenedor de la Biblioteca ---
class LibraryView : public wxScrolled<wxPanel>
{
public:
    LibraryView(wxWindow* parent, AppHandler* app_handler);
    virtual ~LibraryView();

    void set_on_book_card_selected_callback(std::function<void(int)> callback);
    void set_on_book_read_callback(std::function<void(int)> callback);

    void set_layout_mode(bool is_sidebar);
    void load_books();
    void clear_view();

    const std::vector<BookCardPanel*>& get_book_card_panels() const { return m_book_card_panels; }

private:
    wxPanel* _create_no_books_message_widget();
    void _clear_and_destroy_book_cards();
    void _on_delete_book_requested(int book_id);
    void _on_toggle_sort(wxCommandEvent& event);

    AppHandler* m_app_handler;
    std::vector<BookCardPanel*> m_book_card_panels;

    std::function<void(int)> m_on_card_selected_callback; // Para Detalles
    std::function<void(int)> m_on_book_read_callback;     // Para Leer/Editar

    bool m_current_is_sidebar_layout;
    bool m_sort_by_id;

    wxBoxSizer* m_main_vertical_sizer;
    wxButton* m_btn_toggle_sort;

    wxWrapSizer* m_wrap_sizer;
    wxBoxSizer* m_box_sizer_vertical;
    wxPanel* m_no_books_message_widget;
};

#endif // LIBRARYVIEW_H