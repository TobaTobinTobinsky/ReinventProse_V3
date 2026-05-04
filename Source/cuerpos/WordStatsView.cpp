/**
* Archivo: WordStatsView.cpp
* Descripción: Implementación completa de analítica léxica.
* ESTÁNDAR C++20: Literales u8 para garantizar UTF-8 real en Windows.
* PROTOCOLO DE BLINDAJE: Format + FromUTF8 + reinterpret_cast.
*/

#include "../encabezados/WordStatsView.h"
#include "../encabezados/AppHandler.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/tokenzr.h>
#include <wx/statline.h>
#include <wx/statbox.h> 
#include <algorithm>

// ============================================================================
// IMPLEMENTACIÓN: PieChartCtrl (Control de Gráfico Circular)
// ============================================================================

wxBEGIN_EVENT_TABLE(PieChartCtrl, wxControl)
EVT_PAINT(PieChartCtrl::OnPaint)
EVT_SIZE(PieChartCtrl::OnSize)
wxEND_EVENT_TABLE()

PieChartCtrl::PieChartCtrl(wxWindow* parent, wxWindowID id)
    : wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE), m_total_words(0)
{
    this->SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void PieChartCtrl::SetData(const std::vector<WordStat>& data, int total_words)
{
    m_data = data;
    m_total_words = total_words;
    this->Refresh();
}

void PieChartCtrl::OnSize(wxSizeEvent& event)
{
    this->Refresh();
    event.Skip();
}

void PieChartCtrl::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (!gc) return;

    wxSize sz = this->GetClientSize();
    gc->SetBrush(wxBrush(this->GetParent()->GetBackgroundColour()));
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(0, 0, sz.x, sz.y);

    if (m_total_words == 0 || m_data.empty()) return;

    double cx = sz.x / 2.0;
    double cy = sz.y / 2.0;
    double radius = std::min(cx, cy) - 15.0;
    double current_angle = -1.57079632679;

    for (const auto& stat : m_data)
    {
        double sweep_angle = ((double)stat.count / m_total_words) * 2.0 * 3.14159265359;
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(cx, cy);
        path.AddArc(cx, cy, radius, current_angle, current_angle + sweep_angle, true);
        path.CloseSubpath();
        gc->SetBrush(wxBrush(stat.color));
        gc->SetPen(wxPen(wxColour(255, 255, 255), 2));
        gc->FillPath(path);
        gc->StrokePath(path);
        current_angle += sweep_angle;
    }

    double inner_radius = radius * 0.4;
    gc->SetBrush(wxBrush(this->GetParent()->GetBackgroundColour()));
    gc->DrawEllipse(cx - inner_radius, cy - inner_radius, inner_radius * 2, inner_radius * 2);
}

// ============================================================================
// IMPLEMENTACIÓN: WordStatsView (Panel de Estadísticas)
// ============================================================================

WordStatsView::WordStatsView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent, wxID_ANY), m_app_handler(app_handler), m_chapter_id(std::nullopt)
{
    m_color_palette = {
        wxColour(84, 112, 198, 200), wxColour(145, 204, 117, 200),
        wxColour(250, 200, 88, 200), wxColour(238, 102, 102, 200),
        wxColour(115, 192, 222, 200), wxColour(59, 162, 114, 200),
        wxColour(252, 132, 82, 200), wxColour(154, 96, 180, 200),
        wxColour(234, 124, 204, 200), wxColour(140, 140, 140, 200)
    };

    this->_create_controls();
    this->_layout_controls();
}

void WordStatsView::_create_controls()
{
    m_pie_chart = new PieChartCtrl(this, wxID_ANY);
    m_pie_chart->SetMinSize(wxSize(350, 350));

    m_top10_container = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_top10_container->SetBackgroundColour(*wxWHITE);
    m_top10_container->SetScrollRate(0, 5);
    m_top10_sizer = new wxBoxSizer(wxVERTICAL);
    m_top10_container->SetSizer(m_top10_sizer);

    m_others_list = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    // --- BLINDAJE C++20: "PALABRAS TOTALES" ---
    const char* raw_total = reinterpret_cast<const char*>(u8"PALABRAS TOTALES");
    wxString label_total_text = wxString::Format("%s: 0", wxString::FromUTF8(raw_total));

    m_total_words_label = new wxStaticText(this, wxID_ANY, label_total_text, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    m_total_words_label->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    m_total_words_label->SetForegroundColour(wxColour(0, 0, 100));
}

void WordStatsView::_layout_controls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxHORIZONTAL);
    main_sizer->Add(m_pie_chart, 1, wxEXPAND | wxALL, 20);

    wxBoxSizer* right_column = new wxBoxSizer(wxVERTICAL);

    // --- CAJITA 1: TOP 10 (ESTÁNDAR C++20 u8 + reinterpret_cast) ---
    const char* raw_top10 = reinterpret_cast<const char*>(u8"Las 10 más utilizadas:");
    wxString title_top10 = wxString::Format("%s", wxString::FromUTF8(raw_top10));

    wxStaticBoxSizer* box_top10 = new wxStaticBoxSizer(wxVERTICAL, this, title_top10);
    box_top10->Add(m_top10_container, 1, wxEXPAND | wxALL, 5);

    // --- CAJITA 2: OTROS (ESTÁNDAR C++20 u8) ---
    const char* raw_others = reinterpret_cast<const char*>(u8"Otras palabras detectadas (A-Z):");
    wxString title_others = wxString::Format("%s", wxString::FromUTF8(raw_others));

    wxStaticBoxSizer* box_others = new wxStaticBoxSizer(wxVERTICAL, this, title_others);
    box_others->Add(m_others_list, 1, wxEXPAND | wxALL, 5);

    right_column->Add(box_top10, 1, wxEXPAND | wxALL, 10);
    right_column->Add(box_others, 1, wxEXPAND | wxALL, 10);

    right_column->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    right_column->Add(m_total_words_label, 0, wxALIGN_RIGHT | wxALL, 15);

    main_sizer->Add(right_column, 1, wxEXPAND);
    this->SetSizer(main_sizer);
}

