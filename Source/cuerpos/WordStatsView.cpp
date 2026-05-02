/**
* Archivo: WordStatsView.cpp
* Descripción: Implementación de la analítica léxica pura.
*              Sin filtros, sin mentiras. Todo lo registrado en la DB se procesa.
*/

#include "../encabezados/WordStatsView.h"
#include "../encabezados/AppHandler.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/tokenzr.h>
#include <wx/statline.h>
#include <algorithm>

// ============================================================================
// IMPLEMENTACIÓN: PieChartCtrl
// ============================================================================

wxBEGIN_EVENT_TABLE(PieChartCtrl, wxControl)
EVT_PAINT(PieChartCtrl::OnPaint)
EVT_SIZE(PieChartCtrl::OnSize)
wxEND_EVENT_TABLE()

PieChartCtrl::PieChartCtrl(wxWindow* parent, wxWindowID id)
    : wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
    m_total_words(0)
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

    if (!gc)
    {
        return;
    }

    wxSize sz = this->GetClientSize();

    gc->SetBrush(wxBrush(this->GetParent()->GetBackgroundColour()));

    gc->SetPen(*wxTRANSPARENT_PEN);

    gc->DrawRectangle(0, 0, sz.x, sz.y);

    if (m_total_words == 0 || m_data.empty())
    {
        return;
    }

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

    gc->SetPen(*wxTRANSPARENT_PEN);

    gc->DrawEllipse(cx - inner_radius, cy - inner_radius, inner_radius * 2, inner_radius * 2);
}

// ============================================================================
// IMPLEMENTACIÓN: WordStatsView
// ============================================================================

WordStatsView::WordStatsView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent),
    m_app_handler(app_handler),
    m_chapter_id(std::nullopt)
{
    m_color_palette = {
        wxColour(84, 112, 198, 200),
        wxColour(145, 204, 117, 200),
        wxColour(250, 200, 88, 200),
        wxColour(238, 102, 102, 200),
        wxColour(115, 192, 222, 200),
        wxColour(59, 162, 114, 200),
        wxColour(252, 132, 82, 200),
        wxColour(154, 96, 180, 200),
        wxColour(234, 124, 204, 200),
        wxColour(140, 140, 140, 200)
    };

    this->_create_controls();

    this->_layout_controls();
}

void WordStatsView::_create_controls()
{
    m_pie_chart = new PieChartCtrl(this, wxID_ANY);

    m_pie_chart->SetMinSize(wxSize(350, 350));

    m_title_top10 = new wxStaticText(this, wxID_ANY, "Las 10 más utilizadas:");

    m_title_top10->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    m_top10_container = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 220), wxSUNKEN_BORDER);

    m_top10_container->SetBackgroundColour(*wxWHITE);

    m_top10_container->SetScrollRate(0, 5);

    m_top10_sizer = new wxBoxSizer(wxVERTICAL);

    m_top10_container->SetSizer(m_top10_sizer);

    m_title_others = new wxStaticText(this, wxID_ANY, "Otras palabras detectadas (A-Z):");

    m_title_others->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    m_others_list = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 150));

    m_total_words_label = new wxStaticText(this, wxID_ANY, "PALABRAS TOTALES: 0", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);

    m_total_words_label->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    m_total_words_label->SetForegroundColour(wxColour(0, 0, 100));
}

void WordStatsView::_layout_controls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxHORIZONTAL);

    main_sizer->Add(m_pie_chart, 1, wxEXPAND | wxALL, 20);

    wxBoxSizer* right_sizer = new wxBoxSizer(wxVERTICAL);

    right_sizer->Add(m_title_top10, 0, wxTOP | wxLEFT | wxRIGHT, 10);

    right_sizer->Add(m_top10_container, 0, wxEXPAND | wxALL, 10);

    right_sizer->Add(m_title_others, 0, wxTOP | wxLEFT | wxRIGHT, 10);

    right_sizer->Add(m_others_list, 1, wxEXPAND | wxALL, 10);

    right_sizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    right_sizer->Add(m_total_words_label, 0, wxALIGN_RIGHT | wxALL, 15);

    main_sizer->Add(right_sizer, 1, wxEXPAND);

    this->SetSizer(main_sizer);
}

