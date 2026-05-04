/**
 * Archivo: NewBookDialog.cpp
 * Descripción: Implementación del diálogo de creación de libros.
 * Lógica aplicada: Blindaje Universal mediante wxString::Format.
 */

#include "../encabezados/NewBookDialog.h"
#include "../encabezados/AppHandler.h"
#include <wx/sizer.h>
#include <wx/msgdlg.h>

wxBEGIN_EVENT_TABLE(NewBookDialog, wxDialog)
// Eventos vinculados dinámicamente mediante Bind en el constructor
wxEND_EVENT_TABLE()

NewBookDialog::NewBookDialog(wxWindow* parent, AppHandler* app_handler, const wxString& dialog_title)
    : wxDialog(parent, wxID_ANY, wxString::Format("%s", dialog_title), wxDefaultPosition, wxSize(500, 350)),
    app_handler(app_handler)
{
    _create_controls();
    _layout_controls();

    this->CentreOnParent();
}

void NewBookDialog::_create_controls()
{
    form_panel = new wxPanel(this, wxID_ANY);

    // Aplicación del Blindaje en etiquetas estáticas para consistencia total
    title_label = new wxStaticText(form_panel, wxID_ANY, wxString::Format("Título (*):"));
    author_label = new wxStaticText(form_panel, wxID_ANY, wxString::Format("Autor (*):"));
    synopsis_label = new wxStaticText(form_panel, wxID_ANY, wxString::Format("Sinopsis:"));

    title_ctrl = new wxTextCtrl(form_panel, wxID_ANY);
    author_ctrl = new wxTextCtrl(form_panel, wxID_ANY);

    synopsis_ctrl = new wxTextCtrl(
        form_panel,
        wxID_ANY,
        wxString::Format(""), // Uso de Format en lugar de wxEmptyString
        wxDefaultPosition,
        wxSize(-1, 100),
        wxTE_MULTILINE
    );

    ok_btn = new wxButton(this, wxID_OK, wxString::Format("Aceptar"));
    cancel_btn = new wxButton(this, wxID_CANCEL, wxString::Format("Cancelar"));

    // Vinculación segura
    ok_btn->Bind(wxEVT_BUTTON, &NewBookDialog::on_ok, this);
}

void NewBookDialog::_layout_controls()
{
    wxBoxSizer* main_dialog_sizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* form_content_sizer = new wxFlexGridSizer(0, 2, 10, 10);
    form_content_sizer->AddGrowableCol(1);

    form_content_sizer->Add(title_label, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    form_content_sizer->Add(title_ctrl, 1, wxEXPAND);

    form_content_sizer->Add(author_label, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    form_content_sizer->Add(author_ctrl, 1, wxEXPAND);

    form_content_sizer->Add(synopsis_label, 0, wxALIGN_RIGHT | wxALIGN_TOP | wxTOP, 5);
    form_content_sizer->Add(synopsis_ctrl, 1, wxEXPAND);

    form_panel->SetSizer(form_content_sizer);

    main_dialog_sizer->Add(form_panel, 1, wxEXPAND | wxALL, 15);

    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    button_sizer->Add(ok_btn, 0, wxRIGHT, 10);
    button_sizer->Add(cancel_btn, 0);

    main_dialog_sizer->Add(button_sizer, 0, wxALIGN_CENTER | wxBOTTOM, 15);

    this->SetSizer(main_dialog_sizer);
}

void NewBookDialog::on_ok(wxCommandEvent& event)
{
    // Captura de datos con blindaje de formato
    wxString title = wxString::Format("%s", title_ctrl->GetValue().Trim(true).Trim(false));
    wxString author = wxString::Format("%s", author_ctrl->GetValue().Trim(true).Trim(false));

    // Validación con mensajes blindados
    if (title.IsEmpty())
    {
        wxMessageBox(
            wxString::Format("Por favor, ingrese un título para el libro."),
            wxString::Format("Campo Requerido"),
            wxOK | wxICON_WARNING,
            this
        );
        title_ctrl->SetFocus();
        return;
    }

    if (author.IsEmpty())
    {
        wxMessageBox(
            wxString::Format("Por favor, ingrese un autor para el libro."),
            wxString::Format("Campo Requerido"),
            wxOK | wxICON_WARNING,
            this
        );
        author_ctrl->SetFocus();
        return;
    }

    this->EndModal(wxID_OK);
}

std::map<std::string, wxString> NewBookDialog::get_book_data()
{
    std::map<std::string, wxString> data;

    // Empaquetamiento final de datos usando Format para asegurar la integridad de la cadena
    // antes de ser enviada al AppHandler
    data["title"] = wxString::Format("%s", title_ctrl->GetValue().Trim());
    data["author"] = wxString::Format("%s", author_ctrl->GetValue().Trim());
    data["synopsis"] = wxString::Format("%s", synopsis_ctrl->GetValue().Trim());

    return data;
}