/*
* Archivo: NewBookDialog.h
* Descripción: Define un diálogo para crear un nuevo libro con campos esenciales.
*/

#ifndef NEWBOOKDIALOG_H
#define NEWBOOKDIALOG_H

#include <wx/wx.h>
#include <map>
#include <string>

class AppHandler;

class NewBookDialog : public wxDialog
{
public:
    /**
     * Inicializa una nueva instancia del diálogo NewBookDialog.
     * @param parent: La ventana padre.
     * @param app_handler: Manejador de la lógica de aplicación.
     * @param dialog_title: El título de la ventana.
     */
    NewBookDialog(
        wxWindow* parent,
        AppHandler* app_handler,
        const wxString& dialog_title = "Nuevo Libro"
    );

    /**
     * Recopila y devuelve los datos ingresados por el usuario.
     * @return std::map con claves de string y valores de wxString para mantener la codificación.
     */
    std::map<std::string, wxString> get_book_data();

private:
    // Construcción de la interfaz
    void _create_controls();
    void _layout_controls();

    // Manejador del botón Aceptar (Validación)
    void on_ok(wxCommandEvent& event);

    // Lógica y datos
    AppHandler* app_handler;

    // Controles (Widgets)
    wxPanel* form_panel;
    wxStaticText* title_label;
    wxStaticText* author_label;
    wxStaticText* synopsis_label;

    wxTextCtrl* title_ctrl;
    wxTextCtrl* author_ctrl;
    wxTextCtrl* synopsis_ctrl;

    wxButton* ok_btn;
    wxButton* cancel_btn;

    wxDECLARE_EVENT_TABLE();
};

#endif // NEWBOOKDIALOG_H