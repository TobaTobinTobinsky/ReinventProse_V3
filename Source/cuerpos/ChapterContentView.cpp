/**
* Archivo: ChapterContentView.cpp
* Descripción: Implementación del editor con interruptor animado, contador de palabras y lógica de protección UTF-8.
*/

#include "../encabezados/ChapterContentView.h"
#include "../encabezados/AppHandler.h"
#include "../encabezados/Util.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/tokenzr.h> // Necesario para contar palabras

// ============================================================================
// IMPLEMENTACIÓN: ModernToggleSwitch (Interruptor de Hardware Virtual)
// ============================================================================

wxBEGIN_EVENT_TABLE(ModernToggleSwitch, wxControl)
EVT_PAINT(ModernToggleSwitch::OnPaint)
EVT_LEFT_DOWN(ModernToggleSwitch::OnLeftDown)
EVT_TIMER(wxID_ANY, ModernToggleSwitch::OnTimer)
wxEND_EVENT_TABLE()

ModernToggleSwitch::ModernToggleSwitch(wxWindow* parent, wxWindowID id)
    : wxControl(parent, id, wxDefaultPosition, wxSize(55, 30), wxBORDER_NONE),
    m_isOn(false), m_progress(0.0)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_timer.SetOwner(this);
}

void ModernToggleSwitch::SetState(bool on)
{
    if (m_isOn == on) return;
    m_isOn = on;
    m_timer.Start(16); // ~60 FPS para suavidad total
}

void ModernToggleSwitch::OnTimer(wxTimerEvent& event)
{
    double step = 0.08; // Velocidad para llegar en ~0.5s

    if (m_isOn)
    {
        m_progress += step;
        if (m_progress >= 1.0)
        {
            m_progress = 1.0;
            m_timer.Stop();
        }
    }
    else
    {
        m_progress -= step;
        if (m_progress <= 0.0)
        {
            m_progress = 0.0;
            m_timer.Stop();
        }
    }
    Refresh();
}

void ModernToggleSwitch::OnLeftDown(wxMouseEvent& event)
{
    if (!IsEnabled()) return;
    SetState(!m_isOn);

    // Notificamos al padre que el estado cambió
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
    ProcessEvent(evt);
}

void ModernToggleSwitch::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (!gc) return;

    // Fondo del control (mimetismo con el panel)
    gc->SetBrush(wxBrush(GetParent()->GetBackgroundColour()));
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(0, 0, GetSize().x, GetSize().y);

    // 1. DIBUJAR LA PASTILLA (Fondo del interruptor)
    // Interpolación de color: Rojo (Off) a Verde (On)
    unsigned char r = (unsigned char)(200 * (1.0 - m_progress));
    unsigned char g = (unsigned char)(180 * m_progress);
    wxColour bgColor(r, g, 40);

    gc->SetBrush(wxBrush(bgColor));
    gc->SetPen(wxPen(wxColour(100, 100, 100), 1));
    gc->DrawRoundedRectangle(2, 2, m_width, m_height, m_height / 2);

    // 2. DIBUJAR EL INTERRUPTOR (Círculo)
    double start_x = 4 + (m_height / 2);
    double end_x = m_width - (m_height / 2);
    double current_x = start_x + (m_progress * (end_x - start_x));

    // Efecto 3D para el círculo
    gc->SetBrush(wxBrush(wxColour(255, 255, 255)));
    gc->SetPen(wxPen(wxColour(50, 50, 50), 1));
    gc->DrawEllipse(current_x - (m_height / 2) + 2, 4, m_height - 4, m_height - 4);
}

// ============================================================================
// IMPLEMENTACIÓN: ChapterContentView
// ============================================================================

wxBEGIN_EVENT_TABLE(ChapterContentView, wxPanel)
EVT_TEXT(wxID_ANY, ChapterContentView::on_text_changed)
EVT_BUTTON(ID_TOGGLE_SWITCH, ChapterContentView::on_toggle_switch)
wxEND_EVENT_TABLE()

ChapterContentView::ChapterContentView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent), app_handler(app_handler), chapter_id(std::nullopt),
    _is_dirty_view(false), _loading_data(false), _is_in_edit_mode(false)
{
    _create_controls();
    _layout_controls();
    load_content(std::nullopt);
}

void ChapterContentView::_create_controls()
{
    content_label = new wxStaticText(this, wxID_ANY, "Contenido del Capítulo:");

    // Etiqueta del contador de palabras con diseño estilizado
    m_word_count_label = new wxStaticText(this, wxID_ANY, "Palabras: 0");
    m_word_count_label->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD));
    m_word_count_label->SetForegroundColour(wxColour(100, 100, 100));

    content_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxTE_RICH2);

    wxFont writingFont(12, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    content_ctrl->SetFont(writingFont);
    content_ctrl->SetEditable(false);

    // Creamos el interruptor moderno
    m_toggle = new ModernToggleSwitch(this, ID_TOGGLE_SWITCH);
    m_toggle_label = new wxStaticText(this, wxID_ANY, "Modo Edición: Off");
    m_toggle_label->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
}

