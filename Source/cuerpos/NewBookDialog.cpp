/**
* Archivo: NewBookDialog.cpp
* Descripción: Implementación del diálogo de creación de libros con validación y soporte UTF-8.
*/

#include "../encabezados/NewBookDialog.h"
#include "../encabezados/AppHandler.h"
#include <wx/sizer.h>
#include <wx/msgdlg.h>

wxBEGIN_EVENT_TABLE(NewBookDialog, wxDialog)
// El evento OK se vincula manualmente con Bind para mayor flexibilidad
wxEND_EVENT_TABLE()

NewBookDialog::NewBookDialog(wxWindow* parent, AppHandler* app_handler, const wxString& dialog_title)
    : wxDialog(parent, wxID_ANY, dialog_title, wxDefaultPosition, wxSize(500, 350)),
    app_handler(app_handler)
{
    _create_controls();
    _layout_controls();

    this->CentreOnParent();
}

void NewBookDialog::_create_controls()
{
    form_panel = new wxPanel(this, wxID_ANY);

    title_label = new wxStaticText(form_panel, wxID_ANY, "Título (*):");
    author_label = new wxStaticText(form_panel, wxID_ANY, "Autor (*):");
    synopsis_label = new wxStaticText(form_panel, wxID_ANY, "Sinopsis:");

    title_ctrl = new wxTextCtrl(form_panel, wxID_ANY);
    author_ctrl = new wxTextCtrl(form_panel, wxID_ANY);

    synopsis_ctrl = new wxTextCtrl(
        form_panel,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(-1, 100),
        wxTE_MULTILINE
    );

    ok_btn = new wxButton(this, wxID_OK, "Aceptar");
    cancel_btn = new wxButton(this, wxID_CANCEL, "Cancelar");

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
    wxString title = title_ctrl->GetValue().Trim(true).Trim(false);
    wxString author = author_ctrl->GetValue().Trim(true).Trim(false);

    if (title.IsEmpty())
    {
        wxMessageBox("Por favor, ingrese un título para el libro.",
            "Campo Requerido", wxOK | wxICON_WARNING, this);
        title_ctrl->SetFocus();
        return;
    }

    if (author.IsEmpty())
    {
        wxMessageBox("Por favor, ingrese un autor para el libro.",
            "Campo Requerido", wxOK | wxICON_WARNING, this);
        author_ctrl->SetFocus();
        return;
    }

    this->EndModal(wxID_OK);
}

std::map<std::string, wxString> NewBookDialog::get_book_data()
{
    std::map<std::string, wxString> data;

    // Se mantiene como wxString para que la conversión a UTF-8 se haga en AppHandler
    data["title"] = title_ctrl->GetValue().Trim();
    data["author"] = author_ctrl->GetValue().Trim();
    data["synopsis"] = synopsis_ctrl->GetValue().Trim();

    return data;
}