/**
* File Name: LibraryView.cpp
* Descripción: Implementación de la vista de biblioteca.
*              Blindaje total mediante wxString::Format para estabilidad Unicode.
* Autor: AutoDoc AI (Protocolo de Estabilidad de Cadenas v3)
* Date: 07/06/2025
*/

#include "../encabezados/LibraryView.h"
#include "../encabezados/AppHandler.h"
#include "../encabezados/Util.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/msgdlg.h>
#include <wx/mstream.h>
#include <algorithm>

// ============================================================================
// AUXILIAR: FUNCIÓN DE RENDERIZADO GLOSSY 3D
// ============================================================================
void DrawLibraryGlossyComponent(
    wxGraphicsContext* gc,
    double x, double y, double w, double h, double r,
    wxColour baseColor,
    wxString label,
    bool isDarkText = false)
{
    wxGraphicsBrush bezel = gc->CreateLinearGradientBrush(
        x, y, x + w, y + h,
        wxColour(255, 255, 255), wxColour(150, 150, 150)
    );
    gc->SetBrush(bezel);
    gc->SetPen(wxPen(wxColour(100, 100, 100), 1));
    gc->DrawRoundedRectangle(x, y, w, h, r);

    double m = 1.5;
    wxGraphicsBrush body = gc->CreateLinearGradientBrush(
        x, y, x, y + h,
        baseColor, baseColor.ChangeLightness(60)
    );
    gc->SetBrush(body);
    gc->SetPen(wxPen(baseColor.ChangeLightness(40), 1));
    gc->DrawRoundedRectangle(x + m, y + m, w - (m * 2), h - (m * 2), r - 1);

    wxGraphicsPath gloss = gc->CreatePath();
    gloss.AddRoundedRectangle(x + m + 1, y + m + 1, w - (m * 2) - 2, (h / 2) - 1, r - 2);
    wxGraphicsBrush glossBrush = gc->CreateLinearGradientBrush(
        x, y, x, y + h / 2,
        wxColour(255, 255, 255, 160), wxColour(255, 255, 255, 20)
    );
    gc->SetBrush(glossBrush);
    gc->FillPath(gloss);

    gc->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD), isDarkText ? *wxBLACK : *wxWHITE);
    double tw, th;
    gc->GetTextExtent(label, &tw, &th);
    gc->DrawText(label, x + (w - tw) / 2.0, y + (h - th) / 2.0);
}

// ============================================================================
// IMPLEMENTACIÓN DE BookCardPanel (La Ficha del Libro)
// ============================================================================

wxBEGIN_EVENT_TABLE(BookCardPanel, wxPanel)
EVT_LEFT_DOWN(BookCardPanel::on_internal_card_click)
EVT_PAINT(BookCardPanel::on_paint)
wxEND_EVENT_TABLE()

BookCardPanel::BookCardPanel(
    wxWindow* parent,
    const LibroFicha& book_data,
    AppHandler* app_handler,
    std::function<void(int)> on_details_click,
    std::function<void(int)> on_read_click,
    std::function<void(int)> on_delete_click)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(CARD_WIDTH, CARD_HEIGHT), wxBORDER_NONE),
    m_book(book_data),
    m_app_handler(app_handler),
    m_on_details_click(on_details_click),
    m_on_read_click(on_read_click),
    m_on_delete_click(on_delete_click),
    m_is_active_style(false)
{
    ACTIVE_BG_COLOUR = wxColour(230, 245, 255);
    INACTIVE_BG_COLOUR = wxColour(255, 255, 255);

    this->SetBackgroundStyle(wxBG_STYLE_PAINT);

    bool loaded = false;
    if (!m_book.cover_image_data.empty())
    {
        wxMemoryInputStream stream(m_book.cover_image_data.data(), m_book.cover_image_data.size());
        wxImage img;
        if (img.LoadFile(stream, wxBITMAP_TYPE_ANY))
        {
            img.Rescale(IMAGE_WIDTH, IMAGE_HEIGHT, wxIMAGE_QUALITY_HIGH);
            m_cover_bitmap = wxBitmap(img);
            loaded = true;
        }
    }

    if (!loaded)
    {
        m_cover_bitmap = Util::CreatePlaceholderBitmap(IMAGE_WIDTH, IMAGE_HEIGHT, wxString::Format("Portada"));
    }

    set_active_style(false);
}

