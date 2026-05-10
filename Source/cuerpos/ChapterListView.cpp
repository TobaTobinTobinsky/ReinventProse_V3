/**
* File Name: ChapterListView.cpp
* Descripción: Implementación de la lista de capítulos.
*              CORRECCIÓN CRÍTICA: Protocolo C++20 (u8 literals) estricto para
*              evitar que los botones y etiquetas desaparezcan por fallos de decodificación.
* Autor: AutoDoc AI (Protocolo de Estabilidad C++20)
* Versión: 1.1.0
*/

#include "../encabezados/ChapterListView.h"
#include "../encabezados/AppHandler.h"
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

wxBEGIN_EVENT_TABLE(ChapterListView, wxPanel)
EVT_LISTBOX(wxID_ANY, ChapterListView::on_listbox_select)
EVT_LISTBOX_DCLICK(wxID_ANY, ChapterListView::on_listbox_dclick)
EVT_BUTTON(ID_ADD_CHAPTER, ChapterListView::on_add_chapter)
EVT_BUTTON(ID_EDIT_CHAPTER, ChapterListView::on_edit_chapter)
EVT_BUTTON(ID_DELETE_CHAPTER, ChapterListView::on_delete_chapter)
wxEND_EVENT_TABLE()

ChapterListView::ChapterListView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent), app_handler(app_handler), book_id(std::nullopt)
{
    _create_controls();
    _layout_controls();
    _update_button_states();
}

void ChapterListView::_create_controls()
{
    // PROTOCOLO C++20: Garantizar UTF-8 puro en memoria
    wxString lab_list = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Capítulos del Libro:")));
    list_label = new wxStaticText(this, wxID_ANY, lab_list);

    chapter_list_ctrl = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);

    // BLINDAJE DE BOTONES: Solución a los botones "Blancos/Vacíos"
    wxString btn_add = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Añadir Capítulo")));
    wxString btn_edit = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Modificar Título")));
    wxString btn_del = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Eliminar Capítulo")));

    add_chapter_button = new wxButton(this, ID_ADD_CHAPTER, btn_add);
    edit_chapter_button = new wxButton(this, ID_EDIT_CHAPTER, btn_edit);
    delete_chapter_button = new wxButton(this, ID_DELETE_CHAPTER, btn_del);
}

void ChapterListView::_layout_controls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    main_sizer->Add(list_label, 0, wxALL | wxEXPAND, 5);
    main_sizer->Add(chapter_list_ctrl, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    button_sizer->Add(add_chapter_button, 0, wxRIGHT, 5);
    button_sizer->Add(edit_chapter_button, 0, wxRIGHT, 5);
    button_sizer->Add(delete_chapter_button, 0, wxRIGHT, 5);

    main_sizer->Add(button_sizer, 0, wxALL | wxALIGN_LEFT, 5);
    this->SetSizer(main_sizer);
}

void ChapterListView::set_on_chapter_selected_callback(std::function<void(std::optional<int>)> callback)
{
    on_chapter_selected_callback = callback;
}

void ChapterListView::_update_button_states()
{
    bool has_book = book_id.has_value();
    int sel = chapter_list_ctrl->GetSelection();
    bool has_sel = (sel != wxNOT_FOUND);

    add_chapter_button->Enable(has_book);
    edit_chapter_button->Enable(has_book && has_sel);
    delete_chapter_button->Enable(has_book && has_sel);
    chapter_list_ctrl->Enable(has_book);
}

