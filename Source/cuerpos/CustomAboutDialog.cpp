/**
* File Name: CustomAboutDialog.cpp
* Descripción: Implementación del Frame de créditos con soporte estricto UTF-8,
*              wxString::Format y estructura de código expandida para depuración.
*/

#include "../encabezados/CustomAboutDialog.h"
#include "../encabezados/Util.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/mstream.h>

// Resources
#include "IconData.h"

// ============================================================================
// IMPLEMENTACIÓN: ReinventProseAboutInfo
// ============================================================================

ReinventProseAboutInfo::ReinventProseAboutInfo()
{
}

void ReinventProseAboutInfo::SetName(const std::string& name)
{
    m_name = name;
}

std::string ReinventProseAboutInfo::GetName() const
{
    return m_name;
}

void ReinventProseAboutInfo::SetVersion(const std::string& version)
{
    m_version = version;
}

std::string ReinventProseAboutInfo::GetVersion() const
{
    return m_version;
}

void ReinventProseAboutInfo::SetDescription(const std::string& description)
{
    m_description = description;
}

std::string ReinventProseAboutInfo::GetDescription() const
{
    return m_description;
}

void ReinventProseAboutInfo::SetCopyright(const std::string& copyright_text)
{
    m_copyright = copyright_text;
}

std::string ReinventProseAboutInfo::GetCopyright() const
{
    return m_copyright;
}

void ReinventProseAboutInfo::SetLicense(const std::string& license_text)
{
    m_license = license_text;
}

std::string ReinventProseAboutInfo::GetLicense() const
{
    return m_license;
}

void ReinventProseAboutInfo::SetWebSite(const std::string& url, const std::string& desc)
{
    m_website_url = url;
    m_website_desc = desc;
}

std::string ReinventProseAboutInfo::GetWebSiteURL() const
{
    return m_website_url;
}

std::string ReinventProseAboutInfo::GetWebSiteDescription() const
{
    return m_website_desc;
}

void ReinventProseAboutInfo::SetIcon(const wxIcon& icon)
{
    m_icon = icon;
}

wxIcon ReinventProseAboutInfo::GetIcon() const
{
    return m_icon;
}

void ReinventProseAboutInfo::AddDeveloper(const std::string& developer)
{
    m_developers.push_back(developer);
}

std::vector<std::string> ReinventProseAboutInfo::GetDevelopers() const
{
    return m_developers;
}

void ReinventProseAboutInfo::AddDocWriter(const std::string& writer)
{
    m_doc_writers.push_back(writer);
}

std::vector<std::string> ReinventProseAboutInfo::GetDocWriters() const
{
    return m_doc_writers;
}

void ReinventProseAboutInfo::AddArtist(const std::string& artist)
{
    m_artists.push_back(artist);
}

std::vector<std::string> ReinventProseAboutInfo::GetArtists() const
{
    return m_artists;
}

void ReinventProseAboutInfo::AddTranslator(const std::string& translator)
{
    m_translators.push_back(translator);
}

std::vector<std::string> ReinventProseAboutInfo::GetTranslators() const
{
    return m_translators;
}

void ReinventProseAboutInfo::AddCollaborator(const std::string& name, const std::string& contribution)
{
    m_collaborators.push_back({ name, contribution });
}

std::vector<std::pair<std::string, std::string>> ReinventProseAboutInfo::GetCollaborators() const
{
    return m_collaborators;
}

// ============================================================================
// IMPLEMENTACIÓN: ReinventProseAboutFrame
// ============================================================================

wxBEGIN_EVENT_TABLE(ReinventProseAboutFrame, wxFrame)
EVT_SIZE(ReinventProseAboutFrame::OnSize)
wxEND_EVENT_TABLE()

ReinventProseAboutFrame::ReinventProseAboutFrame(wxWindow* parent, const ReinventProseAboutInfo& info)
    : wxFrame(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(550, 600), wxDEFAULT_FRAME_STYLE | wxSTAY_ON_TOP),
    m_info(info)
{
    // Blindaje del Título de la ventana
    wxString frameTitle = wxString::Format("Acerca de %s", wxString::FromUTF8(m_info.GetName()));
    this->SetTitle(frameTitle);

    // Inicializar icono de barra de tareas
    wxMemoryInputStream stream(app_icon_data, app_icon_data_size);
    wxImage img;

    if (img.LoadFile(stream, wxBITMAP_TYPE_ICO))
    {
        wxIcon windowIcon;
        windowIcon.CopyFromBitmap(wxBitmap(img));
        this->SetIcon(windowIcon);
    }

    _create_ui();
    _populate_data();

    this->CentreOnParent();
}

