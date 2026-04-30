/*
* File Name: ConcreteIdeaView.h
*/

#ifndef CONCRETEIDEAVIEW_H
#define CONCRETEIDEAVIEW_H

#include <wx/wx.h>
#include <wx/wrapsizer.h>
#include <vector>
#include <string>
#include <optional>
#include <functional>

class AppHandler;

// Diálogo de edición
class IdeaDetailDialog : public wxDialog {
public:
    IdeaDetailDialog(wxWindow* parent, const std::string& initial_text);
    std::string GetText() const;
private:
    wxTextCtrl* m_text_ctrl;
};

// Tarjeta individual
class ConcreteIdeaCard : public wxPanel {
public:
    ConcreteIdeaCard(wxWindow* parent, int id, const std::string& text, bool is_first, bool is_last,
        std::function<void(int, std::string)> on_read,
        std::function<void(int)> on_delete,
        std::function<void(int, bool)> on_move);

private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);

    int m_id;
    std::string m_text;
    bool m_is_first, m_is_last;
    std::function<void(int, std::string)> m_on_read;
    std::function<void(int)> m_on_delete;
    std::function<void(int, bool)> m_on_move;
};

// Tablero principal
class ConcreteIdeaView : public wxPanel {
public:
    ConcreteIdeaView(wxWindow* parent, AppHandler* app_handler);
    void load_ideas(std::optional<int> chapter_id);
    void enable_view(bool enable);

private:
    void _create_controls();
    void _layout_controls();
    void _update_button_states();
    void _on_delete_requested(int id);
    void OnAddIdea(wxCommandEvent& event);
    void OnCardReadClicked(int id, std::string text);

    AppHandler* m_app_handler;
    std::optional<int> m_chapter_id;
    wxStaticText* m_info_label;
    wxButton* m_add_button;
    wxWrapSizer* m_wrap_sizer;
};

#endif