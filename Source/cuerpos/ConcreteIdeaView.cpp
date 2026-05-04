/**
* File Name: ConcreteIdeaView.cpp
* Descripción: Implementación del tablero de ideas con diseño de alta gama,
*              lógica de reordenamiento y blindaje estricto UTF-8 / wxString::Format.
*/

#include "../encabezados/ConcreteIdeaView.h"
#include "../encabezados/AppHandler.h"
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/tokenzr.h>

// ============================================================================
// AUXILIAR: FUNCIÓN DE RENDERIZADO GLOSSY 3D
// ============================================================================

void DrawGlossyComponent(
    wxGraphicsContext* gc,
    double x,
    double y,
    double w,
    double h,
    double r,
    wxColour baseColor,
    wxString label,
    bool isDarkText)
{
    // 1. MARCO METÁLICO (Bisel exterior)
    wxGraphicsBrush bezel = gc->CreateLinearGradientBrush(
        x, y,
        x + w, y + h,
        wxColour(255, 255, 255),
        wxColour(150, 150, 150)
    );
    gc->SetBrush(bezel);
    gc->SetPen(wxPen(wxColour(100, 100, 100), 1));
    gc->DrawRoundedRectangle(x, y, w, h, r);

    // 2. CUERPO DE CRISTAL (Color Base con profundidad)
    double m = 1.5;
    wxGraphicsBrush body = gc->CreateLinearGradientBrush(
        x, y,
        x, y + h,
        baseColor,
        baseColor.ChangeLightness(60)
    );
    gc->SetBrush(body);
    gc->SetPen(wxPen(baseColor.ChangeLightness(40), 1));
    gc->DrawRoundedRectangle(x + m, y + m, w - (m * 2), h - (m * 2), r - 1);

    // 3. BRILLO SUPERIOR (Efecto Glass)
    wxGraphicsPath gloss = gc->CreatePath();
    gloss.AddRoundedRectangle(x + m + 1, y + m + 1, w - (m * 2) - 2, (h / 2) - 1, r - 2);

    wxGraphicsBrush glossBrush = gc->CreateLinearGradientBrush(
        x, y,
        x, y + h / 2,
        wxColour(255, 255, 255, 160),
        wxColour(255, 255, 255, 20)
    );
    gc->SetBrush(glossBrush);
    gc->FillPath(gloss);

    // 4. ETIQUETA DE TEXTO
    gc->SetFont(
        wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD),
        isDarkText ? *wxBLACK : *wxWHITE
    );

    double tw, th;
    gc->GetTextExtent(label, &tw, &th);
    gc->DrawText(label, x + (w - tw) / 2, y + (h - th) / 2);
}

// ============================================================================
// CLASE 1: TARJETA DE IDEA CONCRETA (ConcreteIdeaCard)
// ============================================================================

ConcreteIdeaCard::ConcreteIdeaCard(
    wxWindow* parent,
    int id,
    const std::string& text,
    bool is_first,
    bool is_last,
    std::function<void(int, std::string)> on_read,
    std::function<void(int)> on_delete,
    std::function<void(int, bool)> on_move)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(195, 170), wxBORDER_NONE),
    m_id(id),
    m_text(text),
    m_is_first(is_first),
    m_is_last(is_last),
    m_on_read(on_read),
    m_on_delete(on_delete),
    m_on_move(on_move)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    Bind(wxEVT_PAINT, &ConcreteIdeaCard::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &ConcreteIdeaCard::OnLeftDown, this);
}

