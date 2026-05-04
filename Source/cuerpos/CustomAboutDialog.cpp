/**
* File Name: CustomAboutDialog.cpp
* Descripción: Implementación del Frame de créditos blindado.
* ESTÁNDAR C++20: Literales u8 para garantizar UTF-8 real en Windows.
* PROTOCOLO DE BLINDAJE: Format + FromUTF8 + reinterpret_cast.
*/

#include "../encabezados/CustomAboutDialog.h"
#include "../encabezados/Util.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/mstream.h>

// Resources
#include "IconData.h"

// --- Implementación de ReinventProseAboutInfo ---

ReinventProseAboutInfo::ReinventProseAboutInfo() {}
void ReinventProseAboutInfo::SetName(const std::string& name) { m_name = name; }
std::string ReinventProseAboutInfo::GetName() const { return m_name; }
void ReinventProseAboutInfo::SetVersion(const std::string& version) { m_version = version; }
std::string ReinventProseAboutInfo::GetVersion() const { return m_version; }
void ReinventProseAboutInfo::SetDescription(const std::string& description) { m_description = description; }
std::string ReinventProseAboutInfo::GetDescription() const { return m_description; }
void ReinventProseAboutInfo::SetCopyright(const std::string& copyright) { m_copyright = copyright; }
std::string ReinventProseAboutInfo::GetCopyright() const { return m_copyright; }
void ReinventProseAboutInfo::SetLicense(const std::string& license) { m_license = license; }
std::string ReinventProseAboutInfo::GetLicense() const { return m_license; }
void ReinventProseAboutInfo::SetWebSite(const std::string& url, const std::string& desc) { m_website_url = url; m_website_desc = desc; }
std::string ReinventProseAboutInfo::GetWebSiteURL() const { return m_website_url; }
std::string ReinventProseAboutInfo::GetWebSiteDescription() const { return m_website_desc; }
void ReinventProseAboutInfo::SetIcon(const wxIcon& icon) { m_icon = icon; }
wxIcon ReinventProseAboutInfo::GetIcon() const { return m_icon; }
void ReinventProseAboutInfo::AddDeveloper(const std::string& d) { m_developers.push_back(d); }
std::vector<std::string> ReinventProseAboutInfo::GetDevelopers() const { return m_developers; }
void ReinventProseAboutInfo::AddDocWriter(const std::string& w) { m_doc_writers.push_back(w); }
std::vector<std::string> ReinventProseAboutInfo::GetDocWriters() const { return m_doc_writers; }
void ReinventProseAboutInfo::AddArtist(const std::string& a) { m_artists.push_back(a); }
std::vector<std::string> ReinventProseAboutInfo::GetArtists() const { return m_artists; }
void ReinventProseAboutInfo::AddTranslator(const std::string& t) { m_translators.push_back(t); }
std::vector<std::string> ReinventProseAboutInfo::GetTranslators() const { return m_translators; }
void ReinventProseAboutInfo::AddCollaborator(const std::string& n, const std::string& c) { m_collaborators.push_back({ n, c }); }
std::vector<std::pair<std::string, std::string>> ReinventProseAboutInfo::GetCollaborators() const { return m_collaborators; }

// --- Implementación de ReinventProseAboutFrame ---

wxBEGIN_EVENT_TABLE(ReinventProseAboutFrame, wxFrame)
EVT_SIZE(ReinventProseAboutFrame::OnSize)
wxEND_EVENT_TABLE()

ReinventProseAboutFrame::ReinventProseAboutFrame(wxWindow* parent, const ReinventProseAboutInfo& info)
    : wxFrame(parent, wxID_ANY,
        wxString::Format("%s %s",
            wxString::FromUTF8(reinterpret_cast<const char*>(u8"Acerca de")),
            wxString::FromUTF8(info.GetName().c_str())),
        wxDefaultPosition, wxSize(550, 600),
        wxDEFAULT_FRAME_STYLE | wxSTAY_ON_TOP),
    m_info(info)
{
    wxMemoryInputStream stream(app_icon_data, app_icon_data_size);
    wxImage img;
    if (img.LoadFile(stream, wxBITMAP_TYPE_ICO)) {
        wxIcon smallIcon;
        smallIcon.CopyFromBitmap(wxBitmap(img));
        this->SetIcon(smallIcon);
    }

    _create_ui();
    _populate_data();
    this->CentreOnParent();
}

