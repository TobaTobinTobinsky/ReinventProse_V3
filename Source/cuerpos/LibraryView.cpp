/**
* File Name: LibraryView.cpp
* Descripción: Implementación de la vista de biblioteca (Catálogo de tarjetas).
*/

#include "../encabezados/LibraryView.h"
#include "../encabezados/AppHandler.h"
#include "../encabezados/Util.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

// --- IMPLEMENTACIÓN DE BookCardPanel ---

wxBEGIN_EVENT_TABLE(BookCardPanel, wxPanel)
EVT_LEFT_DOWN(BookCardPanel::on_internal_card_click)
EVT_PAINT(BookCardPanel::on_paint)
wxEND_EVENT_TABLE()

BookCardPanel::BookCardPanel(wxWindow* parent, const LibroFicha& book_data, AppHandler* app_handler, std::function<void(int)> on_click)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
    m_book(book_data), m_app_handler(app_handler), m_on_click(on_click), m_is_active_style(false)
{
    // Colores definidos dinámicamente
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

void BookCardPanel::_create_controls() {
    // Usamos el espacio de nombres Util:: correctamente
    std::optional<wxBitmap> img_opt = Util::LoadImage(wxString::FromUTF8(m_book.cover_image_path));

    if (img_opt.has_value() && img_opt->IsOk()) {
        wxImage image = img_opt->ConvertToImage().Rescale(IMAGE_WIDTH, IMAGE_HEIGHT, wxIMAGE_QUALITY_HIGH);
        m_cover_image_ctrl = new wxStaticBitmap(this, wxID_ANY, wxBitmap(image));
    }
    else {
        wxBitmap placeholder = Util::CreatePlaceholderBitmap(IMAGE_WIDTH, IMAGE_HEIGHT, "Portada");
        m_cover_image_ctrl = new wxStaticBitmap(this, wxID_ANY, placeholder);
    }

    m_title_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8(m_book.title),
        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    m_title_label->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    m_author_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("por " + m_book.author),
        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    m_author_label->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));

    // Todos los elementos son clicables para seleccionar la tarjeta
    m_cover_image_ctrl->Bind(wxEVT_LEFT_DOWN, &BookCardPanel::on_internal_card_click, this);
    m_title_label->Bind(wxEVT_LEFT_DOWN, &BookCardPanel::on_internal_card_click, this);
    m_author_label->Bind(wxEVT_LEFT_DOWN, &BookCardPanel::on_internal_card_click, this);
}