void ConcreteIdeaCard::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

    if (!gc)
    {
        return;
    }

    wxSize sz = GetClientSize();

    // A. FONDO MIMÉTICO (Transparencia con el tablero)
    gc->SetBrush(wxBrush(GetParent()->GetBackgroundColour()));
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(0, 0, sz.x, sz.y);

    // B. EL MARCO DORADO (Técnica de capas para borde de 3px)
    int vis_h = sz.y - 25;

    wxGraphicsBrush goldBrush = gc->CreateLinearGradientBrush(
        0, 0,
        sz.x, vis_h,
        wxColour(255, 215, 0),
        wxColour(184, 134, 11)
    );

    gc->SetBrush(goldBrush);
    gc->SetPen(wxPen(wxColour(139, 101, 8), 1));
    gc->DrawRoundedRectangle(2, 2, sz.x - 4, vis_h - 4, 8);

    gc->SetBrush(*wxWHITE_BRUSH);
    gc->SetPen(wxPen(wxColour(200, 200, 200), 1));
    gc->DrawRoundedRectangle(5, 5, sz.x - 10, vis_h - 10, 6);

    // C. LETRA GIGANTE (Marca de agua decorativa)
    if (!m_text.empty())
    {
        // BLINDAJE: Extraemos la inicial DESPUÉS de convertir a wxString. 
        // std::string.substr(0,1) rompe caracteres UTF-8 de 2 bytes (como Ñ o Tildes).
        wxString full_text_for_watermark = wxString::FromUTF8(m_text);
        wxString first = full_text_for_watermark.Left(1).Upper();

        gc->SetFont(
            wxFont(vis_h - 20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD),
            wxColour(250, 250, 250)
        );

        double tw, th;
        gc->GetTextExtent(first, &tw, &th);
        gc->DrawText(first, (sz.x - tw) / 2, (vis_h - th) / 2);
    }

    // D. ALGORITMO DE WORD WRAPPING (Inteligencia de texto)
    gc->SetFont(
        wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL),
        *wxBLACK
    );

    wxString full_text = wxString::FromUTF8(m_text);
    wxStringTokenizer tk(full_text, " \t\r\n");
    wxString currentLine = "";
    double y_cursor = 20;

    while (tk.HasMoreTokens())
    {
        wxString word = tk.GetNextToken();

        // BLINDAJE: Eliminada la concatenación (+) por un wxString::Format seguro.
        wxString testLine = currentLine.IsEmpty() ? word : wxString::Format("%s %s", currentLine, word);

        double tw, th;
        gc->GetTextExtent(testLine, &tw, &th);

        if (tw > sz.x - 30)
        {
            gc->DrawText(currentLine, 15, y_cursor);
            y_cursor += 14;
            currentLine = word;

            if (y_cursor > vis_h - 30)
            {
                currentLine = "...";
                break;
            }
        }
        else
        {
            currentLine = testLine;
        }
    }
    gc->DrawText(currentLine, 15, y_cursor);

    // E. BOTONES PROCEDURALES (Glossy Pastel)
    DrawGlossyComponent(gc.get(), 10, sz.y - 32, 55, 24, 6, wxColour(144, 238, 144), "Leer", true);
    DrawGlossyComponent(gc.get(), 70, sz.y - 32, 55, 24, 6, wxColour(173, 216, 230), "Borrar", true);

    // F. CONTADOR (Cristal Rojo - Límite 200)
    int count = (int)m_text.length();
    int display_count = count > 200 ? 200 : count;

    DrawGlossyComponent(
        gc.get(),
        sz.x - 55,
        sz.y - 32,
        45,
        24,
        6,
        wxColour(150, 0, 0),
        wxString::Format("%03d", display_count),
        false
    );

    // G. BOTONES DE MOVIMIENTO LATERAL (50% Desbordados)
    if (!m_is_first)
    {
        DrawGlossyComponent(gc.get(), -8, (vis_h / 2) - 15, 25, 30, 4, wxColour(200, 200, 200), "<", true);
    }

    if (!m_is_last)
    {
        DrawGlossyComponent(gc.get(), sz.x - 17, (vis_h / 2) - 15, 25, 30, 4, wxColour(200, 200, 200), ">", true);
    }
}

