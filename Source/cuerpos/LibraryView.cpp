/**
* File Name: LibraryView.cpp
* Descripción: Implementación de la vista de biblioteca con lectura de BLOBs para portadas.
*/

#include "../encabezados/LibraryView.h"
#include "../encabezados/AppHandler.h"
#include "../encabezados/Util.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/msgdlg.h>
#include <wx/mstream.h>
#include <algorithm>

// --- IMPLEMENTACIÓN DE BookCardPanel ---

wxBEGIN_EVENT_TABLE(BookCardPanel, wxPanel)
EVT_LEFT_DOWN(BookCardPanel::on_internal_card_click)
EVT_PAINT(BookCardPanel::on_paint)
wxEND_EVENT_TABLE()

BookCardPanel::BookCardPanel(
    wxWindow* parent,
    const LibroFicha& book_data,
    AppHandler* app_handler,
    std::function<void(int)> on_click,
    std::function<void(int)> on_delete)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
    m_book(book_data), m_app_handler(app_handler), m_on_click(on_click), m_on_delete(on_delete), m_is_active_style(false)
{
    ACTIVE_BG_COLOUR = wxColour(220, 235, 255);
    INACTIVE_BG_COLOUR = wxColour(255, 255, 255);
    ACTIVE_BORDER_COLOUR = wxColour(0, 0, 128);
    INACTIVE_BORDER_COLOUR = wxColour(180, 180, 180);

    this->SetBackgroundStyle(wxBG_STYLE_PAINT);
    this->SetMinSize(wxSize(CARD_WIDTH, -1));

    _create_controls();
    _layout_controls();

    set_active_style(false);
}

void BookCardPanel::_create_controls()
{
    bool loaded = false;

    // LECTURA DESDE BLOB EN RAM
    if (!m_book.cover_image_data.empty())
    {
        wxMemoryInputStream stream(m_book.cover_image_data.data(), m_book.cover_image_data.size());
        wxImage img;
        if (img.LoadFile(stream, wxBITMAP_TYPE_ANY))
        {
            img.Rescale(IMAGE_WIDTH, IMAGE_HEIGHT, wxIMAGE_QUALITY_HIGH);
            m_cover_image_ctrl = new wxStaticBitmap(this, wxID_ANY, wxBitmap(img));
            loaded = true;
        }
    }

    if (!loaded)
    {
        wxBitmap placeholder = Util::CreatePlaceholderBitmap(IMAGE_WIDTH, IMAGE_HEIGHT, "Portada");
        m_cover_image_ctrl = new wxStaticBitmap(this, wxID_ANY, placeholder);
    }

    m_title_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8(m_book.title),
        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    m_title_label->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    m_author_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("por " + m_book.author),
        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    m_author_label->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));

    m_btn_edit = new wxButton(this, wxID_ANY, "Editar", wxDefaultPosition, wxSize(60, 25));
    m_btn_delete = new wxButton(this, wxID_ANY, "Borrar", wxDefaultPosition, wxSize(60, 25));

    m_btn_delete->SetForegroundColour(wxColour(200, 0, 0));

    m_cover_image_ctrl->Bind(wxEVT_LEFT_DOWN, &BookCardPanel::on_internal_card_click, this);
    m_title_label->Bind(wxEVT_LEFT_DOWN, &BookCardPanel::on_internal_card_click, this);
    m_author_label->Bind(wxEVT_LEFT_DOWN, &BookCardPanel::on_internal_card_click, this);

    m_btn_edit->Bind(wxEVT_BUTTON, &BookCardPanel::on_edit_btn_click, this);
    m_btn_delete->Bind(wxEVT_BUTTON, &BookCardPanel::on_delete_btn_click, this);
}