void ReinventProseAboutFrame::_create_ui()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Renderizado del Logo Central
    wxMemoryInputStream logoStream(app_icon_data, app_icon_data_size);
    wxImage logoImg;

    if (logoImg.LoadFile(logoStream, wxBITMAP_TYPE_ICO))
    {
        wxBitmap logoBmp(logoImg.Rescale(120, 120, wxIMAGE_QUALITY_HIGH));
        m_logo_ctrl = new wxStaticBitmap(this, wxID_ANY, logoBmp);
    }
    else
    {
        m_logo_ctrl = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    }

    main_sizer->Add(m_logo_ctrl, 0, wxALIGN_CENTER | wxALL, 15);

    // Configuración del Notebook con pestañas multilínea
    m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxNB_MULTILINE);

    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_general), wxString::Format("General"));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_license), wxString::Format("Licencia"));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_developers), wxString::FromUTF8("Equipo de Desarrollo"));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_doc), wxString::Format("Documentadores"));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_artists), wxString::Format("Artistas"));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_translators), wxString::FromUTF8("Traducción"));
    m_notebook->AddPage(_create_tab_text_panel(m_notebook, &m_txt_collaborators), wxString::Format("Colaboradores"));

    main_sizer->Add(m_notebook, 1, wxEXPAND | wxALL, 10);

    // Botón de salida
    wxButton* close_btn = new wxButton(this, wxID_ANY, wxString::Format("Cerrar"));

    close_btn->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        this->Destroy();
        });

    main_sizer->Add(close_btn, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    this->SetSizer(main_sizer);
    this->SetSize(wxSize(500, 550));
}

wxPanel* ReinventProseAboutFrame::_create_tab_text_panel(wxNotebook* parent, wxTextCtrl** ctrl_out)
{
    wxPanel* panel = new wxPanel(parent);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    *ctrl_out = new wxTextCtrl(
        panel,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxBORDER_NONE
    );

    wxColour bg = panel->GetBackgroundColour();
    (*ctrl_out)->SetBackgroundColour(bg);

    sizer->Add(*ctrl_out, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);

    return panel;
}

void ReinventProseAboutFrame::_populate_data()
{
    // Construcción de la información General
    wxString name = wxString::FromUTF8(m_info.GetName());
    wxString version = wxString::FromUTF8(m_info.GetVersion());
    wxString copyright = wxString::FromUTF8(m_info.GetCopyright());
    wxString description = wxString::FromUTF8(m_info.GetDescription());

    wxString gen = wxString::Format("%s %s\n\n%s\n\n%s",
        name,
        version,
        copyright,
        description
    );

    if (!m_info.GetWebSiteURL().empty())
    {
        wxString url = wxString::FromUTF8(m_info.GetWebSiteURL());
        wxString webLine = wxString::Format("\n\nSitio Web: %s", url);
        gen += webLine;
    }

    m_txt_general->SetValue(gen);

    // Licencia
    wxString license = wxString::FromUTF8(m_info.GetLicense());
    m_txt_license->SetValue(license);

    // Listas de personal
    auto populate_list = [](wxTextCtrl* ctrl, const std::vector<std::string>& list) {
        wxString content;
        if (list.empty())
        {
            content = wxString::Format("(No especificado)");
        }
        else
        {
            for (const auto& item : list)
            {
                wxString line = wxString::Format("- %s\n", wxString::FromUTF8(item));
                content += line;
            }
        }
        ctrl->SetValue(content);
        };

    populate_list(m_txt_developers, m_info.GetDevelopers());
    populate_list(m_txt_doc, m_info.GetDocWriters());
    populate_list(m_txt_artists, m_info.GetArtists());
    populate_list(m_txt_translators, m_info.GetTranslators());

    // Colaboradores
    wxString colab;
    auto colab_list = m_info.GetCollaborators();

    if (colab_list.empty())
    {
        colab = wxString::Format("(Ninguno)");
    }
    else
    {
        for (const auto& pair : colab_list)
        {
            wxString person = wxString::FromUTF8(pair.first);
            wxString task = wxString::FromUTF8(pair.second);

            if (!task.empty())
            {
                colab += wxString::Format("- %s: %s\n", person, task);
            }
            else
            {
                colab += wxString::Format("- %s\n", person);
            }
        }
    }

    m_txt_collaborators->SetValue(colab);
}

void ReinventProseAboutFrame::OnCloseButton(wxCommandEvent& event)
{
    this->Destroy();
}

void ReinventProseAboutFrame::OnSize(wxSizeEvent& event)
{
    this->Layout();
}