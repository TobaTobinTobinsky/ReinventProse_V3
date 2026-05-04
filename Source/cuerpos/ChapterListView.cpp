/**
* File Name: ChapterListView.cpp
* Descripción: Implementación de la lista de capítulos con manejo robusto y seguro UTF-8.
*              Refactorizado para usar exclusivamente wxString::Format y evitar corrupción por concatenación.
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

void ChapterListView::_create_controls() {
    // Uso de FromUTF8 para proteger las tildes estáticas
    list_label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Capítulos del Libro:"));

    // Creamos la lista y le indicamos a wxWidgets que solo se puede seleccionar 1 ítem a la vez
    chapter_list_ctrl = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);

    // Botones blindados contra problemas de codificación
    add_chapter_button = new wxButton(this, ID_ADD_CHAPTER, wxString::FromUTF8("Añadir Capítulo"));
    edit_chapter_button = new wxButton(this, ID_EDIT_CHAPTER, wxString::FromUTF8("Modificar Título"));
    delete_chapter_button = new wxButton(this, ID_DELETE_CHAPTER, wxString::FromUTF8("Eliminar Capítulo"));
}

void ChapterListView::_layout_controls() {
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Margen superior
    main_sizer->Add(list_label, 0, wxALL | wxEXPAND, 5);

    // La lista se expande en toda el área central (Proporción 1)
    main_sizer->Add(chapter_list_ctrl, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // Contenedor de botones horizontales
    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    button_sizer->Add(add_chapter_button, 0, wxRIGHT, 5);
    button_sizer->Add(edit_chapter_button, 0, wxRIGHT, 5);
    button_sizer->Add(delete_chapter_button, 0, wxRIGHT, 5);

    main_sizer->Add(button_sizer, 0, wxALL | wxALIGN_LEFT, 5);
    this->SetSizer(main_sizer);
}

void ChapterListView::set_on_chapter_selected_callback(std::function<void(std::optional<int>)> callback) {
    on_chapter_selected_callback = callback;
}

void ChapterListView::_update_button_states() {
    bool has_book = book_id.has_value();
    int sel = chapter_list_ctrl->GetSelection();
    bool has_sel = (sel != wxNOT_FOUND);

    add_chapter_button->Enable(has_book);
    edit_chapter_button->Enable(has_book && has_sel);
    delete_chapter_button->Enable(has_book && has_sel);
    chapter_list_ctrl->Enable(has_book);
}

void ChapterListView::load_chapters(std::optional<int> id) {
    this->book_id = id;
    chapter_list_ctrl->Clear();
    chapters_data.clear();

    if (!book_id.has_value()) {
        list_label->SetLabel(wxString::FromUTF8("Capítulos: (Seleccione un libro)"));
        _update_button_states();
        if (on_chapter_selected_callback) {
            on_chapter_selected_callback(std::nullopt);
        }
        return;
    }

    // Actualizar la etiqueta visual con el nombre del libro seleccionado
    auto book_opt = app_handler->get_book_details(book_id.value());
    std::string titleStr = "Desconocido";
    if (book_opt.has_value() && book_opt->count("title")) {
        // Obtenemos el título desde la base de datos (Garantizado std::string)
        titleStr = std::get<std::string>((*book_opt)["title"]);
    }

    // TÉCNICA NINJA: En vez de concatenar, usamos Format combinando constantes UTF-8 y variables blindadas
    wxString labelText = wxString::Format(
        wxString::FromUTF8("Capítulos de: %s"),
        wxString::FromUTF8(titleStr)
    );

    if (labelText.Length() > 30) {
        // Reemplazo de `labelText.Left(30) + "..."` por Format puro
        labelText = wxString::Format("%s...", labelText.Left(30));
    }
    list_label->SetLabel(labelText);

    // Obtener los capítulos del AppHandler (Devuelve std::vector<DBRow>)
    chapters_data = app_handler->get_chapters_by_book_id(book_id.value());

    for (auto& chapter : chapters_data) {
        // En SQLite y DBManager definimos que los enteros grandes vienen como 'long long'
        long long ch_id = std::get<long long>(chapter["id"]);
        long long ch_num = std::get<long long>(chapter["chapter_number"]);
        std::string ch_title = std::get<std::string>(chapter["title"]);

        // Construcción blindada de la fila de la lista
        wxString display_text = wxString::Format(
            wxString::FromUTF8("Cap. %lld: %s"),
            ch_num,
            wxString::FromUTF8(ch_title)
        );

        // Agregar el item a la lista
        int pos = chapter_list_ctrl->Append(display_text);

        // TRUCO C++: Guardar el ID de base de datos como ClientData (Puntero falso) para recuperarlo luego
        chapter_list_ctrl->SetClientData(pos, (void*)(uintptr_t)ch_id);
    }

    _update_button_states();

    // Si al cargar no hay nada seleccionado, notificamos a la vista principal
    if (chapter_list_ctrl->GetSelection() == wxNOT_FOUND && on_chapter_selected_callback) {
        on_chapter_selected_callback(std::nullopt);
    }
}

void ChapterListView::on_listbox_select(wxCommandEvent& event) {
    int sel = chapter_list_ctrl->GetSelection();
    std::optional<int> selected_id = std::nullopt;

    // Si hay un item seleccionado, recuperamos su ID desde el puntero falso
    if (sel != wxNOT_FOUND) {
        selected_id = (int)(uintptr_t)chapter_list_ctrl->GetClientData(sel);
    }

    // Notificamos al MainWindow
    if (on_chapter_selected_callback) {
        on_chapter_selected_callback(selected_id);
    }

    _update_button_states();
    event.Skip();
}

void ChapterListView::on_listbox_dclick(wxCommandEvent& event) {
    on_listbox_select(event);
}

void ChapterListView::on_add_chapter(wxCommandEvent& event) {
    if (!book_id.has_value()) return;

    // Diálogos blindados con FromUTF8
    wxTextEntryDialog dlg(
        this,
        wxString::FromUTF8("Título del nuevo capítulo:"),
        wxString::FromUTF8("Añadir Capítulo")
    );

    if (dlg.ShowModal() == wxID_OK) {
        // BLINDAJE FORMAT PARA LAS TILDES AL EXTRAER VALORES
        wxString title = wxString::Format("%s", dlg.GetValue().Trim(true).Trim(false));

        if (title.IsEmpty()) {
            wxMessageBox(
                wxString::FromUTF8("El título del capítulo no puede estar vacío."),
                wxString::FromUTF8("Error de Validación"),
                wxOK | wxICON_ERROR,
                this
            );
            return;
        }

        // Calcular qué número de capítulo le toca a este (Busca el mayor y suma 1)
        int next_num = 1;
        if (!chapters_data.empty()) {
            int max_val = 0;
            for (auto& c : chapters_data) {
                int n = (int)std::get<long long>(c["chapter_number"]);
                if (n > max_val) max_val = n;
            }
            next_num = max_val + 1;
        }

        // Guardar en Base de Datos
        auto new_id = app_handler->create_new_chapter(book_id.value(), next_num, title);

        if (new_id.has_value()) {
            app_handler->set_dirty(false);

            // Refrescar la lista de la pantalla para que aparezca el nuevo capítulo
            load_chapters(book_id);

            // Seleccionar el capítulo recién creado
            select_chapter_by_id(new_id.value());
        }
    }
}

void ChapterListView::on_edit_chapter(wxCommandEvent& event) {
    int sel = chapter_list_ctrl->GetSelection();
    if (!book_id.has_value() || sel == wxNOT_FOUND) return;

    // Buscar el ID y título actual
    int ch_id = (int)(uintptr_t)chapter_list_ctrl->GetClientData(sel);
    wxString current_title = "";

    for (auto& c : chapters_data) {
        if ((int)std::get<long long>(c["id"]) == ch_id) {
            current_title = wxString::FromUTF8(std::get<std::string>(c["title"]));
            break;
        }
    }

    // Mostrar el diálogo de edición con el texto pre-rellenado y títulos blindados
    wxTextEntryDialog dlg(
        this,
        wxString::FromUTF8("Introduzca el nuevo título para el capítulo:"),
        wxString::FromUTF8("Modificar Título"),
        current_title
    );

    if (dlg.ShowModal() == wxID_OK) {
        // BLINDAJE FORMAT AL RECIBIR DATOS DEL USUARIO
        wxString new_t = wxString::Format("%s", dlg.GetValue().Trim(true).Trim(false));

        if (new_t.IsEmpty()) {
            wxMessageBox(
                wxString::FromUTF8("El título del capítulo no puede estar vacío."),
                wxString::Format("%s", "Error"),
                wxOK | wxICON_ERROR,
                this
            );
            return;
        }

        // Si el título es idéntico al original, no hacemos nada
        if (new_t == current_title) return;

        // Actualizar Base de datos
        if (app_handler->update_chapter_title(ch_id, new_t)) {
            app_handler->set_dirty(false);

            // Refrescar UI y volver a seleccionar el mismo capítulo modificado
            load_chapters(book_id);
            select_chapter_by_id(ch_id);
        }
    }
}

void ChapterListView::on_delete_chapter(wxCommandEvent& event) {
    int sel = chapter_list_ctrl->GetSelection();
    if (!book_id.has_value() || sel == wxNOT_FOUND) return;

    int ch_id = (int)(uintptr_t)chapter_list_ctrl->GetClientData(sel);
    wxString text = chapter_list_ctrl->GetString(sel);

    // ELIMINACIÓN DE LA CONCATENACIÓN CRÍTICA ('+' DESTERRADO)
    // Uso de wxString::Format inyectando la variable (text) de manera segura
    wxString msg = wxString::Format(
        wxString::FromUTF8("¿Está seguro de que desea eliminar el capítulo:\n\n'%s'?\n\nEsta acción es irreversible y también eliminará todo su contenido e ideas asociadas."),
        text
    );

    wxMessageDialog dlg(
        this,
        msg,
        wxString::FromUTF8("Confirmar Eliminación"),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
    );

    // Si el usuario acepta, procedemos al borrado (La cascada SQL borra el resto)
    if (dlg.ShowModal() == wxID_YES) {
        if (app_handler->delete_chapter(ch_id)) {
            app_handler->set_dirty(false);
            load_chapters(book_id);
        }
    }
}

std::optional<int> ChapterListView::get_selected_chapter_id() const {
    int sel = chapter_list_ctrl->GetSelection();
    if (sel == wxNOT_FOUND) return std::nullopt;
    return (int)(uintptr_t)chapter_list_ctrl->GetClientData(sel);
}

void ChapterListView::select_chapter_by_id(std::optional<int> id) {
    if (!id.has_value()) {
        int current_selection = chapter_list_ctrl->GetSelection();
        if (current_selection != wxNOT_FOUND) {
            chapter_list_ctrl->SetSelection(wxNOT_FOUND);

            // Avisar manualmente que se deseleccionó
            wxCommandEvent evt(wxEVT_LISTBOX, chapter_list_ctrl->GetId());
            evt.SetEventObject(chapter_list_ctrl);
            chapter_list_ctrl->GetEventHandler()->ProcessEvent(evt);
        }
        _update_button_states();
        return;
    }

    // Buscar en la lista el item que tenga el ClientData igual al ID buscado
    for (unsigned int i = 0; i < chapter_list_ctrl->GetCount(); i++) {
        if ((int)(uintptr_t)chapter_list_ctrl->GetClientData(i) == id.value()) {
            if (chapter_list_ctrl->GetSelection() != i) {
                chapter_list_ctrl->SetSelection(i);

                // Forzar el evento de selección para que MainWindow reaccione
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

    // Si llegó hasta aquí, el capítulo no existe en la UI, limpiamos
    if (chapter_list_ctrl->GetSelection() != wxNOT_FOUND) {
        chapter_list_ctrl->SetSelection(wxNOT_FOUND);
        wxCommandEvent evt(wxEVT_LISTBOX, chapter_list_ctrl->GetId());
        evt.SetEventObject(chapter_list_ctrl);
        chapter_list_ctrl->GetEventHandler()->ProcessEvent(evt);
    }
    _update_button_states();
}