void ConcreteIdeaCard::OnLeftDown(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    wxSize sz = GetClientSize();
    int vh = sz.y - 25;

    // Clic en el botón "Leer"
    if (pos.x >= 10 && pos.x <= 65 && pos.y >= sz.y - 32 && pos.y <= sz.y - 8)
    {
        m_on_read(m_id, m_text);
    }
    // Clic en el botón "Borrar"
    else if (pos.x >= 70 && pos.x <= 125 && pos.y >= sz.y - 32 && pos.y <= sz.y - 8)
    {
        m_on_delete(m_id);
    }
    // Clic en la flecha izquierda
    else if (!m_is_first && pos.x >= -8 && pos.x <= 17 && pos.y >= (vh / 2) - 15 && pos.y <= (vh / 2) + 15)
    {
        m_on_move(m_id, false);
    }
    // Clic en la flecha derecha
    else if (!m_is_last && pos.x >= sz.x - 17 && pos.x <= sz.x + 8 && pos.y >= (vh / 2) - 15 && pos.y <= (vh / 2) + 15)
    {
        m_on_move(m_id, true);
    }
    else
    {
        event.Skip();
    }
}

// ============================================================================
// CLASE 2: DIÁLOGO DE EDICIÓN (IdeaDetailDialog)
// ============================================================================

IdeaDetailDialog::IdeaDetailDialog(wxWindow* parent, const std::string& initial_text)
    : wxDialog(parent, wxID_ANY, "Editar Idea", wxDefaultPosition, wxSize(400, 350))
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    m_text_ctrl = new wxTextCtrl(
        this, wxID_ANY,
        wxString::FromUTF8(initial_text),
        wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE
    );
    m_text_ctrl->SetMaxLength(200);

    main_sizer->Add(m_text_ctrl, 1, wxEXPAND | wxALL, 15);

    // Fila del contador
    wxBoxSizer* counter_sizer = new wxBoxSizer(wxHORIZONTAL);
    counter_sizer->AddStretchSpacer(1);

    // BLINDAJE: Uso estricto de Format para contadores
    wxStaticText* label_count = new wxStaticText(
        this, wxID_ANY,
        wxString::Format("%zu / 200", initial_text.length())
    );
    label_count->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD));
    label_count->SetForegroundColour(wxColour(100, 100, 100));

    counter_sizer->Add(label_count, 0, wxRIGHT, 20);
    main_sizer->Add(counter_sizer, 0, wxEXPAND);

    // Botonera inferior
    wxBoxSizer* btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    btn_sizer->Add(new wxButton(this, wxID_OK, "Guardar"), 0, wxRIGHT, 10);
    btn_sizer->Add(new wxButton(this, wxID_CANCEL, "Cerrar"), 0);

    main_sizer->Add(btn_sizer, 0, wxALIGN_CENTER | wxBOTTOM | wxTOP, 15);

    // Actualización dinámica del contador
    m_text_ctrl->Bind(wxEVT_TEXT, [label_count](wxCommandEvent& ev) {
        int len = (int)ev.GetString().Length();

        // BLINDAJE: Actualización por Format
        label_count->SetLabel(wxString::Format("%d / 200", len));

        if (len >= 200)
        {
            label_count->SetForegroundColour(*wxRED);
        }
        else
        {
            label_count->SetForegroundColour(wxColour(100, 100, 100));
        }
        });

    SetSizer(main_sizer);
    CentreOnParent();
}

std::string IdeaDetailDialog::GetText() const
{
    return std::string(m_text_ctrl->GetValue().ToUTF8().data());
}

// ============================================================================
// CLASE 3: TABLERO PRINCIPAL (ConcreteIdeaView)
// ============================================================================

ConcreteIdeaView::ConcreteIdeaView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent, wxID_ANY),
    m_app_handler(app_handler),
    m_chapter_id(std::nullopt)
{
    SetBackgroundColour(wxColour(245, 245, 245));
    _create_controls();
    _layout_controls();
    load_ideas(std::nullopt);
}

void ConcreteIdeaView::_create_controls()
{
    m_info_label = new wxStaticText(this, wxID_ANY, "Ideas del Capítulo:");

    m_add_button = new wxButton(this, wxID_ANY, "+ Nueva Idea (Máx 200)");
    m_add_button->Bind(wxEVT_BUTTON, &ConcreteIdeaView::OnAddIdea, this);

    m_wrap_sizer = new wxWrapSizer(wxHORIZONTAL, wxREMOVE_LEADING_SPACES);
}