void BookCardPanel::set_active_style(bool is_active)
{
    if (m_is_active_style != is_active)
    {
        m_is_active_style = is_active;
        this->Refresh();
    }
}

void BookCardPanel::on_paint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

    if (!gc) return;

    wxSize sz = GetClientSize();

    // 1. Fondo Transparente
    gc->SetBrush(wxBrush(GetParent()->GetBackgroundColour()));
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(0, 0, sz.x, sz.y);

    // 2. Marco Principal
    int vis_h = sz.y - 18;
    wxColour frameStart = m_is_active_style ? wxColour(100, 200, 255) : wxColour(255, 215, 0);
    wxColour frameEnd = m_is_active_style ? wxColour(0, 100, 200) : wxColour(184, 134, 11);

    wxGraphicsBrush goldBrush = gc->CreateLinearGradientBrush(0, 0, sz.x, vis_h, frameStart, frameEnd);
    gc->SetBrush(goldBrush);
    gc->SetPen(wxPen(m_is_active_style ? wxColour(0, 50, 100) : wxColour(139, 101, 8), 1));
    gc->DrawRoundedRectangle(2, 2, sz.x - 4, vis_h - 4, 8);

    gc->SetBrush(wxBrush(m_is_active_style ? ACTIVE_BG_COLOUR : INACTIVE_BG_COLOUR));
    gc->SetPen(wxPen(wxColour(200, 200, 200), 1));
    gc->DrawRoundedRectangle(5, 5, sz.x - 10, vis_h - 10, 6);

    // 3. Portada
    double img_x = (sz.x - IMAGE_WIDTH) / 2.0;
    double img_y = 15.0;
    gc->DrawBitmap(m_cover_bitmap, img_x, img_y, IMAGE_WIDTH, IMAGE_HEIGHT);

    // 4. Título (BLINDAJE FORMAT)
    wxString title_text = wxString::FromUTF8(m_book.title);
    gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD), *wxBLACK);

    if (title_text.Length() > 20) {
        title_text = wxString::Format("%s...", title_text.Left(17));
    }

    double tw, th;
    gc->GetTextExtent(title_text, &tw, &th);
    gc->DrawText(title_text, (sz.x - tw) / 2.0, img_y + IMAGE_HEIGHT + 10);

    // 5. Autor (BLINDAJE FORMAT)
    wxString author_text = wxString::Format("por %s", wxString::FromUTF8(m_book.author));
    gc->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL), wxColour(80, 80, 80));

    if (author_text.Length() > 25) {
        author_text = wxString::Format("%s...", author_text.Left(22));
    }

    double aw, ah;
    gc->GetTextExtent(author_text, &aw, &ah);
    gc->DrawText(author_text, (sz.x - aw) / 2.0, img_y + IMAGE_HEIGHT + 28);

    // 6. BOTONES (Procedurales)
    double btn_w = 60; double btn_h = 24;
    double btn_y = vis_h - (btn_h / 2);
    double gap = 10;
    double start_x = (sz.x - (btn_w * 2) - gap) / 2.0;

    DrawLibraryGlossyComponent(gc.get(), start_x, btn_y, btn_w, btn_h, 6, wxColour(144, 238, 144), wxString::Format("Leer"), true);

    if (m_is_active_style) {
        DrawLibraryGlossyComponent(gc.get(), start_x + btn_w + gap, btn_y, btn_w, btn_h, 6, wxColour(255, 150, 150), wxString::Format("Borrar"), true);
    }
    else {
        DrawLibraryGlossyComponent(gc.get(), start_x + btn_w + gap, btn_y, btn_w, btn_h, 6, wxColour(173, 216, 230), wxString::Format("Detalles"), true);
    }
}

