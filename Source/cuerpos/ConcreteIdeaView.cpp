/**
* File Name: ConcreteIdeaView.cpp
* Descripción: Implementación del tablero de ideas (Tarjetas interactivas con UTF-8).
*/

#include "../encabezados/ConcreteIdeaView.h"
#include "../encabezados/AppHandler.h"
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

// ============================================================================
// CLASE 1: TARJETA DE IDEA CONCRETA (El Post-It)
// ============================================================================

ConcreteIdeaCard::ConcreteIdeaCard(wxWindow* parent, int id, const std::string& text, std::function<void(int, std::string)> on_click)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(200, 120), wxBORDER_NONE),
    m_id(id), m_text(text), m_on_click(on_click)
{
    // Fundamental para evitar parpadeos y permitir dibujos vectoriales limpios
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    Bind(wxEVT_PAINT, &ConcreteIdeaCard::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &ConcreteIdeaCard::OnLeftDown, this);
}

void ConcreteIdeaCard::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

    if (!gc) return;

    wxSize sz = GetClientSize();

    // Dibujo del fondo (Amarillo Pastel para simular post-it)
    gc->SetBrush(wxBrush(wxColour(255, 255, 180)));
    gc->SetPen(wxPen(wxColour(200, 200, 100), 1));
    gc->DrawRoundedRectangle(2, 2, sz.x - 4, sz.y - 4, 6); // Bordes ligeramente redondeados

    // Dibujo del texto con soporte UTF-8
    gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), *wxBLACK);
    wxString display_text = wxString::FromUTF8(m_text);

    // Si la idea es muy larga, la truncamos visualmente en la tarjeta
    if (display_text.Length() > 90) {
        display_text = display_text.Left(87) + "...";
    }

    // Dibujamos el texto con margen interno
    gc->DrawText(display_text, 10, 10, sz.x - 20);
}

void ConcreteIdeaCard::OnLeftDown(wxMouseEvent& event) {
    if (m_on_click) m_on_click(m_id, m_text);
}

// ============================================================================
// CLASE 2: DIÁLOGO EMERGENTE DE EDICIÓN (La Ventanita)
// ============================================================================

IdeaDetailDialog::IdeaDetailDialog(wxWindow* parent, const std::string& initial_text)
    : wxDialog(parent, wxID_ANY, "Detalle de Idea Concreta", wxDefaultPosition, wxSize(400, 300)),
    m_deleted(false)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // TextCtrl multilínea donde se muestra la idea completa
    m_text_ctrl = new wxTextCtrl(this, wxID_ANY, wxString::FromUTF8(initial_text),
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);

    // Configuración visual: fuente un poco más grande
    m_text_ctrl->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    main_sizer->Add(m_text_ctrl, 1, wxEXPAND | wxALL, 15);

    // Botones de acción
    wxBoxSizer* btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* save_btn = new wxButton(this, wxID_OK, "Guardar Cambios");
    wxButton* del_btn = new wxButton(this, wxID_ANY, "Eliminar Idea");
    wxButton* cancel_btn = new wxButton(this, wxID_CANCEL, "Cerrar");

    // Botón eliminar lleva lógica especial
    del_btn->Bind(wxEVT_BUTTON, &IdeaDetailDialog::OnDelete, this);

    btn_sizer->Add(save_btn, 0, wxRIGHT, 10);
    btn_sizer->Add(del_btn, 0, wxRIGHT, 10);
    btn_sizer->Add(cancel_btn, 0);

    main_sizer->Add(btn_sizer, 0, wxALIGN_CENTER | wxBOTTOM, 15);
    SetSizer(main_sizer);
}

void IdeaDetailDialog::OnDelete(wxCommandEvent& event) {
    // Doble check antes de volar la idea
    if (wxMessageBox("¿Está seguro que desea eliminar permanentemente esta idea?", "Confirmar Eliminación", wxYES_NO | wxICON_WARNING | wxNO_DEFAULT) == wxYES) {
        m_deleted = true;
        EndModal(wxID_OK); // Salimos del diálogo marcando que se aceptó la acción
    }
}

std::string IdeaDetailDialog::GetText() const {
    // Aseguramos devolver el texto en UTF-8 para la base de datos
    return m_text_ctrl->GetValue().ToUTF8().data();
}

// ============================================================================
// CLASE 3: TABLERO PRINCIPAL (ConcreteIdeaView)
// ============================================================================

ConcreteIdeaView::ConcreteIdeaView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent, wxID_ANY), m_app_handler(app_handler), m_chapter_id(std::nullopt)
{
    SetBackgroundColour(wxColour(245, 245, 245)); // Gris clarito de fondo de tablero
    _create_controls();
    _layout_controls();
    load_ideas(std::nullopt);
}