void BookCardPanel::_layout_controls()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_cover_image_ctrl, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    sizer->Add(m_title_label, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    sizer->Add(m_author_label, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    wxBoxSizer* btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    btn_sizer->Add(m_btn_edit, 0, wxRIGHT, 5);
    btn_sizer->Add(m_btn_delete, 0, 0, 0);

    sizer->Add(btn_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

    this->SetSizer(sizer);

    m_title_label->Wrap(CARD_WIDTH - 10);
    m_author_label->Wrap(CARD_WIDTH - 10);
}

void BookCardPanel::on_internal_card_click(wxMouseEvent& event)
{
    if (m_on_click) m_on_click(m_book.id);
    event.StopPropagation();
}

void BookCardPanel::on_edit_btn_click(wxCommandEvent& event)
{
    if (m_on_click) m_on_click(m_book.id);
}

void BookCardPanel::on_delete_btn_click(wxCommandEvent& event)
{
    if (m_on_delete) m_on_delete(m_book.id);
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

    if (gc)
    {
        wxSize sz = GetClientSize();
        wxGraphicsPath path = gc->CreatePath();
        path.AddRoundedRectangle(0.5, 0.5, sz.x - 1, sz.y - 1, 3);

        if (m_is_active_style)
        {
            gc->SetBrush(wxBrush(ACTIVE_BG_COLOUR));
            gc->SetPen(wxPen(ACTIVE_BORDER_COLOUR, ACTIVE_BORDER_WIDTH));
        }
        else
        {
            gc->SetBrush(wxBrush(INACTIVE_BG_COLOUR));
            gc->SetPen(wxPen(INACTIVE_BORDER_COLOUR, INACTIVE_BORDER_WIDTH));
        }

        gc->FillPath(path);
        gc->StrokePath(path);
    }
}

// --- IMPLEMENTACIÓN DE LibraryView ---

LibraryView::LibraryView(wxWindow* parent, AppHandler* app_handler)
    : wxScrolled<wxPanel>(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
    m_app_handler(app_handler), m_current_is_sidebar_layout(false), m_sort_by_id(true)
{
    SetBackgroundColour(wxColour(240, 240, 240));

    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);

    m_btn_toggle_sort = new wxButton(this, wxID_ANY, "Ordenar: ID");
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

LibraryView::~LibraryView()
{
    if (m_current_is_sidebar_layout) delete m_wrap_sizer;
    else delete m_box_sizer_vertical;
}

wxPanel* LibraryView::_create_no_books_message_widget()
{
    wxPanel* panel = new wxPanel(this);
    wxStaticText* label = new wxStaticText(panel, wxID_ANY, "Mensaje Temporal");
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
    for (auto card : m_book_card_panels)
    {
        card->Destroy();
    }
    m_book_card_panels.clear();
}

void LibraryView::set_on_book_card_selected_callback(std::function<void(int)> callback)
{
    m_on_card_selected_callback = callback;
}

void LibraryView::_on_toggle_sort(wxCommandEvent& event)
{
    m_sort_by_id = !m_sort_by_id;
    m_btn_toggle_sort->SetLabel(m_sort_by_id ? "Ordenar: ID" : "Ordenar: A-Z");
    load_books();
}

void LibraryView::_on_delete_book_requested(int book_id)
{
    auto book_details = m_app_handler->get_book_details(book_id);
    wxString book_title = "este libro";

    if (book_details && book_details->count("title"))
    {
        book_title = wxString::FromUTF8(std::get<std::string>(book_details->at("title")));
    }

    wxString msg = wxString::Format(
        "¿Está completamente seguro de que desea eliminar permanentemente '%s'?\n\n"
        "Esta acción borrará todos sus capítulos e ideas y no se puede deshacer.",
        book_title
    );

    wxMessageDialog dlg(this, msg, "Confirmar Eliminación", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);

    if (dlg.ShowModal() == wxID_YES)
    {
        if (m_app_handler->delete_book(book_id))
        {
            m_app_handler->set_dirty(false);
            load_books();

            if (m_current_is_sidebar_layout)
            {
                if (m_book_card_panels.empty())
                {
                    if (m_on_card_selected_callback)
                    {
                        m_on_card_selected_callback(book_id);
                    }
                }
                else
                {
                    int first_id = m_book_card_panels[0]->get_book_id();
                    if (m_on_card_selected_callback)
                    {
                        m_on_card_selected_callback(first_id);
                    }
                }
            }
        }
    }
}

void LibraryView::set_layout_mode(bool is_sidebar)
{
    if (m_current_is_sidebar_layout == is_sidebar) return;

    this->Freeze();
    _clear_and_destroy_book_cards();

    if (m_current_is_sidebar_layout)
    {
        m_main_vertical_sizer->Detach(m_box_sizer_vertical);
        m_box_sizer_vertical->Clear(false);
    }
    else
    {
        m_main_vertical_sizer->Detach(m_wrap_sizer);
        m_wrap_sizer->Clear(false);
    }

    m_current_is_sidebar_layout = is_sidebar;
    m_no_books_message_widget->Show(false);

    if (m_current_is_sidebar_layout)
    {
        m_main_vertical_sizer->Add(m_box_sizer_vertical, 1, wxEXPAND | wxALL, 0);
    }
    else
    {
        m_main_vertical_sizer->Add(m_wrap_sizer, 1, wxEXPAND | wxALL, 0);
    }

    load_books();

    this->Thaw();
    this->Layout();
    this->FitInside();
}

void LibraryView::load_books()
{
    this->Freeze();

    wxSizer* active_sizer = m_current_is_sidebar_layout ? (wxSizer*)m_box_sizer_vertical : (wxSizer*)m_wrap_sizer;

    _clear_and_destroy_book_cards();
    active_sizer->Detach(m_no_books_message_widget);
    m_no_books_message_widget->Show(false);

    std::vector<DBRow> raw_books = m_app_handler->get_all_books();

    if (raw_books.size() < 2 || m_current_is_sidebar_layout)
    {
        m_btn_toggle_sort->Hide();
    }
    else
    {
        m_btn_toggle_sort->Show();
    }

    if (raw_books.empty())
    {
        m_no_books_message_widget->Show(true);
        wxWindowList& children = m_no_books_message_widget->GetChildren();
        if (!children.empty())
        {
            if (auto label = dynamic_cast<wxStaticText*>(children.GetFirst()->GetData()))
            {
                if (m_current_is_sidebar_layout)
                {
                    label->SetLabel("No hay libros.");
                    label->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
                }
                else
                {
                    label->SetLabel("No hay libros en la biblioteca.");
                    label->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
                }
                m_no_books_message_widget->Layout();
            }
        }
        active_sizer->Add(m_no_books_message_widget, 1, wxEXPAND | wxALL, 10);
    }
    else
    {
        if (m_sort_by_id)
        {
            std::sort(raw_books.begin(), raw_books.end(), [](const DBRow& a, const DBRow& b) {
                return std::get<long long>(a.at("id")) < std::get<long long>(b.at("id"));
                });
        }
        else
        {
            std::sort(raw_books.begin(), raw_books.end(), [](const DBRow& a, const DBRow& b) {
                return std::get<std::string>(a.at("title")) < std::get<std::string>(b.at("title"));
                });
        }

        for (const auto& row : raw_books)
        {
            LibroFicha ficha;
            ficha.id = (int)std::get<long long>(row.at("id"));
            ficha.title = std::get<std::string>(row.at("title"));
            ficha.author = std::get<std::string>(row.at("author"));

            // ASIGNACIÓN DEL BLOB
            if (row.count("cover_image_data") && std::holds_alternative<std::vector<uint8_t>>(row.at("cover_image_data")))
            {
                ficha.cover_image_data = std::get<std::vector<uint8_t>>(row.at("cover_image_data"));
            }

            BookCardPanel* card = new BookCardPanel(
                this,
                ficha,
                m_app_handler,
                m_on_card_selected_callback,
                [this](int id) { _on_delete_book_requested(id); }
            );

            m_book_card_panels.push_back(card);

            if (m_current_is_sidebar_layout)
            {
                active_sizer->Add(card, 0, wxEXPAND | wxALL, 5);
            }
            else
            {
                active_sizer->Add(card, 0, wxALL, 10);
            }
        }
    }

    this->Thaw();
    this->Layout();
    this->FitInside();
    this->Refresh();
}

void LibraryView::clear_view()
{
    this->Freeze();
    wxSizer* active_sizer = m_current_is_sidebar_layout ? (wxSizer*)m_box_sizer_vertical : (wxSizer*)m_wrap_sizer;

    _clear_and_destroy_book_cards();
    active_sizer->Detach(m_no_books_message_widget);
    m_no_books_message_widget->Show(false);
    active_sizer->Layout();

    this->Thaw();
    this->Layout();
    this->FitInside();
}