void BookCardPanel::on_internal_card_click(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    wxSize sz = GetClientSize();
    int vis_h = sz.y - 18;
    double btn_w = 60; double btn_h = 24;
    double btn_y = vis_h - (btn_h / 2);
    double gap = 10;
    double start_x = (sz.x - (btn_w * 2) - gap) / 2.0;

    if (pos.x >= start_x && pos.x <= start_x + btn_w && pos.y >= btn_y && pos.y <= btn_y + btn_h) {
        if (m_on_read_click) m_on_read_click(m_book.id);
    }
    else if (pos.x >= start_x + btn_w + gap && pos.x <= start_x + (btn_w * 2) + gap && pos.y >= btn_y && pos.y <= btn_y + btn_h) {
        if (m_is_active_style) { if (m_on_delete_click) m_on_delete_click(m_book.id); }
        else { if (m_on_details_click) m_on_details_click(m_book.id); }
    }
    else if (pos.y < vis_h) {
        if (m_on_details_click) m_on_details_click(m_book.id);
    }
    else { event.Skip(); }
}

// ============================================================================
// IMPLEMENTACIÓN DE LibraryView
// ============================================================================

LibraryView::LibraryView(wxWindow* parent, AppHandler* app_handler)
    : wxScrolled<wxPanel>(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
    m_app_handler(app_handler), m_current_is_sidebar_layout(false), m_sort_by_id(true)
{
    SetBackgroundColour(wxColour(240, 240, 240));
    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);

    m_btn_toggle_sort = new wxButton(this, wxID_ANY, wxString::Format("Ordenar: ID"));
    m_btn_toggle_sort->Bind(wxEVT_BUTTON, &LibraryView::_on_toggle_sort, this);
    m_main_vertical_sizer->Add(m_btn_toggle_sort, 0, wxALIGN_RIGHT | wxALL, 5);

    m_wrap_sizer = new wxWrapSizer(wxHORIZONTAL, wxREMOVE_LEADING_SPACES);
    m_box_sizer_vertical = new wxBoxSizer(wxVERTICAL);

    m_no_books_message_widget = _create_no_books_message_widget();
    m_no_books_message_widget->Show(false);

    m_main_vertical_sizer->Add(m_wrap_sizer, 1, wxEXPAND | wxALL, 0);
    this->SetSizer(m_main_vertical_sizer);
    this->SetScrollRate(0, 10);
}

LibraryView::~LibraryView() {}