void ChapterContentView::_layout_controls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Área superior del interruptor
    wxBoxSizer* toggle_container = new wxBoxSizer(wxVERTICAL);
    toggle_container->Add(m_toggle, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 10);
    toggle_container->Add(m_toggle_label, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, 5);

    // Área de etiquetas (Título a la izquierda, Contador a la derecha)
    wxBoxSizer* labels_sizer = new wxBoxSizer(wxHORIZONTAL);
    labels_sizer->Add(content_label, 0, wxALIGN_CENTER_VERTICAL);
    labels_sizer->AddStretchSpacer(1); // Empuja el contador hacia la derecha
    labels_sizer->Add(m_word_count_label, 0, wxALIGN_CENTER_VERTICAL);

    main_sizer->Add(toggle_container, 0, wxEXPAND | wxBOTTOM, 10);
    main_sizer->Add(labels_sizer, 0, wxALL | wxEXPAND, 5);
    main_sizer->Add(content_ctrl, 1, wxEXPAND | wxALL, 5);

    this->SetSizer(main_sizer);
}

void ChapterContentView::_update_word_count()
{
    if (content_ctrl == nullptr || m_word_count_label == nullptr)
    {
        return;
    }

    wxString current_text = content_ctrl->GetValue();

    // Usamos el tokenizador para separar por espacios, saltos de línea y tabulaciones
    wxStringTokenizer tokenizer(current_text, " \t\r\n");
    int count = tokenizer.CountTokens();

    m_word_count_label->SetLabel(wxString::Format("Palabras: %d", count));

    // Forzamos actualización visual rápida del layout por si los números crecen
    this->Layout();
}

void ChapterContentView::load_content(std::optional<int> id)
{
    _loading_data = true;
    chapter_id = id;
    content_ctrl->Clear();

    if (chapter_id.has_value())
    {
        content_label->SetLabel("Editor de Prosa:");
        auto details_opt = app_handler->get_chapter_details(chapter_id.value());
        if (details_opt.has_value())
        {
            DBRow details = details_opt.value();
            if (details.count("content"))
            {
                wxString texto = wxString::FromUTF8(std::get<std::string>(details["content"]));
                content_ctrl->SetValue(texto);
            }
        }
    }
    else
    {
        content_label->SetLabel("Contenido: (Seleccione un capítulo)");
    }

    // Actualizamos el contador recién cargamos el contenido
    _update_word_count();

    content_ctrl->SetInsertionPoint(0);
    _is_dirty_view = false;
    _is_in_edit_mode = false;
    m_toggle->SetState(false);
    _update_edit_mode_ui();
    _loading_data = false;
}

bool ChapterContentView::save_changes()
{
    if (!_is_dirty_view || !chapter_id.has_value()) return true;

    // BLINDAJE FORMAT (La Técnica del Jefe)
    wxString textoSeguro = wxString::Format("%s", content_ctrl->GetValue());

    bool success = app_handler->update_chapter_content_via_handler(
        chapter_id.value(),
        textoSeguro
    );

    if (success) _is_dirty_view = false;
    return success;
}

void ChapterContentView::on_toggle_switch(wxCommandEvent& event)
{
    if (!chapter_id.has_value())
    {
        m_toggle->SetState(false);
        return;
    }

    _is_in_edit_mode = m_toggle->IsOn();

    if (!_is_in_edit_mode && _is_dirty_view)
    {
        save_changes();
    }

    _update_edit_mode_ui();
}

void ChapterContentView::_update_edit_mode_ui()
{
    bool can_edit = _is_in_edit_mode && chapter_id.has_value();

    content_ctrl->SetEditable(can_edit);

    // Actualizar el texto del label
    m_toggle_label->SetLabel(can_edit ? "Modo Edición: On" : "Modo Edición: Off");
    m_toggle_label->SetForegroundColour(can_edit ? wxColour(0, 150, 0) : wxColour(150, 0, 0));

    if (can_edit)
    {
        content_ctrl->SetBackgroundColour(wxColour(255, 255, 240));
        content_ctrl->SetFocus();
    }
    else
    {
        content_ctrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    }

    this->Layout();
    content_ctrl->Refresh();
}

void ChapterContentView::on_text_changed(wxCommandEvent& event)
{
    // El contador se actualiza siempre, sin importar el modo
    _update_word_count();

    if (!_loading_data && _is_in_edit_mode)
    {
        set_view_dirty(true);
    }

    event.Skip();
}

void ChapterContentView::set_view_dirty(bool is_dirty)
{
    if (_is_dirty_view != is_dirty)
    {
        _is_dirty_view = is_dirty;
        if (_is_dirty_view) app_handler->set_dirty(true);
    }
}

void ChapterContentView::enable_view(bool enable) { this->Enable(enable); }
bool ChapterContentView::is_dirty() const { return _is_dirty_view; }
bool ChapterContentView::is_editable() const { return _is_in_edit_mode; }
bool ChapterContentView::force_save_if_dirty() { return save_changes(); }