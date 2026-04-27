/**
* File Name: AppHandler.cpp
* Descripción: Implementación del manejador de lógica y puente entre UI y DB.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
*/

#include "../encabezados/AppHandler.h"
#include <wx/msgdlg.h>

// Inicialización del puntero estático de la instancia Singleton
AppHandler* AppHandler::_instance = nullptr;

AppHandler* AppHandler::get_instance(DBManager* db_manager, const wxString& db_name) {
    if (!_instance) {
        // Si no se proporciona un DBManager, se intenta obtener o crear uno
        DBManager* db_mngr = db_manager ? db_manager : DBManager::get_instance(db_name.ToStdString());
        _instance = new AppHandler(db_mngr, db_name);
    }
    return _instance;
}

AppHandler::AppHandler(DBManager* db_manager, const wxString& db_name)
    : db_manager(db_manager), main_window(nullptr), _is_dirty(false), _initialized(true) {
}

void AppHandler::initialize_database() {
    try {
        db_manager->create_database();
    }
    catch (const DatabaseError& e) {
        wxMessageBox(wxString::Format("Error al inicializar la base de datos: %s", e.what()),
            "Error de Base de Datos", wxOK | wxICON_ERROR);
        exit(1); // Error crítico, la app no puede seguir sin DB
    }
}

void AppHandler::set_main_window(wxFrame* window) {
    this->main_window = window;
}

wxFrame* AppHandler::get_main_window() const {
    return this->main_window;
}

// --- MÉTODOS PARA LIBROS ---

std::optional<int> AppHandler::create_new_book(const wxString& title, const wxString& author, const wxString& synopsis,
    const wxString& prologue, const wxString& back_cover_text, const wxString& cover_image_path) {
    try {
        int book_id = db_manager->create_book(
            title.ToStdString(),
            author.ToStdString(),
            synopsis.ToStdString(),
            prologue.ToStdString(),
            back_cover_text.ToStdString(),
            cover_image_path.ToStdString()
        );
        if (book_id > 0) {
            set_dirty(true);
            return book_id;
        }
    }
    catch (const BookCreationError& e) {
        wxMessageBox(wxString::Format("Error al crear el libro: %s", e.what()),
            "Error de Creación de Libro", wxOK | wxICON_ERROR, main_window);
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error inesperado: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
    }
    return std::nullopt;
}

std::vector<DBRow> AppHandler::get_all_books() {
    try {
        return db_manager->get_all_books();
    }
    catch (const DatabaseError& e) {
        wxMessageBox(wxString::Format("Error al obtener libros: %s", e.what()),
            "Error de Base de Datos", wxOK | wxICON_ERROR, main_window);
        return {};
    }
}

