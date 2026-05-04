/**
* File Name: BookDetailsView.cpp
* Descripción: Implementación del panel de libro, con blindaje estricto UTF-8
*              y estandarización universal de wxString::Format para prevención de crashes.
*/

#include "../encabezados/BookDetailsView.h"
#include "../encabezados/AppHandler.h"
#include "../encabezados/Util.h"
#include <wx/sizer.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/mstream.h>

BookDetailsView::BookDetailsView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent, wxID_ANY),
    app_handler(app_handler),
    book_id(std::nullopt),
    _is_dirty_view(false),
    _loading_data(false),
    m_current_cover_image_data(std::nullopt)
{
    _create_controls();
    _layout_controls();
    load_book_details(std::nullopt);
}

void BookDetailsView::_create_controls()
{
    // Blindaje de literales estáticos con caracteres especiales usando FromUTF8
    title_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Título (*):"));
    author_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Autor (*):"));
    synopsis_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Sinopsis:"));
    prologue_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Prólogo:"));
    back_cover_text_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Texto de Contraportada:"));
    cover_image_label_text = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Imagen de Portada:"));

    title_ctrl = new wxTextCtrl(this, wxID_ANY);
    author_ctrl = new wxTextCtrl(this, wxID_ANY);
    synopsis_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE);
    prologue_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE);
    back_cover_text_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE);

    // Blindaje en la creación del placeholder
    wxBitmap placeholder = Util::CreatePlaceholderBitmap(100, 150, wxString::FromUTF8("Portada"));
    cover_image_display = new wxStaticBitmap(this, wxID_ANY, placeholder, wxDefaultPosition, wxSize(100, 150));

    wxTextCtrl* text_controls[] = { title_ctrl, author_ctrl, synopsis_ctrl, prologue_ctrl, back_cover_text_ctrl };
    for (auto ctrl : text_controls)
    {
        ctrl->Bind(wxEVT_TEXT, &BookDetailsView::on_text_changed, this);
    }

    cover_image_display->Bind(wxEVT_LEFT_DOWN, &BookDetailsView::on_image_clicked, this);
}

