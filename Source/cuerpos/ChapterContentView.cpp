/**
* Archivo: ChapterContentView.cpp
* Descripción: Implementación del panel de prosa.
*              CORRECCIÓN CRÍTICA: Control de errores estricto en el guardado a base de datos.
* Autor: AutoDoc AI (Protocolo de Estabilidad C++20)
* Versión: 4.1.0
*/

#include "../encabezados/ChapterContentView.h"
#include "../encabezados/AppHandler.h"
#include "../encabezados/Util.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/tokenzr.h>
#include <wx/msgdlg.h> // REQUERIDO PARA REPORTAR FALLOS

// ============================================================================
// CLASE AUXILIAR: ModernToggleSwitch
// ============================================================================
class ModernToggleSwitch : public wxControl
{
public:
    ModernToggleSwitch(wxWindow* parent, wxWindowID id)
        : wxControl(parent, id, wxDefaultPosition, wxSize(55, 30), wxBORDER_NONE),
        m_isOn(false), m_progress(0.0)
    {
        this->SetBackgroundStyle(wxBG_STYLE_PAINT);
        m_timer.SetOwner(this);
        this->Bind(wxEVT_PAINT, &ModernToggleSwitch::OnPaint, this);
        this->Bind(wxEVT_LEFT_DOWN, &ModernToggleSwitch::OnLeftDown, this);
        this->Bind(wxEVT_TIMER, &ModernToggleSwitch::OnTimer, this);
    }
    bool IsOn() const { return m_isOn; }
    void SetState(bool on) { if (m_isOn == on) return; m_isOn = on; m_timer.Start(16); }

private:
    void OnTimer(wxTimerEvent& event) {
        double step = 0.1;
        if (m_isOn) { m_progress += step; if (m_progress >= 1.0) { m_progress = 1.0; m_timer.Stop(); } }
        else { m_progress -= step; if (m_progress <= 0.0) { m_progress = 0.0; m_timer.Stop(); } }
        this->Refresh();
    }
    void OnLeftDown(wxMouseEvent& event) {
        if (!this->IsEnabled()) return;
        this->SetState(!m_isOn);
        wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
        evt.SetEventObject(this);
        this->ProcessEvent(evt);
    }
    void OnPaint(wxPaintEvent& event) {
        wxAutoBufferedPaintDC dc(this);
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
        if (!gc) return;
        gc->SetBrush(wxBrush(GetParent()->GetBackgroundColour()));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRectangle(0, 0, GetSize().x, GetSize().y);

        unsigned char r = (unsigned char)(200 * (1.0 - m_progress));
        unsigned char g = (unsigned char)(180 * m_progress);
        wxColour bgColor(r, g, 40);

        gc->SetBrush(wxBrush(bgColor));
        gc->SetPen(wxPen(wxColour(100, 100, 100), 1));
        gc->DrawRoundedRectangle(2, 2, 50, 26, 13);

        double start_x = 15; double end_x = 37;
        double current_x = start_x + (m_progress * (end_x - start_x));

        gc->SetBrush(*wxWHITE_BRUSH);
        gc->SetPen(wxPen(wxColour(50, 50, 50), 1));
        gc->DrawEllipse(current_x - 11, 4, 22, 22);
    }
    wxTimer m_timer; bool m_isOn; double m_progress;
};

// ============================================================================
// IMPLEMENTACIÓN: ChapterContentView
// ============================================================================

wxBEGIN_EVENT_TABLE(ChapterContentView, wxPanel)
EVT_BUTTON(ID_TOGGLE_SWITCH, ChapterContentView::on_toggle_switch)
wxEND_EVENT_TABLE()

ChapterContentView::ChapterContentView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent), app_handler(app_handler), chapter_id(std::nullopt),
    _is_dirty_view(false), _loading_data(false), _is_in_edit_mode(false)
{
    this->_create_controls();
    this->_layout_controls();
    m_visual_editor->Bind(wxEVT_TEXT, &ChapterContentView::on_text_changed, this);
    this->load_content(std::nullopt);
}

void ChapterContentView::_create_controls()
{
    wxString prose_lab = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Estructura de Prosa Visual:")));
    content_label = new wxStaticText(this, wxID_ANY, prose_lab);

    wxString word_lab = wxString::Format("%s: 0", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Palabras")));
    m_word_count_label = new wxStaticText(this, wxID_ANY, word_lab);
    m_word_count_label->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD));
    m_word_count_label->SetForegroundColour(wxColour(100, 100, 100));

    m_visual_editor = new StructuredEditor(this, ID_VISUAL_EDITOR_CTRL);
    m_visual_editor->SetEditable(false);

    m_toggle = new ModernToggleSwitch(this, ID_TOGGLE_SWITCH);

    wxString mode_lab = wxString::Format("%s: Off", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Modo Edición")));
    m_toggle_label = new wxStaticText(this, wxID_ANY, mode_lab);
    m_toggle_label->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
}