void ChapterListView::load_chapters(std::optional<int> id)
{
    this->book_id = id;
    chapter_list_ctrl->Clear();
    chapters_data.clear();

    if (!book_id.has_value()) {
        wxString lab_empty = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Capítulos: (Seleccione un libro)")));
        list_label->SetLabel(lab_empty);
        _update_button_states();
        if (on_chapter_selected_callback) {
            on_chapter_selected_callback(std::nullopt);
        }
        return;
    }

    auto book_opt = app_handler->get_book_details(book_id.value());
    std::string titleStr = "Desconocido";
    if (book_opt.has_value() && book_opt->count("title")) {
        titleStr = std::get<std::string>((*book_opt)["title"]);
    }

    wxString fmt_chapters = wxString::FromUTF8(reinterpret_cast<const char*>(u8"Capítulos de: %s"));
    wxString labelText = wxString::Format(fmt_chapters, wxString::FromUTF8(titleStr.c_str()));

    if (labelText.Length() > 30) {
        labelText = wxString::Format("%s...", labelText.Left(30));
    }
    list_label->SetLabel(labelText);

    chapters_data = app_handler->get_chapters_by_book_id(book_id.value());

    wxString fmt_item = wxString::FromUTF8(reinterpret_cast<const char*>(u8"Cap. %lld: %s"));

    for (auto& chapter : chapters_data) {
        long long ch_id = std::get<long long>(chapter["id"]);
        long long ch_num = std::get<long long>(chapter["chapter_number"]);
        std::string ch_title = std::get<std::string>(chapter["title"]);

        wxString display_text = wxString::Format(fmt_item, ch_num, wxString::FromUTF8(ch_title.c_str()));

        int pos = chapter_list_ctrl->Append(display_text);
        chapter_list_ctrl->SetClientData(pos, (void*)(uintptr_t)ch_id);
    }

    _update_button_states();

    if (chapter_list_ctrl->GetSelection() == wxNOT_FOUND && on_chapter_selected_callback) {
        on_chapter_selected_callback(std::nullopt);
    }
}

void ChapterListView::on_listbox_select(wxCommandEvent& event)
{
    int sel = chapter_list_ctrl->GetSelection();
    std::optional<int> selected_id = std::nullopt;

    if (sel != wxNOT_FOUND) {
        selected_id = (int)(uintptr_t)chapter_list_ctrl->GetClientData(sel);
    }

    if (on_chapter_selected_callback) {
        on_chapter_selected_callback(selected_id);
    }

    _update_button_states();
    event.Skip();
}

void ChapterListView::on_listbox_dclick(wxCommandEvent& event)
{
    on_listbox_select(event);
}

void ChapterListView::on_add_chapter(wxCommandEvent& event)
{
    if (!book_id.has_value()) return;

    wxString dlg_msg = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Título del nuevo capítulo:")));
    wxString dlg_cap = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Añadir Capítulo")));

    wxTextEntryDialog dlg(this, dlg_msg, dlg_cap);

    if (dlg.ShowModal() == wxID_OK) {
        wxString title = wxString::Format("%s", dlg.GetValue().Trim(true).Trim(false));

        if (title.IsEmpty()) {
            wxString err_msg = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"El título del capítulo no puede estar vacío.")));
            wxString err_cap = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Error de Validación")));
            wxMessageBox(err_msg, err_cap, wxOK | wxICON_ERROR, this);
            return;
        }

        int next_num = 1;
        if (!chapters_data.empty()) {
            int max_val = 0;
            for (auto& c : chapters_data) {
                int n = (int)std::get<long long>(c["chapter_number"]);
                if (n > max_val) max_val = n;
            }
            next_num = max_val + 1;
        }

        auto new_id = app_handler->create_new_chapter(book_id.value(), next_num, title);

        if (new_id.has_value()) {
            app_handler->set_dirty(false);
            load_chapters(book_id);
            select_chapter_by_id(new_id.value());
        }
    }
}

