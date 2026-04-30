/*
* File Name: LibraryView.h
* Descripción: Vista que muestra la colección de libros como tarjetas.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 2.0.0
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

// Estructura simplificada con BLOB en lugar de ruta
struct LibroFicha
{
    int id;
    std::string title;
    std::string author;
    std::vector<uint8_t> cover_image_data;
};

class BookCardPanel : public wxPanel
{
public:
    static const int CARD_WIDTH = 150;
    static const int IMAGE_WIDTH = 100;
    static const int IMAGE_HEIGHT = 150;

    BookCardPanel(
        wxWindow* parent,
        const LibroFicha& book_data,
        AppHandler* app_handler,
        std::function<void(int)> on_click,
        std::function<void(int)> on_delete
    );

    void set_active_style(bool is_active);
    int get_book_id() const { return m_book.id; }

private:
    void _create_controls();
    void _layout_controls();

    void on_internal_card_click(wxMouseEvent& event);
    void on_edit_btn_click(wxCommandEvent& event);
    void on_delete_btn_click(wxCommandEvent& event);
    void on_paint(wxPaintEvent& event);

    LibroFicha m_book;
    AppHandler* m_app_handler;
    std::function<void(int)> m_on_click;
    std::function<void(int)> m_on_delete;
    bool m_is_active_style;

    wxStaticBitmap* m_cover_image_ctrl;
    wxStaticText* m_title_label;
    wxStaticText* m_author_label;

    wxButton* m_btn_edit;
    wxButton* m_btn_delete;

    wxColour ACTIVE_BG_COLOUR;
    wxColour INACTIVE_BG_COLOUR;
    wxColour ACTIVE_BORDER_COLOUR;
    wxColour INACTIVE_BORDER_COLOUR;
    const int ACTIVE_BORDER_WIDTH = 2;
    const int INACTIVE_BORDER_WIDTH = 1;

    wxDECLARE_EVENT_TABLE();
};

class LibraryView : public wxScrolled<wxPanel>
{
public:
    LibraryView(wxWindow* parent, AppHandler* app_handler);
    virtual ~LibraryView();

    void set_on_book_card_selected_callback(std::function<void(int)> callback);
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
    std::function<void(int)> m_on_card_selected_callback;
    bool m_current_is_sidebar_layout;

    bool m_sort_by_id;

    wxBoxSizer* m_main_vertical_sizer;
    wxButton* m_btn_toggle_sort;

    wxWrapSizer* m_wrap_sizer;
    wxBoxSizer* m_box_sizer_vertical;
    wxPanel* m_no_books_message_widget;
};

#endif // LIBRARYVIEW_H