void ReinventProseAboutFrame::_create_ui() {
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    wxMemoryInputStream logoStream(app_icon_data, app_icon_data_size);
    wxImage logoImg;
    if (logoImg.LoadFile(logoStream, wxBITMAP_TYPE_ICO)) {
        wxBitmap logoBmp(logoImg.Rescale(120, 120, wxIMAGE_QUALITY_HIGH));
        m_logo_ctrl = new wxStaticBitmap(this, wxID_ANY, logoBmp);
    }
    else {
        m_logo_ctrl = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    }
    main_sizer->Add(m_logo_ctrl, 0, wxALIGN_CENTER | wxALL, 15);

    m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxNB_MULTILINE);

    // PROTOCOLO: Etiquetas de pestañas blindadas
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_general), wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"General"))));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_license), wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Licencia"))));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_developers), wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Equipo de Desarrollo"))));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_doc), wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Documentadores"))));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_artists), wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Artistas"))));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_translators), wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Traducción"))));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_collaborators), wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Colaboradores"))));

    main_sizer->Add(m_notebook, 1, wxEXPAND | wxALL, 10);

    wxButton* close_btn = new wxButton(this, wxID_ANY, wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Cerrar"))));
    close_btn->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) { this->Destroy(); });

    main_sizer->Add(close_btn, 0, wxALIGN_CENTER | wxBOTTOM, 10);
    SetSizer(main_sizer);
    this->SetSize(wxSize(500, 550));
}

wxPanel* ReinventProseAboutFrame::_create_tab_text_panel(wxNotebook* parent, wxTextCtrl** ctrl_out) {
    wxPanel* panel = new wxPanel(parent);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    *ctrl_out = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxBORDER_NONE);
    (*ctrl_out)->SetBackgroundColour(panel->GetBackgroundColour());
    sizer->Add(*ctrl_out, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);
    return panel;
}

void ReinventProseAboutFrame::_populate_data() {
    // PROTOCOLO: Construcción de texto "General" sin operadores << o +
    wxString gen = wxString::Format("%s %s\n\n%s\n\n%s",
        wxString::FromUTF8(m_info.GetName().c_str()),
        wxString::FromUTF8(m_info.GetVersion().c_str()),
        wxString::FromUTF8(m_info.GetCopyright().c_str()),
        wxString::FromUTF8(m_info.GetDescription().c_str())
    );

    if (!m_info.GetWebSiteURL().empty()) {
        gen = wxString::Format("%s\n\n%s: %s", gen, wxString::FromUTF8(reinterpret_cast<const char*>(u8"Sitio Web")), wxString::FromUTF8(m_info.GetWebSiteURL().c_str()));
    }
    m_txt_general->SetValue(gen);

    m_txt_license->SetValue(wxString::Format("%s", wxString::FromUTF8(m_info.GetLicense().c_str())));

    // Lambda blindada para poblar listas
    auto populate_list = [](wxTextCtrl* ctrl, const std::vector<std::string>& list) {
        if (list.empty()) {
            ctrl->SetValue(wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"(No especificado)"))));
        }
        else {
            wxString s = wxEmptyString;
            for (const auto& item : list) {
                s = wxString::Format("%s- %s\n", s, wxString::FromUTF8(item.c_str()));
            }
            ctrl->SetValue(s);
        }
        };

    populate_list(m_txt_developers, m_info.GetDevelopers());
    populate_list(m_txt_doc, m_info.GetDocWriters());
    populate_list(m_txt_artists, m_info.GetArtists());
    populate_list(m_txt_translators, m_info.GetTranslators());

    // Colaboradores
    auto colab_list = m_info.GetCollaborators();
    if (colab_list.empty()) {
        m_txt_collaborators->SetValue(wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"(Ninguno)"))));
    }
    else {
        wxString colab = wxEmptyString;
        for (const auto& pair : colab_list) {
            wxString line = wxString::Format("- %s", wxString::FromUTF8(pair.first.c_str()));
            if (!pair.second.empty()) {
                line = wxString::Format("%s: %s", line, wxString::FromUTF8(pair.second.c_str()));
            }
            colab = wxString::Format("%s%s\n", colab, line);
        }
        m_txt_collaborators->SetValue(colab);
    }
}

void ReinventProseAboutFrame::OnCloseButton(wxCommandEvent& event) { this->Destroy(); }
void ReinventProseAboutFrame::OnSize(wxSizeEvent& event) { this->Layout(); }