void WordStatsView::_add_colored_row(const WordStat& stat)
{
    wxPanel* row = new wxPanel(m_top10_container, wxID_ANY);
    row->SetBackgroundColour(stat.color);

    wxBoxSizer* row_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* word_txt = new wxStaticText(row, wxID_ANY, stat.word);
    word_txt->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    wxString count_val = wxString::Format(": %d", stat.count);
    wxStaticText* count_txt = new wxStaticText(row, wxID_ANY, count_val);
    count_txt->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    row_sizer->Add(word_txt, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    row_sizer->Add(count_txt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    row->SetSizer(row_sizer);

    m_top10_sizer->Add(row, 0, wxEXPAND | wxBOTTOM, 1);
}

void WordStatsView::load_stats(std::optional<int> chapter_id)
{
    m_chapter_id = chapter_id;
    this->Freeze();

    m_others_list->Clear();
    m_top10_sizer->Clear(true);
    m_pie_chart->SetData({}, 0);

    const char* raw_total = reinterpret_cast<const char*>(u8"PALABRAS TOTALES");
    wxString total_reset = wxString::Format("%s: 0", wxString::FromUTF8(raw_total));
    m_total_words_label->SetLabel(total_reset);

    if (m_chapter_id.has_value())
    {
        auto details_opt = m_app_handler->get_chapter_details(m_chapter_id.value());
        if (details_opt.has_value())
        {
            DBRow details = details_opt.value();
            if (details.count("content"))
            {
                std::string raw = std::get<std::string>(details["content"]);
                wxString safe_text = wxString::Format("%s", wxString::FromUTF8(raw));
                this->_analyze_text(safe_text);
            }
        }
    }

    m_top10_container->Layout();
    m_top10_container->FitInside();
    this->Layout();
    this->Thaw();
}

void WordStatsView::_analyze_text(const wxString& text)
{
    if (text.IsEmpty()) return;

    wxString clean_text;
    for (size_t i = 0; i < text.Length(); i++) {
        wxChar c = text[i];
        if (wxIsalnum(c)) clean_text += c;
        else clean_text += ' ';
    }
    clean_text = clean_text.Lower();

    std::map<wxString, int> word_counts;
    wxString delimiters = wxString::Format(" \t\r\n");
    wxStringTokenizer tokenizer(clean_text, delimiters);

    int total_absolute_words = 0;
    while (tokenizer.HasMoreTokens()) {
        wxString word = tokenizer.GetNextToken();
        total_absolute_words++;
        word_counts[word]++;
    }

    std::vector<std::pair<wxString, int>> sorted_by_freq(word_counts.begin(), word_counts.end());
    std::sort(sorted_by_freq.begin(), sorted_by_freq.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

    std::vector<WordStat> top_stats;
    int words_in_top_sum = 0;
    int max_top = std::min((int)sorted_by_freq.size(), 10);

    for (int i = 0; i < max_top; i++)
    {
        WordStat stat;
        stat.word = wxString::Format("%s", sorted_by_freq[i].first);
        stat.count = sorted_by_freq[i].second;
        stat.color = m_color_palette[i % m_color_palette.size()];
        top_stats.push_back(stat);
        words_in_top_sum += stat.count;
        this->_add_colored_row(stat);
    }

    for (size_t i = max_top; i < sorted_by_freq.size(); i++)
    {
        wxString item = wxString::Format("%s (%d)", sorted_by_freq[i].first, sorted_by_freq[i].second);
        m_others_list->Append(item);
    }

    m_pie_chart->SetData(top_stats, words_in_top_sum);

    const char* raw_total = reinterpret_cast<const char*>(u8"PALABRAS TOTALES");
    wxString final_total = wxString::Format("%s: %d", wxString::FromUTF8(raw_total), total_absolute_words);
    m_total_words_label->SetLabel(final_total);
}

// RESOLUCIÓN LNK2001: Implementación de enable_view
void WordStatsView::enable_view(bool enable)
{
    this->Enable(enable);
}