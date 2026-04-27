/**
* File Name: BookDetailsView.cpp
* Description: Implementación del panel para mostrar y editar los detalles de un libro.
* Author: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#include "../encabezados/BookDetailsView.h"
#include "../encabezados/AppHandler.h"
#include "../encabezados/Util.h"
#include <wx/sizer.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/filename.h> // Necesario para comprobar la existencia de archivos de imagen

BookDetailsView::BookDetailsView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent), app_handler(app_handler), book_id(std::nullopt),
    _is_dirty_view(false), _loading_data(false), current_cover_image_path(std::nullopt)
{
    _create_controls();
    _layout_controls();

    // Carga inicial vacía
    load_book_details(std::nullopt);
}

void BookDetailsView::_create_controls() {
    // 1. Creación de Etiquetas
    title_label = new wxStaticText(this, wxID_ANY, "Título (*):");
    author_label = new wxStaticText(this, wxID_ANY, "Autor (*):");
    synopsis_label = new wxStaticText(this, wxID_ANY, "Sinopsis:");
    prologue_label = new wxStaticText(this, wxID_ANY, "Prólogo:");
    back_cover_text_label = new wxStaticText(this, wxID_ANY, "Texto de Contraportada:");
    cover_image_label_text = new wxStaticText(this, wxID_ANY, "Imagen de Portada:");

    // 2. Creación de Controles de Texto
    title_ctrl = new wxTextCtrl(this, wxID_ANY);
    author_ctrl = new wxTextCtrl(this, wxID_ANY);

    // Campos multilínea con altura inicial sugerida (-1 ancho dinámico, 80 alto fijo inicial)
    synopsis_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE);
    prologue_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE);
    back_cover_text_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE);

    // 3. Creación de la Imagen de Portada (Manejo correcto de ambigüedad C2668)
    wxBitmap placeholder = Util::CreatePlaceholderBitmap(100, 150, "Portada");
    cover_image_display = new wxStaticBitmap(this, wxID_ANY, placeholder, wxDefaultPosition, wxSize(100, 150));

    // 4. Vinculación dinámica de eventos (Sustituyendo a la tabla de eventos estática para evitar bugs)
    wxTextCtrl* text_controls[] = { title_ctrl, author_ctrl, synopsis_ctrl, prologue_ctrl, back_cover_text_ctrl };
    for (auto ctrl : text_controls) {
        ctrl->Bind(wxEVT_TEXT, &BookDetailsView::on_text_changed, this);
    }

    // El click sobre la imagen abre el diálogo de selección de archivo
    cover_image_display->Bind(wxEVT_LEFT_DOWN, &BookDetailsView::on_image_clicked, this);
}

void BookDetailsView::_layout_controls() {
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // FlexGridSizer para alinear etiquetas y controles en dos columnas
    wxFlexGridSizer* form_sizer = new wxFlexGridSizer(0, 2, 10, 10);
    form_sizer->AddGrowableCol(1); // La columna 1 (los text_ctrl) se expandirá

    // Título
    form_sizer->Add(title_label, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    form_sizer->Add(title_ctrl, 1, wxEXPAND);

    // Autor
    form_sizer->Add(author_label, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    form_sizer->Add(author_ctrl, 1, wxEXPAND);

    // Sinopsis
    form_sizer->Add(synopsis_label, 0, wxALIGN_RIGHT | wxALIGN_TOP | wxTOP, 5);
    form_sizer->Add(synopsis_ctrl, 1, wxEXPAND);

    // Prólogo
    form_sizer->Add(prologue_label, 0, wxALIGN_RIGHT | wxALIGN_TOP | wxTOP, 5);
    form_sizer->Add(prologue_ctrl, 1, wxEXPAND);

    // Contraportada
    form_sizer->Add(back_cover_text_label, 0, wxALIGN_RIGHT | wxALIGN_TOP | wxTOP, 5);
    form_sizer->Add(back_cover_text_ctrl, 1, wxEXPAND);

    // Imagen
    form_sizer->Add(cover_image_label_text, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    form_sizer->Add(cover_image_display, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    main_sizer->Add(form_sizer, 1, wxEXPAND | wxALL, 10);
    this->SetSizer(main_sizer);

    _update_controls_state();
}

void BookDetailsView::set_view_dirty(bool is_dirty) {
    if (_is_dirty_view != is_dirty) {
        _is_dirty_view = is_dirty;
        if (_is_dirty_view) {
            app_handler->set_dirty(true);
        }
    }
}

void BookDetailsView::_update_controls_state() {
    bool are_controls_enabled = book_id.has_value();
    title_ctrl->Enable(are_controls_enabled);
    author_ctrl->Enable(are_controls_enabled);
    synopsis_ctrl->Enable(are_controls_enabled);
    prologue_ctrl->Enable(are_controls_enabled);
    back_cover_text_ctrl->Enable(are_controls_enabled);
    cover_image_display->Enable(are_controls_enabled);
}

void BookDetailsView::load_book_details(std::optional<int> id) {
    _loading_data = true;
    book_id = id;
    current_cover_image_path = std::nullopt;

    if (!book_id.has_value()) {
        // Limpieza de campos
        title_ctrl->SetValue(wxEmptyString);
        author_ctrl->SetValue(wxEmptyString);
        synopsis_ctrl->SetValue(wxEmptyString);
        prologue_ctrl->SetValue(wxEmptyString);
        back_cover_text_ctrl->SetValue(wxEmptyString);
        cover_image_display->SetBitmap(Util::CreatePlaceholderBitmap(100, 150, "Portada"));
    }
    else {
        // Carga de datos desde la BD
        std::optional<DBRow> details_opt = app_handler->get_book_details(book_id.value());

        if (details_opt.has_value()) {
            DBRow& data = details_opt.value();

            // Función lambda para extraer strings seguros de los variants en UTF-8
            auto get_safe_string = [&](const std::string& key) -> wxString {
                if (data.count(key) && std::holds_alternative<std::string>(data.at(key))) {
                    return wxString::FromUTF8(std::get<std::string>(data.at(key)));
                }
                return wxEmptyString;
                };

            title_ctrl->SetValue(get_safe_string("title"));
            author_ctrl->SetValue(get_safe_string("author"));
            synopsis_ctrl->SetValue(get_safe_string("synopsis"));
            prologue_ctrl->SetValue(get_safe_string("prologue"));
            back_cover_text_ctrl->SetValue(get_safe_string("back_cover_text"));

            // Manejo de la imagen de portada
            wxString img_path = get_safe_string("cover_image_path");
            if (!img_path.IsEmpty() && wxFileName::FileExists(img_path)) {
                current_cover_image_path = img_path.ToStdString();
                std::optional<wxBitmap> bmp_opt = Util::LoadImage(img_path);

                if (bmp_opt.has_value() && bmp_opt->IsOk()) {
                    wxImage img = bmp_opt->ConvertToImage();
                    cover_image_display->SetBitmap(wxBitmap(img.Rescale(100, 150, wxIMAGE_QUALITY_HIGH)));
                }
                else {
                    cover_image_display->SetBitmap(Util::CreatePlaceholderBitmap(100, 150, "Error"));
                }
            }
            else {
                cover_image_display->SetBitmap(Util::CreatePlaceholderBitmap(100, 150, "Portada"));
            }
        }
    }

    _is_dirty_view = false;
    _update_controls_state();
    _loading_data = false;
}

bool BookDetailsView::save_changes() {
    if (!_is_dirty_view || !book_id.has_value()) {
        return false;
    }

    // Extracción y limpieza (trim) de los datos obligatorios
    wxString title = title_ctrl->GetValue().Trim(true).Trim(false);
    wxString author = author_ctrl->GetValue().Trim(true).Trim(false);

    if (title.IsEmpty() || author.IsEmpty()) {
        wxMessageBox(wxString::FromUTF8("Título y autor son obligatorios."), "Error", wxOK | wxICON_WARNING, this);
        return false;
    }

    // Preparación del resto de campos convirtiendo a UTF-8 estándar
    wxString synopsis = synopsis_ctrl->GetValue();
    wxString prologue = prologue_ctrl->GetValue();
    wxString back_cover = back_cover_text_ctrl->GetValue();

    // Ruta de imagen (si existe)
    // INCORRECTO: En C++, el operador ternario condicion ? A : B exige que tanto A como B sean del mismo tipo, o que uno pueda convertirse implícitamente al otro sin ambigüedad. Estás devolviendo wxString por un lado y wxEmptyString (que a menudo se define como un const wxChar* o un #define "") por el otro.
    // wxString cover_path = current_cover_image_path.has_value() ? wxString::FromUTF8(current_cover_image_path.value()) : wxEmptyString;
	// CORRECTO: En este caso, es mejor asegurarse de que ambos lados del operador ternario devuelvan un wxString. Puedes usar wxString() para representar una cadena vacía en lugar de wxEmptyString, lo que garantiza que ambos resultados sean del mismo tipo.
    wxString cover_path = current_cover_image_path.has_value() ? wxString::FromUTF8(current_cover_image_path.value()) : wxString();
    // Llamada al manejador
    bool success = app_handler->update_book_details(
        book_id.value(),
        title,
        author,
        synopsis,
        prologue,
        back_cover,
        cover_path
    );

    if (success) {
        set_view_dirty(false);
        return true;
    }

    return false;
}

void BookDetailsView::on_text_changed(wxCommandEvent& event) {
    if (_loading_data || !title_ctrl->IsEnabled()) {
        event.Skip();
        return;
    }
    set_view_dirty(true);
    event.Skip();
}

void BookDetailsView::on_image_clicked(wxMouseEvent& event) {
    if (!cover_image_display->IsEnabled()) return;

    // Filtros de extensión
    wxString wildcard = "Imágenes (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg";

    wxFileDialog dialog(this, "Seleccionar imagen de portada", wxEmptyString, wxEmptyString, wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dialog.ShowModal() == wxID_OK) {
        wxString new_path = dialog.GetPath();
        std::optional<wxBitmap> bmp_opt = Util::LoadImage(new_path);

        if (bmp_opt.has_value() && bmp_opt->IsOk()) {
            wxImage img = bmp_opt->ConvertToImage();
            cover_image_display->SetBitmap(wxBitmap(img.Rescale(100, 150, wxIMAGE_QUALITY_HIGH)));

            // Actualizar ruta y marcar como sucio si cambió
            std::string std_path = new_path.ToStdString();
            if (!current_cover_image_path.has_value() || current_cover_image_path.value() != std_path) {
                current_cover_image_path = std_path;
                set_view_dirty(true);
            }
        }
        else {
            wxMessageBox("No se pudo cargar la imagen seleccionada.", "Error", wxOK | wxICON_ERROR, this);
        }
    }
}

std::optional<std::string> BookDetailsView::get_current_image_path() const {
    return current_cover_image_path;
}

bool BookDetailsView::is_dirty() const {
    return _is_dirty_view;
}

void BookDetailsView::enable_view(bool enable) {
    this->Enable(enable);
    _update_controls_state();
}