void ConcreteIdeaView::_layout_controls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    main_sizer->Add(m_info_label, 0, wxALL, 10);
    main_sizer->Add(m_add_button, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    main_sizer->Add(m_wrap_sizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    this->SetSizer(main_sizer);
}

void ConcreteIdeaView::load_ideas(std::optional<int> id)
{
    this->Freeze();
    m_chapter_id = id;
    m_wrap_sizer->Clear(true);

    if (id.has_value())
    {
        auto ideas = m_app_handler->get_concrete_ideas_for_chapter(id.value());

        for (size_t i = 0; i < ideas.size(); ++i)
        {
            int iid = (int)std::get<long long>(ideas[i].at("id"));
            std::string txt = std::get<std::string>(ideas[i].at("idea"));

            // Calculamos IDs de los vecinos para la lógica de Swap
            int nid = (i < ideas.size() - 1) ? (int)std::get<long long>(ideas[i + 1].at("id")) : -1;
            int pid = (i > 0) ? (int)std::get<long long>(ideas[i - 1].at("id")) : -1;

            ConcreteIdeaCard* card = new ConcreteIdeaCard(
                this, iid, txt,
                (i == 0), (i == ideas.size() - 1),
                [this](int id_read, std::string text_read) {
                    OnCardReadClicked(id_read, text_read);
                },
                [this](int id_del) {
                    _on_delete_requested(id_del);
                },
                [this, pid, nid](int id_move, bool right) {
                    int target = right ? nid : pid;
                    if (target != -1 && m_app_handler->swap_concrete_idea_positions(id_move, target))
                    {
                        load_ideas(m_chapter_id);
                    }
                }
            );

            m_wrap_sizer->Add(card, 0, wxALL, 15);
        }
    }

    _update_button_states();
    this->Layout();
    this->Thaw();
}

void ConcreteIdeaView::_on_delete_requested(int id)
{
    wxMessageDialog dlg(this, "¿Borrar permanentemente esta nota?", "Aviso", wxYES_NO | wxICON_WARNING);

    if (dlg.ShowModal() == wxID_YES)
    {
        if (m_app_handler->delete_concrete_idea_by_id(id))
        {
            m_app_handler->set_dirty(true);
            load_ideas(m_chapter_id);
        }
    }
}

void ConcreteIdeaView::OnCardReadClicked(int id, std::string text)
{
    IdeaDetailDialog dlg(this, text);

    if (dlg.ShowModal() == wxID_OK)
    {
        // BLINDAJE NINJA: dlg.GetText() retorna std::string, LO CONVERTIMOS PREVIAMENTE A wxString
        // para evitar Segfault/Access Violation en el Format.
        wxString safe_utf8_string = wxString::FromUTF8(dlg.GetText());
        wxString secure_text = wxString::Format("%s", safe_utf8_string);

        // BLINDAJE: Garantizamos comparación byte a byte correcta volviendo a extraer el std::string UTF-8
        if (std::string(secure_text.ToUTF8().data()) != text)
        {
            if (m_app_handler->update_concrete_idea_text(id, secure_text))
            {
                m_app_handler->set_dirty(true);
                load_ideas(m_chapter_id);
            }
        }
    }
}

void ConcreteIdeaView::OnAddIdea(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "Escribe tu idea (Máx 200 caracteres):", "Nueva", "", wxOK | wxCANCEL | wxTE_MULTILINE);

    if (auto* t = dynamic_cast<wxTextCtrl*>(dlg.FindWindow(wxID_ANY)))
    {
        t->SetMaxLength(200);
    }

    if (dlg.ShowModal() == wxID_OK)
    {
        // BLINDAJE: dlg.GetValue() retorna wxString, seguro para Format
        wxString secure_text = wxString::Format("%s", dlg.GetValue());

        if (!secure_text.IsEmpty())
        {
            if (m_app_handler->add_concrete_idea_for_chapter(m_chapter_id.value(), secure_text))
            {
                m_app_handler->set_dirty(true);
                load_ideas(m_chapter_id);
            }
        }
    }
}

void ConcreteIdeaView::_update_button_states()
{
    m_add_button->Enable(this->IsEnabled() && m_chapter_id.has_value());
}

void ConcreteIdeaView::enable_view(bool enable)
{
    this->Enable(enable);
    _update_button_states();
}