std::optional<DBRow> AppHandler::get_book_details(int book_id) {
    try {
        return db_manager->get_book_by_id(book_id);
    }
    catch (const BookNotFoundError&) {
        return std::nullopt;
    }
    catch (const DatabaseError& e) {
        wxMessageBox(wxString::Format("Error al obtener detalles: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return std::nullopt;
    }
}

bool AppHandler::update_book_details(int book_id, const wxString& title, const wxString& author, const wxString& synopsis,
    const wxString& prologue, const wxString& back_cover_text, const wxString& cover_image_path) {
    try {
        return db_manager->update_book(
            book_id,
            title.ToStdString(),
            author.ToStdString(),
            synopsis.ToStdString(),
            prologue.ToStdString(),
            back_cover_text.ToStdString(),
            cover_image_path.ToStdString()
        );
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error al actualizar libro: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return false;
    }
}

// --- MÉTODOS PARA CAPÍTULOS ---

std::vector<DBRow> AppHandler::get_chapters_by_book_id(int book_id) {
    try {
        return db_manager->get_chapters_by_book_id(book_id);
    }
    catch (const DatabaseError& e) {
        wxMessageBox(wxString::Format("Error al obtener capítulos: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return {};
    }
}

std::optional<DBRow> AppHandler::get_chapter_details(int chapter_id) {
    try {
        return db_manager->get_chapter_by_id(chapter_id);
    }
    catch (const ChapterNotFoundError&) {
        return std::nullopt;
    }
    catch (const DatabaseError& e) {
        wxMessageBox(wxString::Format("Error al obtener detalles del capítulo: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return std::nullopt;
    }
}

std::optional<int> AppHandler::create_new_chapter(int book_id, int chapter_number, const wxString& title,
    const wxString& content, const wxString& abstract_idea) {
    try {
        int chapter_id = db_manager->create_chapter(
            book_id,
            chapter_number,
            title.ToStdString(),
            content.ToStdString(),
            abstract_idea.ToStdString()
        );
        if (chapter_id > 0) {
            set_dirty(true);
            return chapter_id;
        }
    }
    catch (const ChapterCreationError& e) {
        wxMessageBox(wxString::Format("Error al crear capítulo: %s", e.what()),
            "Error de Creación", wxOK | wxICON_ERROR, main_window);
    }
    return std::nullopt;
}

bool AppHandler::delete_chapter(int chapter_id) {
    try {
        if (db_manager->delete_chapter(chapter_id)) {
            set_dirty(true);
            return true;
        }
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error al eliminar capítulo: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
    }
    return false;
}

bool AppHandler::update_chapter_title(int chapter_id, const wxString& new_title) {
    try {
        return db_manager->update_chapter_title(chapter_id, new_title.ToStdString());
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error al actualizar título: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return false;
    }
}

bool AppHandler::update_chapter_content_via_handler(int chapter_id, const wxString& content) {
    try {
        return db_manager->update_chapter_content_only(chapter_id, content.ToStdString());
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error al actualizar contenido: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return false;
    }
}

bool AppHandler::update_chapter_abstract_idea_via_handler(int chapter_id, const wxString& abstract_idea) {
    try {
        return db_manager->update_chapter_abstract_idea(chapter_id, abstract_idea.ToStdString());
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error al actualizar idea: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return false;
    }
}

// --- MÉTODOS PARA IDEAS CONCRETAS ---

std::vector<DBRow> AppHandler::get_concrete_ideas_for_chapter(int chapter_id) {
    try {
        return db_manager->get_concrete_ideas_by_chapter_id(chapter_id);
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error al obtener ideas: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return {};
    }
}

std::optional<int> AppHandler::add_concrete_idea_for_chapter(int chapter_id, const wxString& idea_text) {
    try {
        int id = db_manager->add_concrete_idea(chapter_id, idea_text.ToStdString());
        return id;
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error al agregar idea: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return std::nullopt;
    }
}

bool AppHandler::update_concrete_idea_text(int concrete_idea_id, const wxString& new_text) {
    try {
        return db_manager->update_concrete_idea(concrete_idea_id, new_text.ToStdString());
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error al actualizar idea: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return false;
    }
}

bool AppHandler::delete_concrete_idea_by_id(int concrete_idea_id) {
    try {
        return db_manager->delete_concrete_idea(concrete_idea_id);
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error al eliminar idea: %s", e.what()),
            "Error", wxOK | wxICON_ERROR, main_window);
        return false;
    }
}

// --- GESTIÓN DEL ESTADO 'DIRTY' ---

void AppHandler::set_dirty(bool is_dirty) {
    if (this->_is_dirty != is_dirty) {
        this->_is_dirty = is_dirty;
        // La actualización visual del título se maneja mediante comunicación directa con MainWindow
        // Esto se disparará desde MainWindow llamando a is_application_dirty()
    }
}

bool AppHandler::is_application_dirty() const {
    return _is_dirty;
}

void AppHandler::prompt_save_changes(std::function<void()> on_save, std::function<void()> on_discard, std::function<void()> on_cancel) {
    if (!main_window) {
        on_cancel();
        return;
    }

    wxMessageDialog dlg(main_window, "¿Desea guardar los cambios realizados?", "Guardar Cambios", wxYES_NO | wxCANCEL | wxICON_QUESTION);
    int result = dlg.ShowModal();

    if (result == wxID_YES) on_save();
    else if (result == wxID_NO) on_discard();
    else on_cancel();
}