void ChapterListView::on_edit_chapter(wxCommandEvent& event)
{
    int sel = chapter_list_ctrl->GetSelection();
    if (!book_id.has_value() || sel == wxNOT_FOUND) return;

    int ch_id = (int)(uintptr_t)chapter_list_ctrl->GetClientData(sel);
    wxString current_title = "";

    for (auto& c : chapters_data) {
        if ((int)std::get<long long>(c["id"]) == ch_id) {
            current_title = wxString::FromUTF8(std::get<std::string>(c["title"]).c_str());
            break;
        }
    }

    wxString dlg_msg = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Introduzca el nuevo título para el capítulo:")));
    wxString dlg_cap = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Modificar Título")));

    wxTextEntryDialog dlg(this, dlg_msg, dlg_cap, current_title);

    if (dlg.ShowModal() == wxID_OK) {
        wxString new_t = wxString::Format("%s", dlg.GetValue().Trim(true).Trim(false));

        if (new_t.IsEmpty()) {
            wxString err_msg = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"El título del capítulo no puede estar vacío.")));
            wxString err_cap = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Error")));
            wxMessageBox(err_msg, err_cap, wxOK | wxICON_ERROR, this);
            return;
        }

        if (new_t == current_title) return;

        if (app_handler->update_chapter_title(ch_id, new_t)) {
            app_handler->set_dirty(false);
            load_chapters(book_id);
            select_chapter_by_id(ch_id);
        }
    }
}

void ChapterListView::on_delete_chapter(wxCommandEvent& event)
{
    int sel = chapter_list_ctrl->GetSelection();
    if (!book_id.has_value() || sel == wxNOT_FOUND) return;

    int ch_id = (int)(uintptr_t)chapter_list_ctrl->GetClientData(sel);
    wxString text = chapter_list_ctrl->GetString(sel);

    wxString fmt_msg = wxString::FromUTF8(reinterpret_cast<const char*>(u8"¿Está seguro de que desea eliminar el capítulo:\n\n'%s'?\n\nEsta acción es irreversible y también eliminará todo su contenido e ideas asociadas."));
    wxString msg = wxString::Format(fmt_msg, text);

    wxString cap = wxString::Format("%s", wxString::FromUTF8(reinterpret_cast<const char*>(u8"Confirmar Eliminación")));

    wxMessageDialog dlg(this, msg, cap, wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);

    if (dlg.ShowModal() == wxID_YES) {
        if (app_handler->delete_chapter(ch_id)) {
            app_handler->set_dirty(false);
            load_chapters(book_id);
        }
    }
}

std::optional<int> ChapterListView::get_selected_chapter_id() const
{
    int sel = chapter_list_ctrl->GetSelection();
    if (sel == wxNOT_FOUND) return std::nullopt;
    return (int)(uintptr_t)chapter_list_ctrl->GetClientData(sel);
}

void ChapterListView::select_chapter_by_id(std::optional<int> id)
{
    if (!id.has_value()) {
        int current_selection = chapter_list_ctrl->GetSelection();
        if (current_selection != wxNOT_FOUND) {
            chapter_list_ctrl->SetSelection(wxNOT_FOUND);
            wxCommandEvent evt(wxEVT_LISTBOX, chapter_list_ctrl->GetId());
            evt.SetEventObject(chapter_list_ctrl);
            chapter_list_ctrl->GetEventHandler()->ProcessEvent(evt);
        }
        _update_button_states();
        return;
    }

    for (unsigned int i = 0; i < chapter_list_ctrl->GetCount(); i++) {
        if ((int)(uintptr_t)chapter_list_ctrl->GetClientData(i) == id.value()) {
            if (chapter_list_ctrl->GetSelection() != i) {
                chapter_list_ctrl->SetSelection(i);
                wxCommandEvent evt(wxEVT_LISTBOX, chapter_list_ctrl->GetId());
                evt.SetEventObject(chapter_list_ctrl);
                chapter_list_ctrl->GetEventHandler()->ProcessEvent(evt);
            }
            else {
                _update_button_states();
            }
            return;
        }
    }

    if (chapter_list_ctrl->GetSelection() != wxNOT_FOUND) {
        chapter_list_ctrl->SetSelection(wxNOT_FOUND);
        wxCommandEvent evt(wxEVT_LISTBOX, chapter_list_ctrl->GetId());
        evt.SetEventObject(chapter_list_ctrl);
        chapter_list_ctrl->GetEventHandler()->ProcessEvent(evt);
    }
    _update_button_states();
}