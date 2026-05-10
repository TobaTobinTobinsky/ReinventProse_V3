/**
* File Name: StructuredEditor.cpp
* Descripción: Implementación del editor WYSIWYG con Iconografía Procedural.
*              CORRECCIÓN CRÍTICA: Uso de tipos estrictos (wxFontWeight/wxFontStyle)
*              para evitar el error C4996 en wxWidgets 3.3.2.
* Autor: AutoDoc AI (Protocolo de Estabilidad C++20)
* Versión: 3.1.1
*/

#include "../encabezados/StructuredEditor.h"
#include <wx/sstream.h>
#include <wx/mstream.h>
#include <wx/msgdlg.h> 
#include <wx/richtext/richtextxml.h>
#include <wx/graphics.h>  
#include <wx/dcmemory.h>  
#include <wx/settings.h>

// ============================================================================
// GENERADOR PROCEDURAL DE BOTONES (El "Truco" de las Tarjetitas)
// ============================================================================

static wxBitmap CreateFormatIcon(const wxString& text, int ptSize, bool isBold, bool isItalic, wxColour textCol)
{
    int size = 32; // Botón Cuadrado Perfecto
    wxBitmap bmp(size, size);
    wxMemoryDC dc(bmp);

    // Fondo base del mismo color que la barra para evitar bordes feos
    dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
    dc.Clear();

    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc)
    {
        // 1. Cuerpo de la "Cajita" (Borde sutil)
        gc->SetBrush(wxBrush(wxColour(245, 245, 245)));
        gc->SetPen(wxPen(wxColour(180, 180, 180), 1));
        gc->DrawRoundedRectangle(1, 1, size - 2, size - 2, 4);

        // 2. Estructuración de la fuente física (RESOLUCIÓN C4996: Tipos estrictos)
        wxFontWeight weight = isBold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL;
        wxFontStyle style = isItalic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL;

        // Blindaje C++20 del nombre de la fuente
        wxString font_name = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Segoe UI")));
        wxFont font(ptSize, wxFONTFAMILY_DEFAULT, style, weight, false, font_name);

        gc->SetFont(font, textCol);

        // 3. Centrado Matemático del texto
        double tw, th;
        gc->GetTextExtent(text, &tw, &th);
        gc->DrawText(text, (size - tw) / 2.0, (size - th) / 2.0);
    }

    dc.SelectObject(wxNullBitmap);
    return bmp;
}

// ============================================================================
// TABLA DE EVENTOS
// ============================================================================

wxBEGIN_EVENT_TABLE(StructuredEditor, wxPanel)
EVT_TOOL(ID_TOOL_H1, StructuredEditor::_on_toolbar_click)
EVT_TOOL(ID_TOOL_H2, StructuredEditor::_on_toolbar_click)
EVT_TOOL(ID_TOOL_H3, StructuredEditor::_on_toolbar_click)
EVT_TOOL(ID_TOOL_BOLD, StructuredEditor::_on_toolbar_click)
EVT_TOOL(ID_TOOL_ITALIC, StructuredEditor::_on_toolbar_click)
EVT_TOOL(ID_TOOL_RESET, StructuredEditor::_on_toolbar_click)

EVT_RICHTEXT_CONTENT_INSERTED(ID_RTC_INTERNAL, StructuredEditor::_on_content_changed)
EVT_RICHTEXT_CONTENT_DELETED(ID_RTC_INTERNAL, StructuredEditor::_on_content_changed)
EVT_RICHTEXT_STYLE_CHANGED(ID_RTC_INTERNAL, StructuredEditor::_on_content_changed)
wxEND_EVENT_TABLE()

StructuredEditor::StructuredEditor(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id)
{
    this->_create_ui();
    this->_setup_editor();
}

void StructuredEditor::_create_ui()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER);
    m_toolbar->SetToolBitmapSize(wxSize(32, 32)); // Forzar cuadrados de 32x32

    // Tooltips Blindados
    auto tool_h1_tip = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Título de Nivel 1 (24pt)")));
    auto tool_h2_tip = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Título de Nivel 2 (18pt)")));
    auto tool_h3_tip = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Título de Nivel 3 (14pt)")));
    auto tool_bold_tip = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Aplicar Negrita")));
    auto tool_italic_tip = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Aplicar Cursiva")));
    auto tool_reset_tip = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Limpiar todo el formato (Texto Plano 12pt)")));

    // Generación Procedural de Botones
    wxBitmap bmp_h1 = CreateFormatIcon(wxString::Format("T1"), 16, true, false, wxColour(40, 70, 160));
    wxBitmap bmp_h2 = CreateFormatIcon(wxString::Format("T2"), 14, true, false, wxColour(50, 100, 200));
    wxBitmap bmp_h3 = CreateFormatIcon(wxString::Format("T3"), 12, true, false, wxColour(80, 120, 180));
    wxBitmap bmp_bold = CreateFormatIcon(wxString::Format("N"), 14, true, false, *wxBLACK);
    wxBitmap bmp_italic = CreateFormatIcon(wxString::Format("C"), 14, false, true, *wxBLACK);
    wxBitmap bmp_reset = CreateFormatIcon(wxString::Format("R"), 12, false, false, wxColour(180, 0, 0));

    // Ensamblaje Visual
    m_toolbar->AddTool(ID_TOOL_H1, wxString::Format("H1"), bmp_h1, tool_h1_tip);
    m_toolbar->AddTool(ID_TOOL_H2, wxString::Format("H2"), bmp_h2, tool_h2_tip);
    m_toolbar->AddTool(ID_TOOL_H3, wxString::Format("H3"), bmp_h3, tool_h3_tip);
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_TOOL_BOLD, wxString::Format("B"), bmp_bold, tool_bold_tip);
    m_toolbar->AddTool(ID_TOOL_ITALIC, wxString::Format("I"), bmp_italic, tool_italic_tip);
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_TOOL_RESET, wxString::Format("R"), bmp_reset, tool_reset_tip);

    m_toolbar->Realize();

    m_rtc = new wxRichTextCtrl(this, ID_RTC_INTERNAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL | wxBORDER_NONE);

    main_sizer->Add(m_toolbar, 0, wxEXPAND);
    main_sizer->Add(m_rtc, 1, wxEXPAND);

    this->SetSizer(main_sizer);
}