wxPanel* LibraryView::_create_no_books_message_widget()
{
    wxPanel* panel = new wxPanel(this);
    wxStaticText* label = new wxStaticText(panel, wxID_ANY, wxString::Format("No hay libros disponibles."));
    label->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->AddStretchSpacer(1);
    sizer->Add(label, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    sizer->AddStretchSpacer(1);
    panel->SetSizer(sizer);
    return panel;
}

void LibraryView::_clear_and_destroy_book_cards()
{
    for (auto card : m_book_card_panels) { card->Destroy(); }
    m_book_card_panels.clear();
}

void LibraryView::set_on_book_card_selected_callback(std::function<void(int)> callback) { m_on_card_selected_callback = callback; }
void LibraryView::set_on_book_read_callback(std::function<void(int)> callback) { m_on_book_read_callback = callback; }

void LibraryView::_on_toggle_sort(wxCommandEvent& event)
{
    m_sort_by_id = !m_sort_by_id;
    m_btn_toggle_sort->SetLabel(wxString::Format("Ordenar: %s", m_sort_by_id ? "ID" : "A-Z"));
    load_books();
}

void LibraryView::_on_delete_book_requested(int book_id)
{
    auto book_details = m_app_handler->get_book_details(book_id);
    wxString book_title = wxString::Format("este libro");

    if (book_details && book_details->count("title")) {
        book_title = wxString::FromUTF8(std::get<std::string>(book_details->at("title")));
    }

    wxString msg = wxString::Format(
        "¿Está completamente seguro de que desea eliminar permanentemente '%s'?\n\n"
        "Esta acción borrará todos sus capítulos e ideas y no se puede deshacer.",
        book_title
    );

    wxMessageDialog dlg(this, msg, wxString::Format("Confirmar Eliminación"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);

    if (dlg.ShowModal() == wxID_YES) {
        if (m_app_handler->delete_book(book_id)) {
            m_app_handler->set_dirty(false);
            load_books();
            if (m_current_is_sidebar_layout && !m_book_card_panels.empty()) {
                if (m_on_card_selected_callback) m_on_card_selected_callback(m_book_card_panels[0]->get_book_id());
            }
        }
    }
}

void LibraryView::set_layout_mode(bool is_sidebar)
{
    if (m_current_is_sidebar_layout == is_sidebar) return;
    this->Freeze();
    _clear_and_destroy_book_cards();

    if (m_current_is_sidebar_layout) { m_main_vertical_sizer->Detach(m_box_sizer_vertical); m_box_sizer_vertical->Clear(false); }
    else { m_main_vertical_sizer->Detach(m_wrap_sizer); m_wrap_sizer->Clear(false); }

    m_current_is_sidebar_layout = is_sidebar;
    m_no_books_message_widget->Show(false);

    if (m_current_is_sidebar_layout) m_main_vertical_sizer->Add(m_box_sizer_vertical, 1, wxEXPAND | wxALL, 0);
    else m_main_vertical_sizer->Add(m_wrap_sizer, 1, wxEXPAND | wxALL, 0);

    load_books();
    this->Thaw(); this->Layout(); this->FitInside();
}

void LibraryView::load_books()
{
    this->Freeze();
    wxSizer* active_sizer = m_current_is_sidebar_layout ? (wxSizer*)m_box_sizer_vertical : (wxSizer*)m_wrap_sizer;
    _clear_and_destroy_book_cards();
    active_sizer->Detach(m_no_books_message_widget);
    m_no_books_message_widget->Show(false);

    std::vector<DBRow> raw_books = m_app_handler->get_all_books();

    if (raw_books.size() < 2 || m_current_is_sidebar_layout) m_btn_toggle_sort->Hide();
    else m_btn_toggle_sort->Show();

    if (raw_books.empty()) {
        m_no_books_message_widget->Show(true);
        wxWindowList& children = m_no_books_message_widget->GetChildren();
        if (!children.empty()) {
            if (auto label = dynamic_cast<wxStaticText*>(children.GetFirst()->GetData())) {
                label->SetLabel(wxString::Format("%s", m_current_is_sidebar_layout ? "No hay libros." : "No hay libros en la biblioteca."));
                m_no_books_message_widget->Layout();
            }
        }
        active_sizer->Add(m_no_books_message_widget, 1, wxEXPAND | wxALL, 10);
    }
    else {
        if (m_sort_by_id) {
            std::sort(raw_books.begin(), raw_books.end(), [](const DBRow& a, const DBRow& b) {
                return std::get<long long>(a.at("id")) < std::get<long long>(b.at("id"));
                });
        }
        else {
            std::sort(raw_books.begin(), raw_books.end(), [](const DBRow& a, const DBRow& b) {
                return std::get<std::string>(a.at("title")) < std::get<std::string>(b.at("title"));
                });
        }

        for (const auto& row : raw_books) {
            LibroFicha ficha;
            ficha.id = (int)std::get<long long>(row.at("id"));
            ficha.title = std::get<std::string>(row.at("title"));
            ficha.author = std::get<std::string>(row.at("author"));
            if (row.count("cover_image_data")) ficha.cover_image_data = std::get<std::vector<uint8_t>>(row.at("cover_image_data"));

            BookCardPanel* card = new BookCardPanel(this, ficha, m_app_handler, m_on_card_selected_callback, m_on_book_read_callback, [this](int id) { _on_delete_book_requested(id); });
            m_book_card_panels.push_back(card);

            if (m_current_is_sidebar_layout) active_sizer->Add(card, 0, wxEXPAND | wxALL, 5);
            else active_sizer->Add(card, 0, wxALL, 10);
        }
    }
    this->Thaw(); this->Layout(); this->FitInside(); this->Refresh();
}

void LibraryView::clear_view()
{
    this->Freeze();
    wxSizer* active_sizer = m_current_is_sidebar_layout ? (wxSizer*)m_box_sizer_vertical : (wxSizer*)m_wrap_sizer;
    _clear_and_destroy_book_cards();
    active_sizer->Detach(m_no_books_message_widget);
    m_no_books_message_widget->Show(false);
    this->Thaw(); this->Layout(); this->FitInside();
}