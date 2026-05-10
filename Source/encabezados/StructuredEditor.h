/**
* File Name: StructuredEditor.h
* Descripción: Interfaz del editor WYSIWYG profesional con detección de cambios.
* Autor: AutoDoc AI (Protocolo de Estabilidad C++20)
* Versión: 3.0.1 (Fix de Firmas de Eventos)
*/

#ifndef STRUCTUREDEDITOR_H
#define STRUCTUREDEDITOR_H

#include <wx/wx.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/richtext/richtextxml.h>
#include <wx/artprov.h> // FIX C2653: Inclusión explícita de iconos

/**
* @class StructuredEditor
* @brief Control de edición visual que encapsula la lógica de formato y persistencia XML.
*/
class StructuredEditor : public wxPanel {
public:
    StructuredEditor(wxWindow* parent, wxWindowID id);

    /**
    * Carga una cadena (XML o Plano) en el editor.
    */
    void SetText(const wxString& content);

    /**
    * Extrae la estructura visual completa en formato XML string.
    */
    wxString GetText() const;

    /**
    * Devuelve el texto plano (sin etiquetas XML) para conteo de palabras.
    */
    wxString GetPlainText() const;

    // Comandos de formato
    void ApplyHeading(int level);
    void ApplyBold();
    void ApplyItalic();

    /**
    * Elimina todo el formato de la selección y restaura el estilo base.
    */
    void ResetFormat();

    void SetEditable(bool editable);

private:
    void _create_ui();
    void _setup_editor();

    // FIX C2672: Los eventos de RichText requieren ESTRICTAMENTE wxRichTextEvent
    void _on_content_changed(wxRichTextEvent& event);

    // Los botones de la barra de herramientas sí usan wxCommandEvent
    void _on_toolbar_click(wxCommandEvent& event);

    wxToolBar* m_toolbar;
    wxRichTextCtrl* m_rtc;

    enum {
        ID_RTC_INTERNAL = 9700,
        ID_TOOL_H1,
        ID_TOOL_H2,
        ID_TOOL_H3,
        ID_TOOL_BOLD,
        ID_TOOL_ITALIC,
        ID_TOOL_RESET
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // STRUCTUREDEDITOR_H