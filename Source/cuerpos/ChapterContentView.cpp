/**
* Archivo: ChapterContentView.cpp
* Descripción: Implementación del editor de texto plano compatible con UTF-8.
*/

#include "../encabezados/ChapterContentView.h"
#include "../encabezados/AppHandler.h"
#include "../encabezados/Util.h"
#include <wx/sizer.h>
#include <wx/settings.h>

wxBEGIN_EVENT_TABLE(ChapterContentView, wxPanel)
EVT_TEXT(wxID_ANY, ChapterContentView::on_text_changed)
EVT_TOOL(ID_EDIT_MODE_TOOL, ChapterContentView::on_edit_button_click)
wxEND_EVENT_TABLE()

ChapterContentView::ChapterContentView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent), app_handler(app_handler), chapter_id(std::nullopt),
    _is_dirty_view(false), _loading_data(false), _is_in_edit_mode(false)
{
    _create_controls();
    _layout_controls();

    // Carga inicial
    load_content(std::nullopt);
}

void ChapterContentView::_create_controls() {
    // Etiqueta superior
    content_label = new wxStaticText(this, wxID_ANY, "Contenido del Capítulo:");

    // El Editor de texto real (Como un Bloc de Notas)
    // Usamos wxTE_MULTILINE para varias líneas y wxTE_RICH2 para mejor manejo de archivos grandes
    content_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxTE_RICH2);

    // Fuente recomendada para lectura/escritura (Consolas o Arial)
    wxFont writingFont(12, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    content_ctrl->SetFont(writingFont);
    content_ctrl->SetEditable(false);

    // Barra de herramientas simplificada (Solo el botón de Editar/Guardar)
    edit_mode_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT);

    // Cargamos el icono usando tu función Util
    wxBitmap editIcon = load_icon_bitmap("edit.png", wxSize(24, 24));
    edit_mode_toolbar->AddTool(ID_EDIT_MODE_TOOL, "Modo Edición", editIcon, "Activar/Desactivar escritura");
    edit_mode_toolbar->Realize();
}

void ChapterContentView::_layout_controls() {
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // 1. Barra de Herramientas
    main_sizer->Add(edit_mode_toolbar, 0, wxEXPAND);

    // 2. Etiqueta
    main_sizer->Add(content_label, 0, wxALL | wxEXPAND, 5);

    // 3. El Editor (que ocupe todo el espacio)
    main_sizer->Add(content_ctrl, 1, wxEXPAND | wxALL, 5);

    this->SetSizer(main_sizer);
}

void ChapterContentView::load_content(std::optional<int> id) {
    _loading_data = true;
    chapter_id = id;
    content_ctrl->Clear();

    if (chapter_id.has_value()) {
        content_label->SetLabel("Escribiendo capítulo...");

        auto details_opt = app_handler->get_chapter_details(chapter_id.value());
        if (details_opt.has_value()) {
            DBRow details = details_opt.value();
            if (details.count("content")) {
                DBValue val = details["content"];
                if (std::holds_alternative<std::string>(val)) {
                    // Convertimos de std::string (UTF-8 de la BD) a wxString para la pantalla
                    wxString textoLimpio = wxString::FromUTF8(std::get<std::string>(val));
                    content_ctrl->SetValue(textoLimpio);
                }
            }
        }
    }
    else {
        content_label->SetLabel("Contenido: (Seleccione un capítulo)");
    }

    content_ctrl->SetInsertionPoint(0);
    _is_dirty_view = false;
    _is_in_edit_mode = false;
    _update_edit_mode_ui();
    _loading_data = false;
}

bool ChapterContentView::save_changes() {
    if (!_is_dirty_view || !chapter_id.has_value()) {
        return true;
    }

    // Obtenemos el texto y lo convertimos a UTF-8 real antes de mandarlo a la base de datos
    std::string textoParaBD = content_ctrl->GetValue().ToUTF8().data();

    bool success = app_handler->update_chapter_content_via_handler(
        chapter_id.value(),
        wxString::FromUTF8(textoParaBD)
    );

    if (success) {
        _is_dirty_view = false;
    }
    return success;
}

void ChapterContentView::on_edit_button_click(wxCommandEvent& event) {
    if (!chapter_id.has_value()) return;

    if (_is_in_edit_mode) {
        // Al salir del modo edición, guardamos automáticamente
        if (_is_dirty_view) {
            save_changes();
        }
        _is_in_edit_mode = false;
    }
    else {
        _is_in_edit_mode = true;
        content_ctrl->SetFocus();
    }
    _update_edit_mode_ui();
}

void ChapterContentView::_update_edit_mode_ui() {
    bool can_edit = _is_in_edit_mode && chapter_id.has_value();

    content_ctrl->SetEditable(can_edit);

    // Cambiamos el color de fondo para que el usuario sepa que puede escribir (un amarillo crema suave)
    if (can_edit) {
        content_ctrl->SetBackgroundColour(wxColour(255, 255, 235));
    }
    else {
        content_ctrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    }

    // Actualizamos el texto del botón en la barra
	// INCORRECTO: No existe SetToolLabel en wxToolBar, debemos buscar el Tool y cambiar su etiqueta
    // edit_mode_toolbar->SetToolLabel(ID_EDIT_MODE_TOOL, can_edit ? "Finalizar y Guardar" : "Modo Edición");
    // CORRECTO: En C++ buscamos el objeto Tool y modificamos su etiqueta y tooltip
    wxToolBarToolBase* tool = edit_mode_toolbar->FindById(ID_EDIT_MODE_TOOL);
    if (tool) {
        tool->SetLabel(can_edit ? "Finalizar y Guardar" : "Modo Edición");
        tool->SetShortHelp(can_edit ? "Finalizar y Guardar" : "Modo Edición");
        edit_mode_toolbar->Realize(); // Obliga a redibujar la barra
    }


    content_ctrl->Refresh();
}

void ChapterContentView::on_text_changed(wxCommandEvent& event) {
    if (!_loading_data && _is_in_edit_mode) {
        set_view_dirty(true);
    }
    event.Skip();
}

void ChapterContentView::set_view_dirty(bool is_dirty) {
    if (_is_dirty_view != is_dirty) {
        _is_dirty_view = is_dirty;
        if (_is_dirty_view) {
            app_handler->set_dirty(true);
        }
    }
}

void ChapterContentView::enable_view(bool enable) {
    this->Enable(enable);
}

bool ChapterContentView::is_dirty() const { return _is_dirty_view; }
bool ChapterContentView::is_editable() const { return _is_in_edit_mode; }
bool ChapterContentView::force_save_if_dirty() { return save_changes(); }