void BookCardPanel::_layout_controls() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_cover_image_ctrl, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    sizer->Add(m_title_label, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    sizer->Add(m_author_label, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    this->SetSizer(sizer);

    // Ajuste de texto
    m_title_label->Wrap(CARD_WIDTH - 10);
    m_author_label->Wrap(CARD_WIDTH - 10);
}

void BookCardPanel::on_internal_card_click(wxMouseEvent& event) {
    if (m_on_click) {
        m_on_click(m_book.id);
    }
    event.StopPropagation(); // Evita que se disparen eventos duplicados hacia el padre
}

void BookCardPanel::set_active_style(bool is_active) {
    if (m_is_active_style != is_active) {
        m_is_active_style = is_active;
        this->Refresh(); // Fuerza a que se dispare el evento Paint
    }
}

void BookCardPanel::on_paint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

    if (gc) {
        wxSize sz = GetClientSize();
        wxGraphicsPath path = gc->CreatePath();
        path.AddRoundedRectangle(0.5, 0.5, sz.x - 1, sz.y - 1, 3);

        if (m_is_active_style) {
            gc->SetBrush(wxBrush(ACTIVE_BG_COLOUR));
            gc->SetPen(wxPen(ACTIVE_BORDER_COLOUR, ACTIVE_BORDER_WIDTH));
        }
        else {
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
    m_app_handler(app_handler), m_current_is_sidebar_layout(false)
{
    SetBackgroundColour(wxColour(240, 240, 240));

    m_wrap_sizer = new wxWrapSizer(wxHORIZONTAL);
    m_box_sizer_vertical = new wxBoxSizer(wxVERTICAL);

    m_no_books_message_widget = _create_no_books_message_widget();
    m_no_books_message_widget->Show(false);

    // Por defecto arranca en vista central (wrap_sizer)
    this->SetSizer(m_wrap_sizer);

    // Método correcto en wxWidgets para hacer el panel Scrolled
    this->SetScrollRate(0, 10);
}

LibraryView::~LibraryView() {
    // Evitamos memory leaks eliminando el sizer que no está activo
    if (m_current_is_sidebar_layout) delete m_wrap_sizer;
    else delete m_box_sizer_vertical;
}

wxPanel* LibraryView::_create_no_books_message_widget() {
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

void LibraryView::_clear_and_destroy_book_cards() {
    for (auto card : m_book_card_panels) {
        card->Destroy();
    }
    m_book_card_panels.clear();
}

void LibraryView::set_on_book_card_selected_callback(std::function<void(int)> callback) {
    m_on_card_selected_callback = callback;
}

void LibraryView::set_layout_mode(bool is_sidebar) {
    if (m_current_is_sidebar_layout == is_sidebar) return;

    this->Freeze(); // Evita parpadeos mientras redibuja
    _clear_and_destroy_book_cards();

    wxSizer* old_sizer = this->GetSizer();
    wxSizer* new_sizer = is_sidebar ? (wxSizer*)m_box_sizer_vertical : (wxSizer*)m_wrap_sizer;

    if (old_sizer) old_sizer->Clear(false); // Clear false = no destruye las ventanas hijas
    new_sizer->Clear(false);

    m_current_is_sidebar_layout = is_sidebar;
    m_no_books_message_widget->Show(false);

    this->SetSizer(new_sizer, false); // False = no borra el sizer viejo de memoria
    load_books();

    this->Thaw();
    this->Layout();
    this->FitInside();
}

void LibraryView::load_books() {
    this->Freeze();

    wxSizer* active_sizer = this->GetSizer();
    if (!active_sizer) {
        this->Thaw();
        return;
    }

    _clear_and_destroy_book_cards();
    active_sizer->Detach(m_no_books_message_widget);
    m_no_books_message_widget->Show(false);

    // Obtener los datos usando la firma correcta del AppHandler
    std::vector<DBRow> raw_books = m_app_handler->get_all_books();

    if (raw_books.empty()) {
        m_no_books_message_widget->Show(true);
        wxWindowList& children = m_no_books_message_widget->GetChildren();
        if (!children.empty()) {
            if (auto label = dynamic_cast<wxStaticText*>(children.GetFirst()->GetData())) {
                if (m_current_is_sidebar_layout) {
                    label->SetLabel("No hay libros.");
                    label->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
                }
                else {
                    label->SetLabel("No hay libros en la biblioteca de ReinventProse 2.0.");
                    label->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
                }
                m_no_books_message_widget->Layout();
            }
        }
        active_sizer->Add(m_no_books_message_widget, 1, wxEXPAND | wxALL, 10);
    }
    else {
        // Crear una tarjeta por cada libro devuelto
        for (const auto& row : raw_books) {
            LibroFicha ficha;

            // Extracción segura (asumiendo que los valores obligatorios existen)
            ficha.id = (int)std::get<long long>(row.at("id"));
            ficha.title = std::get<std::string>(row.at("title"));
            ficha.author = std::get<std::string>(row.at("author"));

            // Cover path puede ser nulo o vacío
            if (row.count("cover_image_path") && std::holds_alternative<std::string>(row.at("cover_image_path"))) {
                ficha.cover_image_path = std::get<std::string>(row.at("cover_image_path"));
            }
            else {
                ficha.cover_image_path = "";
            }

            BookCardPanel* card = new BookCardPanel(this, ficha, m_app_handler, m_on_card_selected_callback);
            m_book_card_panels.push_back(card);

            // Añadir al sizer dependiendo del layout actual
            if (m_current_is_sidebar_layout) active_sizer->Add(card, 0, wxEXPAND | wxALL, 5);
            else active_sizer->Add(card, 0, wxALL, 10);
        }
    }

    this->Thaw();
    this->Layout();
    this->FitInside();
    this->Refresh();
}

void LibraryView::clear_view() {
    this->Freeze();
    wxSizer* active_sizer = this->GetSizer();
    if (active_sizer) {
        _clear_and_destroy_book_cards();
        active_sizer->Detach(m_no_books_message_widget);
        m_no_books_message_widget->Show(false);
        active_sizer->Layout();
    }
    this->Thaw();
    this->Layout();
    this->FitInside();
}