void StructuredEditor::_setup_editor()
{
    if (!wxRichTextBuffer::FindHandler(wxRICHTEXT_TYPE_XML)) {
        wxRichTextBuffer::AddHandler(new wxRichTextXMLHandler);
    }

    m_rtc->SetMargins(wxPoint(25, 25));

    wxRichTextAttr attr;
    attr.SetFontPointSize(12);
    attr.SetFontFaceName(wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Segoe UI"))));

    m_rtc->SetBasicStyle(attr);
    m_rtc->SetDefaultStyle(attr);
}

// ============================================================================
// GESTIÓN DE DATOS CON CONTROL ESTRICTO DE ERRORES
// ============================================================================

void StructuredEditor::SetText(const wxString& content)
{
    m_rtc->Clear();

    wxString xml_header = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"<?xml")));

    if (!content.StartsWith(xml_header))
    {
        m_rtc->SetValue(content);
    }
    else
    {
        wxStringInputStream sstream(content);
        bool load_success = m_rtc->GetBuffer().LoadFile(sstream, wxRICHTEXT_TYPE_XML);

        if (!load_success) {
            wxString err_msg = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"ERROR CRÍTICO: El motor wxRichTextCtrl falló al decodificar el XML proveniente de la base de datos. Los datos podrían estar corruptos.")));
            wxString err_cap = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Fallo de Lectura XML")));
            wxMessageBox(err_msg, err_cap, wxOK | wxICON_ERROR);
        }
    }

    m_rtc->Refresh();
}

wxString StructuredEditor::GetText() const
{
    wxString out_xml;
    wxStringOutputStream sstream(&out_xml);

    wxRichTextCtrl* rtc_bridge = const_cast<wxRichTextCtrl*>(m_rtc);
    bool save_success = rtc_bridge->GetBuffer().SaveFile(sstream, wxRICHTEXT_TYPE_XML);

    if (!save_success) {
        wxString err_msg = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"ERROR CRÍTICO: El motor wxRichTextCtrl no pudo generar la estructura XML del texto. Si guardas ahora, perderás tu trabajo.")));
        wxString err_cap = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Fallo de Escritura XML")));
        wxMessageBox(err_msg, err_cap, wxOK | wxICON_ERROR);

        return wxEmptyString;
    }

    return out_xml;
}

wxString StructuredEditor::GetPlainText() const
{
    return m_rtc->GetValue();
}

// ============================================================================
// COMANDOS DE FORMATO
// ============================================================================

void StructuredEditor::ApplyHeading(int level)
{
    wxRichTextAttr attr;
    attr.SetFontWeight(wxFONTWEIGHT_BOLD);

    switch (level)
    {
    case 1: attr.SetFontPointSize(24); break;
    case 2: attr.SetFontPointSize(18); break;
    case 3: attr.SetFontPointSize(14); break;
    }

    wxRichTextRange range = m_rtc->GetSelectionRange();
    if (range.GetStart() != range.GetEnd()) {
        m_rtc->SetStyle(range, attr);
    }
    else {
        m_rtc->BeginStyle(attr);
    }
    m_rtc->SetFocus();
}

void StructuredEditor::ApplyBold() { m_rtc->ApplyBoldToSelection(); m_rtc->SetFocus(); }
void StructuredEditor::ApplyItalic() { m_rtc->ApplyItalicToSelection(); m_rtc->SetFocus(); }

void StructuredEditor::ResetFormat()
{
    wxRichTextAttr attr;
    attr.SetFontPointSize(12);
    attr.SetFontWeight(wxFONTWEIGHT_NORMAL);
    attr.SetFontStyle(wxFONTSTYLE_NORMAL);
    attr.SetFontFaceName(wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Segoe UI"))));
    attr.SetTextColour(*wxBLACK);

    wxRichTextRange range = m_rtc->GetSelectionRange();
    if (range.GetStart() != range.GetEnd()) {
        m_rtc->SetStyle(range, attr);
    }
    else {
        m_rtc->SetDefaultStyle(attr);
    }
    m_rtc->SetFocus();
}

void StructuredEditor::SetEditable(bool editable)
{
    m_rtc->SetEditable(editable);
    m_toolbar->Enable(editable);
    m_rtc->SetBackgroundColour(editable ? *wxWHITE : wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    m_rtc->Refresh();
}

void StructuredEditor::_on_content_changed(wxRichTextEvent& event)
{
    wxCommandEvent change_evt(wxEVT_TEXT, GetId());
    change_evt.SetEventObject(this);
    GetEventHandler()->ProcessEvent(change_evt);
    event.Skip();
}

void StructuredEditor::_on_toolbar_click(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == ID_TOOL_H1) ApplyHeading(1);
    else if (id == ID_TOOL_H2) ApplyHeading(2);
    else if (id == ID_TOOL_H3) ApplyHeading(3);
    else if (id == ID_TOOL_BOLD) ApplyBold();
    else if (id == ID_TOOL_ITALIC) ApplyItalic();
    else if (id == ID_TOOL_RESET) ResetFormat();
}