void WordStatsView::_add_colored_row(const WordStat& stat)
{
    wxPanel* row = new wxPanel(m_top10_container, wxID_ANY);

    row->SetBackgroundColour(stat.color);

    wxBoxSizer* row_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* word_txt = new wxStaticText(row, wxID_ANY, stat.word);

    word_txt->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    wxStaticText* count_txt = new wxStaticText(row, wxID_ANY, wxString::Format(": %d", stat.count));

    count_txt->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    row_sizer->Add(word_txt, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);

    row_sizer->Add(count_txt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    row->SetSizer(row_sizer);

    m_top10_sizer->Add(row, 0, wxEXPAND | wxBOTTOM, 1);
}

void WordStatsView::load_stats(std::optional<int> chapter_id)
{
    m_chapter_id = chapter_id;

    m_others_list->Clear();

    m_top10_sizer->Clear(true);

    m_pie_chart->SetData({}, 0);

    m_total_words_label->SetLabel("PALABRAS TOTALES: 0");

    if (m_chapter_id.has_value())
    {
        auto details_opt = m_app_handler->get_chapter_details(m_chapter_id.value());

        if (details_opt.has_value())
        {
            DBRow details = details_opt.value();

            if (details.count("content"))
            {
                std::string raw_content = std::get<std::string>(details["content"]);

                wxString texto = wxString::FromUTF8(raw_content);

                this->_analyze_text(texto);
            }
        }
    }

    m_top10_container->Layout();

    this->Layout();
}

void WordStatsView::_analyze_text(const wxString& text)
{
    if (text.IsEmpty())
    {
        return;
    }

    // 1. LIMPIEZA Y TOKENIZACIÓN PURA
    wxString clean_text;

    for (size_t i = 0; i < text.Length(); i++)
    {
        wxChar c = text[i];

        // Incluimos números como palabras válidas de la DB
        if (wxIsalnum(c))
        {
            clean_text += c;
        }
        else
        {
            clean_text += ' ';
        }
    }

    clean_text = clean_text.Lower();

    std::map<wxString, int> word_counts;

    wxStringTokenizer tokenizer(clean_text, " \t\r\n");

    int total_absolute_words = 0;

    while (tokenizer.HasMoreTokens())
    {
        wxString word = tokenizer.GetNextToken();

        total_absolute_words++;

        // SIN FILTROS: Si la palabra existe en la DB, se cuenta.
        word_counts[word]++;
    }

    // 2. ORDENAR POR FRECUENCIA
    std::vector<std::pair<wxString, int>> sorted_by_freq(word_counts.begin(), word_counts.end());

    std::sort(sorted_by_freq.begin(), sorted_by_freq.end(),
        [](const std::pair<wxString, int>& a, const std::pair<wxString, int>& b) {
            return a.second > b.second;
        }
    );

    std::vector<WordStat> top_stats;

    int words_in_top_sum = 0;

    int max_top = std::min((int)sorted_by_freq.size(), 10);

    // 3. POBLAR TOP 10 (COLOR SINCRONIZADO)
    for (int i = 0; i < max_top; i++)
    {
        WordStat stat;

        stat.word = sorted_by_freq[i].first;

        stat.count = sorted_by_freq[i].second;

        stat.color = m_color_palette[i % m_color_palette.size()];

        top_stats.push_back(stat);

        words_in_top_sum += stat.count;

        this->_add_colored_row(stat);
    }

    // 4. POBLAR OTROS (A-Z)
    std::vector<std::pair<wxString, int>> others_alphabetical;

    for (size_t i = max_top; i < sorted_by_freq.size(); i++)
    {
        others_alphabetical.push_back(sorted_by_freq[i]);
    }

    std::sort(others_alphabetical.begin(), others_alphabetical.end(),
        [](const std::pair<wxString, int>& a, const std::pair<wxString, int>& b) {
            return a.first < b.first;
        }
    );

    for (const auto& entry : others_alphabetical)
    {
        wxString other_text = wxString::Format("%s (%d)", entry.first, entry.second);

        m_others_list->Append(other_text);
    }

    // 5. ACTUALIZAR UI
    m_pie_chart->SetData(top_stats, words_in_top_sum);

    m_total_words_label->SetLabel(wxString::Format("PALABRAS TOTALES: %d", total_absolute_words));
}

void WordStatsView::enable_view(bool enable)
{
    this->Enable(enable);
}