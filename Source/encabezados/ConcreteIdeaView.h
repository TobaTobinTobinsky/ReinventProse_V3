/*
* File Name: ConcreteIdeaView.h
* Descripción: Vista para gestionar ideas concretas asociadas a un capítulo como tarjetas dinámicas (Post-Its).
* Author: AutoDoc AI (Sugerido por el Jefe)
* Date: 07/06/2025
* Version: 2.0.0
* License: MIT License
*/

#ifndef CONCRETEIDEAVIEW_H
#define CONCRETEIDEAVIEW_H

#include <wx/wx.h>
#include <wx/wrapsizer.h>
#include <vector>
#include <string>
#include <optional>
#include <functional>
#include "DBManager.h" // Para el tipo estricto DBRow

class AppHandler;

// --- Diálogo emergente para ver, editar o borrar una idea (La Ventanita) ---
class IdeaDetailDialog : public wxDialog {
public:
    IdeaDetailDialog(wxWindow* parent, const std::string& initial_text);

    std::string GetText() const;
    bool IsDeleted() const { return m_deleted; }

private:
    void OnDelete(wxCommandEvent& event);

    wxTextCtrl* m_text_ctrl;
    bool m_deleted;
};


// --- Componente Gráfico: La Tarjeta de Idea Individual (El Post-It) ---
class ConcreteIdeaCard : public wxPanel {
public:
    ConcreteIdeaCard(wxWindow* parent, int id, const std::string& text, std::function<void(int, std::string)> on_click);

    int GetIdeaId() const { return m_id; }
    std::string GetIdeaText() const { return m_text; }

private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);

    int m_id;
    std::string m_text;
    std::function<void(int, std::string)> m_on_click;
};


// --- Panel Contenedor Principal (El Tablero) ---
// Como queremos que los "post-its" fluyan, usamos un Panel que soporta WrapSizer
class ConcreteIdeaView : public wxPanel {
public:
    /**
     * Inicializa el tablero de ideas concretas.
     */
    ConcreteIdeaView(wxWindow* parent, AppHandler* app_handler);

    /**
     * Carga las tarjetas de ideas para un capítulo específico.
     */
    void load_ideas(std::optional<int> chapter_id);

    /**
     * Bloquea o desbloquea el tablero según si hay o no un capítulo cargado.
     */
    void enable_view(bool enable);

private:
    // Construcción visual
    void _create_controls();
    void _layout_controls();
    void _update_button_states();

    // Eventos
    void OnAddIdea(wxCommandEvent& event);
    void OnCardClicked(int id, std::string text);

    // Datos y Control
    AppHandler* m_app_handler;
    std::optional<int> m_chapter_id;

    // Controles UI
    wxStaticText* m_info_label;
    wxButton* m_add_button;
    wxWrapSizer* m_wrap_sizer;
};

#endif // CONCRETEIDEAVIEW_H