/*
* Archivo: ChapterContentView.h
* Descripción: Panel de edición de capítulos con interruptor animado procedural.
*/

#ifndef CHAPTERCONTENTVIEW_H
#define CHAPTERCONTENTVIEW_H

#include <wx/wx.h>
#include <wx/timer.h>
#include <optional>
#include <string>

class AppHandler;

// --- INTERRUPTOR ANIMADO PROCEDURAL ---
class ModernToggleSwitch : public wxControl
{
public:
    ModernToggleSwitch(wxWindow* parent, wxWindowID id);

    bool IsOn() const { return m_isOn; }
    void SetState(bool on);

private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnTimer(wxTimerEvent& event);

    wxTimer m_timer;
    bool m_isOn;
    double m_progress; // 0.0 (OFF) a 1.0 (ON)

    // Configuración visual
    const int m_width = 50;
    const int m_height = 26;

    wxDECLARE_EVENT_TABLE();
};

// --- VISTA DE CONTENIDO ---
class ChapterContentView : public wxPanel
{
public:
    ChapterContentView(wxWindow* parent, AppHandler* app_handler);

    void load_content(std::optional<int> chapter_id);
    bool save_changes();

    bool is_editable() const;
    bool is_dirty() const;
    void enable_view(bool enable);
    bool force_save_if_dirty();
    void set_view_dirty(bool is_dirty = true);

private:
    void _create_controls();
    void _layout_controls();
    void _update_edit_mode_ui();

    void on_text_changed(wxCommandEvent& event);
    void on_toggle_switch(wxCommandEvent& event);

    AppHandler* app_handler;
    std::optional<int> chapter_id;
    bool _is_dirty_view;
    bool _loading_data;
    bool _is_in_edit_mode;

    // Controles
    wxStaticText* content_label;
    wxTextCtrl* content_ctrl;

    // El nuevo interruptor
    ModernToggleSwitch* m_toggle;
    wxStaticText* m_toggle_label;

    enum {
        ID_TOGGLE_SWITCH = 4001
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // CHAPTERCONTENTVIEW_H