void BookDetailsView::_layout_controls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* form_sizer = new wxFlexGridSizer(0, 2, 10, 10);
    form_sizer->AddGrowableCol(1);

    form_sizer->Add(title_label, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    form_sizer->Add(title_ctrl, 1, wxEXPAND);

    form_sizer->Add(author_label, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    form_sizer->Add(author_ctrl, 1, wxEXPAND);

    form_sizer->Add(synopsis_label, 0, wxALIGN_RIGHT | wxALIGN_TOP | wxTOP, 5);
    form_sizer->Add(synopsis_ctrl, 1, wxEXPAND);

    form_sizer->Add(prologue_label, 0, wxALIGN_RIGHT | wxALIGN_TOP | wxTOP, 5);
    form_sizer->Add(prologue_ctrl, 1, wxEXPAND);

    form_sizer->Add(back_cover_text_label, 0, wxALIGN_RIGHT | wxALIGN_TOP | wxTOP, 5);
    form_sizer->Add(back_cover_text_ctrl, 1, wxEXPAND);

    form_sizer->Add(cover_image_label_text, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    form_sizer->Add(cover_image_display, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    main_sizer->Add(form_sizer, 1, wxEXPAND | wxALL, 10);
    this->SetSizer(main_sizer);

    _update_controls_state();
}

void BookDetailsView::set_view_dirty(bool is_dirty)
{
    if (_is_dirty_view != is_dirty)
    {
        _is_dirty_view = is_dirty;
        if (_is_dirty_view)
        {
            app_handler->set_dirty(true);
        }
    }
}

void BookDetailsView::_update_controls_state()
{
    bool are_controls_enabled = book_id.has_value();
    title_ctrl->Enable(are_controls_enabled);
    author_ctrl->Enable(are_controls_enabled);
    synopsis_ctrl->Enable(are_controls_enabled);
    prologue_ctrl->Enable(are_controls_enabled);
    back_cover_text_ctrl->Enable(are_controls_enabled);
    cover_image_display->Enable(are_controls_enabled);
}

void BookDetailsView::load_book_details(std::optional<int> id)
{
    _loading_data = true;
    book_id = id;
    m_current_cover_image_data = std::nullopt;

    if (!book_id.has_value())
    {
        title_ctrl->SetValue(wxEmptyString);
        author_ctrl->SetValue(wxEmptyString);
        synopsis_ctrl->SetValue(wxEmptyString);
        prologue_ctrl->SetValue(wxEmptyString);
        back_cover_text_ctrl->SetValue(wxEmptyString);
        cover_image_display->SetBitmap(Util::CreatePlaceholderBitmap(100, 150, wxString::FromUTF8("Portada")));
    }
    else
    {
        std::optional<DBRow> details_opt = app_handler->get_book_details(book_id.value());

        if (details_opt.has_value())
        {
            DBRow& data = details_opt.value();

            // Función Lambda refactorizada con el "Blindaje del Jefe"
            auto get_safe_string = [&](const std::string& key) -> wxString
                {
                    if (data.count(key) && std::holds_alternative<std::string>(data.at(key)))
                    {
                        // Extraemos el std::string, lo convertimos a UTF8 y lo aseguramos con Format
                        return wxString::Format("%s", wxString::FromUTF8(std::get<std::string>(data.at(key))));
                    }
                    return wxEmptyString;
                };

            title_ctrl->SetValue(get_safe_string("title"));
            author_ctrl->SetValue(get_safe_string("author"));
            synopsis_ctrl->SetValue(get_safe_string("synopsis"));
            prologue_ctrl->SetValue(get_safe_string("prologue"));
            back_cover_text_ctrl->SetValue(get_safe_string("back_cover_text"));

            if (data.count("cover_image_data") && std::holds_alternative<std::vector<uint8_t>>(data.at("cover_image_data")))
            {
                m_current_cover_image_data = std::get<std::vector<uint8_t>>(data.at("cover_image_data"));

                if (!m_current_cover_image_data->empty())
                {
                    wxMemoryInputStream stream(m_current_cover_image_data->data(), m_current_cover_image_data->size());
                    wxImage img;
                    if (img.LoadFile(stream, wxBITMAP_TYPE_ANY))
                    {
                        cover_image_display->SetBitmap(wxBitmap(img.Rescale(100, 150, wxIMAGE_QUALITY_HIGH)));
                    }
                    else
                    {
                        cover_image_display->SetBitmap(Util::CreatePlaceholderBitmap(100, 150, wxString::FromUTF8("Error")));
                    }
                }
                else
                {
                    cover_image_display->SetBitmap(Util::CreatePlaceholderBitmap(100, 150, wxString::FromUTF8("Portada")));
                }
            }
            else
            {
                cover_image_display->SetBitmap(Util::CreatePlaceholderBitmap(100, 150, wxString::FromUTF8("Portada")));
            }
        }
    }

    _is_dirty_view = false;
    _update_controls_state();
    _loading_data = false;
}

bool BookDetailsView::save_changes()
{
    if (!_is_dirty_view || !book_id.has_value())
    {
        return false;
    }

    // Recuperamos los valores y los blindamos inmediatamente con Format
    wxString title = wxString::Format("%s", title_ctrl->GetValue().Trim(true).Trim(false));
    wxString author = wxString::Format("%s", author_ctrl->GetValue().Trim(true).Trim(false));

    if (title.IsEmpty() || author.IsEmpty())
    {
        // MessageBox estandarizado y seguro
        wxMessageBox(
            wxString::Format("%s", wxString::FromUTF8("Título y autor son obligatorios.")),
            wxString::Format("%s", wxString::FromUTF8("Error de Validación")),
            wxOK | wxICON_WARNING,
            this
        );
        return false;
    }

    // Blindamos el resto de los campos de texto
    wxString synopsis = wxString::Format("%s", synopsis_ctrl->GetValue());
    wxString prologue = wxString::Format("%s", prologue_ctrl->GetValue());
    wxString back_cover = wxString::Format("%s", back_cover_text_ctrl->GetValue());

    std::vector<uint8_t> cover_data;
    if (m_current_cover_image_data.has_value())
    {
        cover_data = m_current_cover_image_data.value();
    }

    bool success = app_handler->update_book_details(
        book_id.value(),
        title,
        author,
        synopsis,
        prologue,
        back_cover,
        cover_data
    );

    if (success)
    {
        set_view_dirty(false);
        return true;
    }

    return false;
}

void BookDetailsView::on_text_changed(wxCommandEvent& event)
{
    if (_loading_data || !title_ctrl->IsEnabled())
    {
        event.Skip();
        return;
    }
    set_view_dirty(true);
    event.Skip();
}

void BookDetailsView::on_image_clicked(wxMouseEvent& event)
{
    if (!cover_image_display->IsEnabled()) return;

    // Literales blindados
    wxString wildcard = wxString::FromUTF8("Imágenes (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg");
    wxString dialog_title = wxString::Format("%s", wxString::FromUTF8("Seleccionar imagen de portada"));

    wxFileDialog dialog(this, dialog_title, wxEmptyString, wxEmptyString, wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dialog.ShowModal() == wxID_OK)
    {
        // La ruta ya viene como wxString internamente desde el S.O.
        wxString new_path = wxString::Format("%s", dialog.GetPath());

        wxFile file(new_path, wxFile::read);
        if (file.IsOpened())
        {
            size_t length = file.Length();
            std::vector<uint8_t> buffer(length);
            file.Read(buffer.data(), length);

            wxMemoryInputStream stream(buffer.data(), buffer.size());
            wxImage img;

            if (img.LoadFile(stream, wxBITMAP_TYPE_ANY))
            {
                m_current_cover_image_data = buffer;

                cover_image_display->SetBitmap(wxBitmap(img.Rescale(100, 150, wxIMAGE_QUALITY_HIGH)));
                set_view_dirty(true);
            }
            else
            {
                wxMessageBox(
                    wxString::Format("%s", wxString::FromUTF8("El archivo seleccionado no es una imagen válida o está corrupto.")),
                    wxString::Format("%s", wxString::FromUTF8("Error")),
                    wxOK | wxICON_ERROR,
                    this
                );
            }
        }
        else
        {
            wxMessageBox(
                wxString::Format("%s", wxString::FromUTF8("No se pudo abrir el archivo físico de la imagen.")),
                wxString::Format("%s", wxString::FromUTF8("Error de Lectura")),
                wxOK | wxICON_ERROR,
                this
            );
        }
    }
}

bool BookDetailsView::is_dirty() const
{
    return _is_dirty_view;
}

void BookDetailsView::enable_view(bool enable)
{
    this->Enable(enable);
    _update_controls_state();
}