void ChapterContentView::_layout_controls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* toggle_container = new wxBoxSizer(wxVERTICAL);
    toggle_container->Add(m_toggle, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 10);
    toggle_container->Add(m_toggle_label, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, 5);

    wxBoxSizer* header_sizer = new wxBoxSizer(wxHORIZONTAL);
    header_sizer->Add(content_label, 0, wxALIGN_CENTER_VERTICAL);
    header_sizer->AddStretchSpacer(1);
    header_sizer->Add(m_word_count_label, 0, wxALIGN_CENTER_VERTICAL);

    main_sizer->Add(toggle_container, 0, wxEXPAND | wxBOTTOM, 10);
    main_sizer->Add(header_sizer, 0, wxALL | wxEXPAND, 5);
    main_sizer->Add(m_visual_editor, 1, wxEXPAND | wxALL, 5);

    this->SetSizer(main_sizer);
}

void ChapterContentView::load_content(std::optional<int> id)
{
    _loading_data = true;
    chapter_id = id;

    if (chapter_id.has_value())
    {
        auto details_opt = app_handler->get_chapter_details(chapter_id.value());
        if (details_opt.has_value())
        {
            DBRow details = details_opt.value();
            if (details.count("content"))
            {
                std::string raw_data = std::get<std::string>(details["content"]);
                wxString safe_content = wxString::Format("%s", wxString::FromUTF8(raw_data.c_str()));
                m_visual_editor->SetText(safe_content);
            }
        }
    }
    else
    {
        m_visual_editor->SetText(wxEmptyString);
    }

    this->_update_word_count();

    _is_dirty_view = false;
    _is_in_edit_mode = false;
    m_toggle->SetState(false);
    this->_update_edit_mode_ui();
    _loading_data = false;
}

bool ChapterContentView::save_changes()
{
    if (!_is_dirty_view || !chapter_id.has_value()) return true;

    wxString xml_content = m_visual_editor->GetText();
    wxString plain_text = m_visual_editor->GetPlainText();

    // --- PROTOCOLO DE CERO FALLOS SILENCIOSOS ---
    // Si el XML generado está vacío, pero hay texto visible en el editor, el generador XML colapsó.
    if (xml_content.IsEmpty() && !plain_text.IsEmpty())
    {
        wxString err_msg = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"ERROR CRÍTICO: Se ha intentado guardar el texto, pero el generador de formato colapsó. Para evitar que pierdas tu progreso, el guardado ha sido ABORTADO.")));
        wxString err_cap = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Abortando Guardado por Seguridad")));
        wxMessageBox(err_msg, err_cap, wxOK | wxICON_ERROR);
        return false;
    }

    wxString secure_xml = wxString::Format("%s", xml_content);

    bool success = app_handler->update_chapter_content_via_handler(chapter_id.value(), secure_xml);

    if (success)
    {
        _is_dirty_view = false;
    }
    else
    {
        // Fallo a nivel de SQLite
        wxString err_msg = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"ERROR CRÍTICO: La base de datos SQLite rechazó la actualización del contenido. Verifica si el archivo está en modo solo lectura.")));
        wxString err_cap = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Fallo de Base de Datos")));
        wxMessageBox(err_msg, err_cap, wxOK | wxICON_ERROR);
    }

    return success;
}

void ChapterContentView::on_toggle_switch(wxCommandEvent& event)
{
    if (!chapter_id.has_value()) { m_toggle->SetState(false); return; }

    _is_in_edit_mode = m_toggle->IsOn();

    if (!_is_in_edit_mode && _is_dirty_view) { this->save_changes(); }

    this->_update_edit_mode_ui();
}

void ChapterContentView::_update_edit_mode_ui()
{
    bool can_edit = _is_in_edit_mode && chapter_id.has_value();

    m_visual_editor->SetEditable(can_edit);

    wxString status = can_edit ? wxString::FromUTF8(reinterpret_cast<const char*>(u8"On")) : wxString::FromUTF8(reinterpret_cast<const char*>(u8"Off"));
    wxString label_text = wxString::Format("%s: %s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Modo Edición")), status);

    m_toggle_label->SetLabel(label_text);
    m_toggle_label->SetForegroundColour(can_edit ? wxColour(0, 150, 0) : wxColour(150, 0, 0));

    this->Layout();
}

void ChapterContentView::_update_word_count()
{
    wxString plain_text = m_visual_editor->GetPlainText();
    wxString delimiters = wxString::Format(" \t\r\n");
    wxStringTokenizer tokenizer(plain_text, delimiters);
    int count = tokenizer.CountTokens();

    wxString words_fmt = wxString::Format("%s: %d", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Palabras")), count);
    m_word_count_label->SetLabel(words_fmt);
}

void ChapterContentView::on_text_changed(wxCommandEvent& event)
{
    this->_update_word_count();
    if (!_loading_data && _is_in_edit_mode) { this->set_view_dirty(true); }
    event.Skip();
}

void ChapterContentView::set_view_dirty(bool is_dirty)
{
    if (_is_dirty_view != is_dirty) {
        _is_dirty_view = is_dirty;
        if (_is_dirty_view) app_handler->set_dirty(true);
    }
}

void ChapterContentView::enable_view(bool enable) { this->Enable(enable); }
bool ChapterContentView::is_dirty() const { return _is_dirty_view; }
bool ChapterContentView::is_editable() const { return _is_in_edit_mode; }
bool ChapterContentView::force_save_if_dirty() { return this->save_changes(); }