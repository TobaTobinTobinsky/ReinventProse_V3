/*
* File Name: CustomAboutDialog.h
* Description: Define una ventana emergente "Acerca de" personalizada.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef CUSTOMABOUTDIALOG_H
#define CUSTOMABOUTDIALOG_H

#include <wx/wx.h>
#include <wx/notebook.h>
#include <vector>
#include <string>
#include <tuple>
#include <optional>

// Datos constantes para el logo
const std::string LOGO_FILENAME = "app_icon.ico";
const wxSize LOGO_DISPLAY_SIZE = wxSize(100, 100);

/**
 * Clase contenedora de la información de la aplicación.
 */
class ReinventProseAboutInfo {
public:
    ReinventProseAboutInfo();

    void SetName(const std::string& name);
    std::string GetName() const;

    void SetVersion(const std::string& version);
    std::string GetVersion() const;

    void SetDescription(const std::string& description);
    std::string GetDescription() const;

    void SetCopyright(const std::string& copyright_text);
    std::string GetCopyright() const;

    void SetLicense(const std::string& license_text);
    std::string GetLicense() const;

    void SetWebSite(const std::string& url, const std::string& desc = "");
    std::string GetWebSiteURL() const;
    std::string GetWebSiteDescription() const;

    void SetIcon(const wxIcon& icon);
    wxIcon GetIcon() const;

    void AddDeveloper(const std::string& developer);
    std::vector<std::string> GetDevelopers() const;

    void AddDocWriter(const std::string& writer);
    std::vector<std::string> GetDocWriters() const;

    void AddArtist(const std::string& artist);
    std::vector<std::string> GetArtists() const;

    void AddTranslator(const std::string& translator);
    std::vector<std::string> GetTranslators() const;

    void AddCollaborator(const std::string& name, const std::string& contribution = "");
    std::vector<std::pair<std::string, std::string>> GetCollaborators() const;

private:
    std::string m_name;
    std::string m_version;
    std::string m_description;
    std::string m_copyright;
    std::string m_license;
    std::string m_website_url;
    std::string m_website_desc;
    wxIcon m_icon;
    std::vector<std::string> m_developers;
    std::vector<std::string> m_doc_writers;
    std::vector<std::string> m_artists;
    std::vector<std::string> m_translators;
    std::vector<std::pair<std::string, std::string>> m_collaborators;
};

/**
 * Frame personalizado para mostrar la información (Notebook con pestañas).
 */
class ReinventProseAboutFrame : public wxFrame {
public:
    ReinventProseAboutFrame(wxWindow* parent, const ReinventProseAboutInfo& info);

private:
    void _create_ui();
    void _populate_data();
    wxPanel* _create_tab_text_panel(wxNotebook* parent, wxTextCtrl** ctrl_out);

    void OnCloseButton(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);

    ReinventProseAboutInfo m_info;
    wxStaticBitmap* m_logo_ctrl;
    wxNotebook* m_notebook;

    // Referencias a los controles de texto de cada pestaña
    wxTextCtrl* m_txt_general;
    wxTextCtrl* m_txt_license;
    wxTextCtrl* m_txt_developers;
    wxTextCtrl* m_txt_doc;
    wxTextCtrl* m_txt_artists;
    wxTextCtrl* m_txt_translators;
    wxTextCtrl* m_txt_collaborators;

    wxDECLARE_EVENT_TABLE();
};

#endif // CUSTOMABOUTDIALOG_H