/*
* Archivo: WordStatsView.h
* Descripción: Panel analítico avanzado que refleja la verdad absoluta del
*              contenido del capítulo sin filtros artificiales.
*/

#ifndef WORDSTATSVIEW_H
#define WORDSTATSVIEW_H

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <vector>
#include <optional>
#include <map>

class AppHandler;

// --- ESTRUCTURA DE DATOS PARA EL GRÁFICO ---
struct WordStat
{
    wxString word;

    int count;

    wxColour color;
};

// --- CONTROL PERSONALIZADO: GRÁFICO CIRCULAR ---
class PieChartCtrl : public wxControl
{
public:
    PieChartCtrl(
        wxWindow* parent,
        wxWindowID id
    );

    void SetData(
        const std::vector<WordStat>& data,
        int total_words
    );

private:
    void OnPaint(wxPaintEvent& event);

    void OnSize(wxSizeEvent& event);

    std::vector<WordStat> m_data;

    int m_total_words;

    wxDECLARE_EVENT_TABLE();
};

// --- VISTA PRINCIPAL DE ESTADÍSTICAS ---
class WordStatsView : public wxPanel
{
public:
    WordStatsView(
        wxWindow* parent,
        AppHandler* app_handler
    );

    void load_stats(std::optional<int> chapter_id);

    void enable_view(bool enable);

private:
    void _create_controls();

    void _layout_controls();

    // Motor de análisis (Fuente de Verdad: DB)
    void _analyze_text(const wxString& text);

    void _add_colored_row(const WordStat& stat);

    AppHandler* m_app_handler;

    std::optional<int> m_chapter_id;

    PieChartCtrl* m_pie_chart;

    wxScrolledWindow* m_top10_container;

    wxBoxSizer* m_top10_sizer;

    wxListBox* m_others_list;

    wxStaticText* m_total_words_label;

    wxStaticText* m_title_top10;

    wxStaticText* m_title_others;

    std::vector<wxColour> m_color_palette;
};

#endif // WORDSTATSVIEW_H