void ConcreteIdeaView::_create_controls() {
    m_info_label = new wxStaticText(this, wxID_ANY, "Tablero de Ideas Concretas:");
    m_info_label->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD));

    m_add_button = new wxButton(this, wxID_ANY, "+ Crear Nueva Idea");
    m_add_button->Bind(wxEVT_BUTTON, &ConcreteIdeaView::OnAddIdea, this);

    // Este Sizer es la clave: Acomoda los elementos y hace salto de línea si no caben (Wrap)
    m_wrap_sizer = new wxWrapSizer(wxHORIZONTAL);
}

void ConcreteIdeaView::_layout_controls() {
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    main_sizer->Add(m_info_label, 0, wxALL, 10);
    main_sizer->Add(m_add_button, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Agregamos el wrap sizer al panel principal
    main_sizer->Add(m_wrap_sizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    this->SetSizer(main_sizer);
}

void ConcreteIdeaView::_update_button_states() {
    // Solo permitimos agregar ideas si hay un capítulo enfocado
    bool can_add = this->IsEnabled() && m_chapter_id.has_value();
    m_add_button->Enable(can_add);
}

void ConcreteIdeaView::load_ideas(std::optional<int> id) {
    // Evitamos parpadeos mientras redibujamos el tablero
    this->Freeze();

    m_chapter_id = id;

    // Destruimos todas las tarjetas viejas
    m_wrap_sizer->Clear(true);

    if (id.has_value()) {
        m_info_label->SetLabel("Tablero de Ideas - Haz clic para editar");

        // Consultamos al motor de base de datos
        auto ideas = m_app_handler->get_concrete_ideas_for_chapter(id.value());
        for (const auto& row : ideas) {
            int idea_id = (int)std::get<long long>(row.at("id"));
            std::string text = std::get<std::string>(row.at("idea"));

            // Instanciamos el post-it. Le pasamos un callback lambda que se dispara al hacerle clic.
            ConcreteIdeaCard* card = new ConcreteIdeaCard(this, idea_id, text,
                [this](int iid, std::string itxt) { this->OnCardClicked(iid, itxt); });

            // Agregamos la tarjeta al WrapSizer con 5px de separación
            m_wrap_sizer->Add(card, 0, wxALL, 5);
        }
    }
    else {
        m_info_label->SetLabel("Seleccione un capítulo para ver o crear sus ideas concretas.");
    }

    _update_button_states();

    this->Layout();
    this->Thaw(); // Aplicamos los cambios visuales
}

void ConcreteIdeaView::OnAddIdea(wxCommandEvent& event) {
    if (!m_chapter_id.has_value() || !this->IsEnabled()) return;

    // Abrimos un diálogo estándar, con propiedad multilínea
    wxTextEntryDialog dlg(this, "Escribe tu idea (sé concreto pero no escatimes renglones):", "Nueva Idea", "", wxOK | wxCANCEL | wxTE_MULTILINE);

    if (dlg.ShowModal() == wxID_OK) {
        wxString text = dlg.GetValue().Trim(true).Trim(false);
        if (!text.IsEmpty()) {

            // Convertir de wxString (interfaz) a std::string UTF-8 (Base de Datos)
            std::string text_utf8 = text.ToUTF8().data();

            if (m_app_handler->add_concrete_idea_for_chapter(m_chapter_id.value(), wxString::FromUTF8(text_utf8))) {
                load_ideas(m_chapter_id); // Refrescar el tablero para que aparezca la nueva
                m_app_handler->set_dirty(true); // Prender asterisco
            }
            else {
                wxMessageBox("Hubo un fallo al intentar registrar la idea en la base de datos.", "Error de Motor SQL", wxOK | wxICON_ERROR);
            }
        }
    }
}

void ConcreteIdeaView::OnCardClicked(int id, std::string text) {
    // Invocamos nuestra "Ventanita" emergente
    IdeaDetailDialog dlg(this, text);

    if (dlg.ShowModal() == wxID_OK) {
        // ¿El usuario presionó el botón rojo de Eliminar?
        if (dlg.IsDeleted()) {
            if (m_app_handler->delete_concrete_idea_by_id(id)) {
                m_app_handler->set_dirty(true);
                load_ideas(m_chapter_id); // Volver a pintar tablero
            }
        }
        // Si no, asumimos que presionó Guardar. Vemos si el texto cambió.
        else {
            std::string new_text = dlg.GetText();
            if (new_text != text) {
                // Hay actualización
                if (m_app_handler->update_concrete_idea_text(id, wxString::FromUTF8(new_text))) {
                    m_app_handler->set_dirty(true);
                    load_ideas(m_chapter_id); // Volver a pintar tablero
                }
            }
        }
    }
}

void ConcreteIdeaView::enable_view(bool enable) {
    this->Enable(enable);
    _update_button_states();
}