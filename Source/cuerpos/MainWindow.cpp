/**
 * Archivo: MainWindow.cpp
 * Descripción: Implementación completa de la ventana principal de ReinventProse 3.0.
 */

#include "../encabezados/MainWindow.h"
#include "../encabezados/AppHandler.h"
#include "../encabezados/BookDetailsView.h"
#include "../encabezados/ChapterContentView.h"
#include "../encabezados/AbstractIdeaView.h"
#include "../encabezados/ConcreteIdeaView.h"
#include "../encabezados/LibraryView.h"
#include "../encabezados/ChapterListView.h"
#include "../encabezados/NewBookDialog.h"
#include "../encabezados/Exporter.h"
#include "../encabezados/CustomAboutDialog.h"
#include "../encabezados/Util.h"

// include
#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/stdpaths.h>
#include <fstream>
#include <sstream>
#include <wx/mstream.h>

// Resources
#include <IconData.h>
#include "NewBookData.h"
#include "SaveData.h"
#include "EditData.h"
#include "LibraryData.h"
#include "UndoData.h"
#include "RedoData.h"

 // ============================================================================
 // TABLA DE EVENTOS ESTÁTICA
 // ============================================================================
wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
EVT_CLOSE(MainWindow::on_close)
EVT_MENU(wxID_NEW, MainWindow::on_menu_new_book)
EVT_MENU(MainWindow::ID_TOOL_EDIT_BOOK, MainWindow::on_edit_book_tool_click)
EVT_MENU(wxID_SAVE, MainWindow::on_save_current_book_tool_click)
EVT_MENU(MainWindow::ID_TOOL_BACK_TO_LIBRARY, MainWindow::on_back_to_library_tool_click)
EVT_MENU(wxID_UNDO, MainWindow::on_undo_tool_click)
EVT_MENU(wxID_REDO, MainWindow::on_redo_tool_click)
EVT_MENU(wxID_EXIT, MainWindow::on_menu_exit)
EVT_MENU(wxID_ABOUT, MainWindow::on_menu_about)
EVT_MENU(MainWindow::ID_EXPORT_TXT, MainWindow::on_export_txt)
EVT_MENU(MainWindow::ID_EXPORT_DOCX, MainWindow::on_export_docx)
EVT_MENU(MainWindow::ID_EXPORT_PDF, MainWindow::on_export_pdf)
wxEND_EVENT_TABLE()

// ============================================================================
// CONSTRUCTOR Y DESTRUCTOR
// ============================================================================
MainWindow::MainWindow(wxWindow* parent, const wxString& title, AppHandler* handler)
    : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxSize(1366, 768)),
    m_app_name(title), m_app_handler(handler), m_current_app_state(-1),
    m_edit_notebook(nullptr), m_current_book_id(std::nullopt), m_current_chapter_id(std::nullopt)
{
    m_base_panel = new wxPanel(this);
    m_aui_manager.SetManagedWindow(m_base_panel);

    _set_application_icon();
    _create_status_bar();
    _create_views();
    _create_menu_bar();
    _create_toolbar();

    _load_state();

    wxCommandEvent dummy;
    on_show_library_as_center(dummy, true);
}

MainWindow::~MainWindow() {
    m_aui_manager.UnInit();
}

// ============================================================================
// INICIALIZACIÓN DE COMPONENTES
// ============================================================================

wxString MainWindow::_get_resource_path(const wxString& file_name) {
    auto path = Util::GetAssetPath(file_name.ToStdString());
    if (path) {
        return wxString::FromUTF8(path.value());
    }
    return wxEmptyString;
}

void MainWindow::_set_application_icon() {
    // 1. Creamos el flujo de lectura apuntando al array que vive en IconData.h
    wxMemoryInputStream iconStream(app_icon_data, app_icon_data_size);

    // 2. Cargamos la imagen especificando que el contenido es tipo ICO
    wxImage iconImage;
    if (iconImage.LoadFile(iconStream, wxBITMAP_TYPE_ICO)) {
        // 3. Convertimos a icono y lo pegamos a la ventana
        wxIcon windowIcon;
        windowIcon.CopyFromBitmap(wxBitmap(iconImage));
        this->SetIcon(windowIcon);
    }
    // Si falla, no hace nada (queda el genérico), pero no rompe por rutas inexistentes
}

wxBitmap MainWindow::_load_tool_icon(const wxString& icon_name, const wxSize& icon_size) {
    return Util::LoadIconBitmap(icon_name, icon_size);
}

void MainWindow::_create_status_bar() {
    CreateStatusBar(2);
    SetStatusText("Listo", 0);
    SetStatusText("Libro ID: N/A", 1);
}

void MainWindow::_create_views() {
    // Biblioteca
    m_library_view = new LibraryView(m_base_panel, m_app_handler);
    m_library_view->set_on_book_card_selected_callback([this](int id) { on_library_book_selected(id); });

    // Detalles del libro
    m_book_details_view = new BookDetailsView(m_base_panel, m_app_handler);

    // Lista de Capítulos
    m_chapter_list_view = new ChapterListView(m_base_panel, m_app_handler);
    m_chapter_list_view->set_on_chapter_selected_callback([this](std::optional<int> id) { on_main_window_chapter_selected(id); });

    m_aui_manager.AddPane(m_library_view, wxAuiPaneInfo().Name("library").Caption("Biblioteca").CenterPane().Hide());
    m_aui_manager.AddPane(m_book_details_view, wxAuiPaneInfo().Name("details").Caption("Detalles del Libro").CenterPane().Hide());
    m_aui_manager.AddPane(m_chapter_list_view, wxAuiPaneInfo().Name("chapters").Caption("Capítulos").Left().BestSize(250, -1).Hide());

    // SOLUCIÓN AL BUG: Cargar libros explícitamente al iniciar la app
    m_library_view->load_books();
}

void MainWindow::_ensure_edit_notebook() {
    if (!m_edit_notebook) {
        m_edit_notebook = new wxAuiNotebook(m_base_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS);

        m_chapter_content_view = new ChapterContentView(m_edit_notebook, m_app_handler);
        m_abstract_idea_view = new AbstractIdeaView(m_edit_notebook, m_app_handler);
        m_concrete_idea_view = new ConcreteIdeaView(m_edit_notebook, m_app_handler);

        m_edit_notebook->AddPage(m_chapter_content_view, "Escritura");
        m_edit_notebook->AddPage(m_abstract_idea_view, "Idea Abstracta");
        m_edit_notebook->AddPage(m_concrete_idea_view, "Ideas Concretas");

        m_aui_manager.AddPane(m_edit_notebook, wxAuiPaneInfo().Name("editor").Caption("Editor de Capítulo").CenterPane().Hide());
    }
}

void MainWindow::_create_menu_bar() {
    wxMenuBar* mb = new wxMenuBar();

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, "Nuevo Libro\tCtrl+N");
    fileMenu->Append(ID_TOOL_EDIT_BOOK, "Editar Libro Seleccionado\tCtrl+E");
    fileMenu->Append(wxID_SAVE, "Guardar Cambios\tCtrl+S");
    fileMenu->AppendSeparator();

    wxMenu* exportMenu = new wxMenu();
    exportMenu->Append(ID_EXPORT_TXT, "Como TXT...");
    exportMenu->Append(ID_EXPORT_DOCX, "Como DOCX...");
    exportMenu->Append(ID_EXPORT_PDF, "Como PDF...");
    fileMenu->AppendSubMenu(exportMenu, "Exportar");

    fileMenu->AppendSeparator();
    fileMenu->Append(ID_TOOL_BACK_TO_LIBRARY, "Mostrar Biblioteca\tCtrl+L");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "Salir\tCtrl+Q");
    mb->Append(fileMenu, "&Archivo");

    wxMenu* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, "Deshacer\tCtrl+Z");
    editMenu->Append(wxID_REDO, "Rehacer\tCtrl+Y");
    mb->Append(editMenu, "&Editar");

    wxMenu* viewMenu = new wxMenu();
    m_menu_view_library = viewMenu->AppendCheckItem(ID_VIEW_LIBRARY, "Panel Biblioteca");
    m_menu_view_chapters = viewMenu->AppendCheckItem(ID_VIEW_CHAPTERS, "Panel Capítulos");
    m_menu_view_details = viewMenu->AppendCheckItem(ID_VIEW_DETAILS, "Panel Detalles");
    m_menu_view_editor = viewMenu->AppendCheckItem(ID_VIEW_EDITOR, "Panel Editor");
    mb->Append(viewMenu, "&Ver");

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "Acerca de...");
    mb->Append(helpMenu, "Ayuda");

    SetMenuBar(mb);

    Bind(wxEVT_MENU, [this](wxCommandEvent&) { _on_toggle_pane("library", m_menu_view_library); }, ID_VIEW_LIBRARY);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) { _on_toggle_pane("chapters", m_menu_view_chapters); }, ID_VIEW_CHAPTERS);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) { _on_toggle_pane("details", m_menu_view_details); }, ID_VIEW_DETAILS);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) { _on_toggle_pane("editor", m_menu_view_editor); }, ID_VIEW_EDITOR);
}

void MainWindow::_create_toolbar() {
    wxToolBar* tb = this->CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT | wxTB_FLAT);
    tb->SetToolBitmapSize(wxSize(24, 24));
    _update_toolbar_state(STATE_LIBRARY);
}

// ============================================================================
// LÓGICA DE INTERFAZ Y ESTADOS
// ============================================================================

wxBitmap MainWindow::_get_res_bmp(const unsigned char* data, unsigned int size) {
    wxMemoryInputStream stream(data, size);
    wxImage img;
    if (img.LoadFile(stream, wxBITMAP_TYPE_ANY)) {
        // Rescatamos la imagen al tamaño estándar de la toolbar (24x24)
        return wxBitmap(img.Rescale(24, 24, wxIMAGE_QUALITY_HIGH));
    }
    return wxNullBitmap;
}

void MainWindow::_update_toolbar_state(int state) {
    wxToolBar* tb = GetToolBar();
    if (!tb) return;

    tb->ClearTools();
    m_current_app_state = state;

    // --- ESTADO: BIBLIOTECA ---
    if (state == STATE_LIBRARY) {
        tb->AddTool(wxID_NEW, "Nuevo", _get_res_bmp(new_book_data, new_book_data_size), "Crear nuevo libro");
    }

    // --- ESTADO: DETALLES DEL LIBRO ---
    else if (state == STATE_DETAILS) {
        tb->AddTool(ID_TOOL_BACK_TO_LIBRARY, "Biblioteca", _get_res_bmp(library_data, library_data_size), "Volver a la biblioteca");
        tb->AddTool(ID_TOOL_EDIT_BOOK, "Editar", _get_res_bmp(edit_data, edit_data_size), "Editar capítulos");
        tb->AddTool(wxID_SAVE, "Guardar", _get_res_bmp(save_data, save_data_size), "Guardar cambios");
    }

    // --- ESTADO: EDICIÓN DE CAPÍTULO ---
    else if (state == STATE_EDIT) {
        tb->AddTool(ID_TOOL_BACK_TO_LIBRARY, "Biblioteca", _get_res_bmp(library_data, library_data_size), "Volver a la biblioteca");
        tb->AddTool(wxID_SAVE, "Guardar", _get_res_bmp(save_data, save_data_size), "Guardar cambios");
        tb->AddSeparator();
        tb->AddTool(wxID_UNDO, "Deshacer", _get_res_bmp(undo_data, undo_data_size), "Deshacer (Ctrl+Z)");
        tb->AddTool(wxID_REDO, "Rehacer", _get_res_bmp(redo_data, redo_data_size), "Rehacer (Ctrl+Y)");
    }

    tb->Realize(); // Refresca la toolbar visualmente
}

void MainWindow::set_dirty_status_in_title(bool is_dirty) {
    wxString final_title = m_app_name;

    if (m_current_book_id.has_value() && (m_current_app_state == STATE_DETAILS || m_current_app_state == STATE_EDIT)) {
        auto book_details = m_app_handler->get_book_details(*m_current_book_id);
        if (book_details && book_details->count("title")) {
            final_title += " - " + wxString::FromUTF8(std::get<std::string>(book_details->at("title")));
        }
    }

    if (is_dirty) final_title += "*";
    this->SetTitle(final_title);
}

void MainWindow::_update_library_view_layout(bool is_sidebar) {
    if (m_library_view) m_library_view->set_layout_mode(is_sidebar);
    _highlight_active_book_in_library(is_sidebar ? m_current_book_id : std::nullopt);
}

void MainWindow::_highlight_active_book_in_library(std::optional<int> active_id) {
    if (m_library_view) {
        for (auto card : m_library_view->get_book_card_panels()) {
            card->set_active_style(active_id.has_value() && card->get_book_id() == active_id.value());
        }
    }
}

void MainWindow::_update_notebook_pages_state(bool chapter_is_selected) {
    if (!m_edit_notebook) return;

    m_edit_notebook->Enable(chapter_is_selected);
    if (m_chapter_content_view) m_chapter_content_view->enable_view(chapter_is_selected);
    if (m_abstract_idea_view) m_abstract_idea_view->enable_view(chapter_is_selected);
    if (m_concrete_idea_view) m_concrete_idea_view->enable_view(chapter_is_selected);
}

void MainWindow::_load_chapter_data_into_edit_views(std::optional<int> chapter_id) {
    _ensure_edit_notebook();
    if (m_chapter_content_view) m_chapter_content_view->load_content(chapter_id);
    if (m_abstract_idea_view) m_abstract_idea_view->load_idea(chapter_id);
    if (m_concrete_idea_view) m_concrete_idea_view->load_ideas(chapter_id);
}

void MainWindow::_reevaluate_global_dirty_state() {
    bool is_dirty = false;
    if (m_current_app_state == STATE_DETAILS) {
        is_dirty = m_book_details_view && m_book_details_view->is_dirty();
    }
    else if (m_current_app_state == STATE_EDIT) {
        is_dirty = (m_chapter_content_view && m_chapter_content_view->is_dirty()) ||
            (m_abstract_idea_view && m_abstract_idea_view->is_dirty());
    }

    if (!is_dirty && m_app_handler->is_application_dirty()) is_dirty = true;

    m_app_handler->set_dirty(is_dirty);
    set_dirty_status_in_title(is_dirty);
}

void MainWindow::_clear_chapter_views_and_selection() {
    m_current_chapter_id = std::nullopt;
    if (m_chapter_list_view) m_chapter_list_view->load_chapters(std::nullopt);
    if (m_edit_notebook) {
        _load_chapter_data_into_edit_views(std::nullopt);
        _update_notebook_pages_state(false);
    }
}

bool MainWindow::_confirm_discard_changes() {
    if (!m_app_handler->is_application_dirty()) return true;

    wxString book_addon = "";
    if (m_current_book_id.has_value()) {
        auto book_data = m_app_handler->get_book_details(*m_current_book_id);
        if (book_data && book_data->count("title")) {
            book_addon = " en '" + wxString::FromUTF8(std::get<std::string>(book_data->at("title"))) + "'";
        }
    }

    wxString msg = "Hay cambios sin guardar" + book_addon + ". ¿Desea descartarlos y continuar?";
    wxMessageDialog dlg(this, msg, "Descartar Cambios", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);

    if (dlg.ShowModal() == wxID_YES) {
        if (m_current_app_state == STATE_DETAILS && m_book_details_view) {
            m_book_details_view->set_view_dirty(false);
        }
        else if (m_current_app_state == STATE_EDIT) {
            if (m_chapter_content_view) m_chapter_content_view->set_view_dirty(false);
            if (m_abstract_idea_view) m_abstract_idea_view->set_view_dirty(false);
        }
        m_app_handler->set_dirty(false);
        return true;
    }
    return false;
}

void MainWindow::_update_view_menu_items_state() {
    if (m_menu_view_library) m_menu_view_library->Check(m_aui_manager.GetPane("library").IsShown());
    if (m_menu_view_chapters) m_menu_view_chapters->Check(m_aui_manager.GetPane("chapters").IsShown());
    if (m_menu_view_details) m_menu_view_details->Check(m_aui_manager.GetPane("details").IsShown());
    if (m_menu_view_editor) m_menu_view_editor->Check(m_aui_manager.GetPane("editor").IsShown());
}

// ============================================================================
// MANEJADORES DE EVENTOS
// ============================================================================

void MainWindow::on_show_library_as_center(wxCommandEvent& event, bool force_clean) {
    if (!force_clean && m_app_handler->is_application_dirty() && !_confirm_discard_changes()) return;

    m_current_book_id = std::nullopt;
    m_current_app_state = STATE_LIBRARY;

    _clear_chapter_views_and_selection();
    if (m_book_details_view) m_book_details_view->load_book_details(std::nullopt);

    GetStatusBar()->SetStatusText("Biblioteca", 0);
    GetStatusBar()->SetStatusText("Libro ID: N/A", 1);

    m_aui_manager.GetPane("details").Hide();
    m_aui_manager.GetPane("chapters").Hide();
    if (m_edit_notebook) m_aui_manager.GetPane("editor").Hide();

    m_aui_manager.GetPane("library").Show().CenterPane();

    _update_library_view_layout(false);

    if (force_clean || m_app_handler->is_application_dirty()) {
        m_app_handler->set_dirty(false);
    }

    _update_toolbar_state(STATE_LIBRARY);
    set_dirty_status_in_title(m_app_handler->is_application_dirty());
    _update_view_menu_items_state();

    m_aui_manager.Update();
}

void MainWindow::on_library_book_selected(int selected_book_id) {
    if (selected_book_id == m_current_book_id && (m_current_app_state == STATE_DETAILS || m_current_app_state == STATE_EDIT)) {
        wxCommandEvent dummy;
        on_show_library_as_center(dummy);
        return;
    }

    if (m_app_handler->is_application_dirty()) {
        if (!_confirm_discard_changes()) {
            _highlight_active_book_in_library(m_current_book_id);
            return;
        }
    }

    if (m_current_app_state == STATE_EDIT) {
        _clear_chapter_views_and_selection();
        m_aui_manager.GetPane("editor").Hide();
        m_aui_manager.GetPane("chapters").Hide();
    }

    m_current_book_id = selected_book_id;
    m_current_app_state = STATE_DETAILS;

    auto book_details = m_app_handler->get_book_details(selected_book_id);
    wxString title = book_details && book_details->count("title") ? wxString::FromUTF8(std::get<std::string>(book_details->at("title"))) : wxString("Desconocido");

    GetStatusBar()->SetStatusText(wxString::Format("Libro: %s", title), 0);
    GetStatusBar()->SetStatusText(wxString::Format("Libro ID: %d", selected_book_id), 1);

    if (m_book_details_view) m_book_details_view->load_book_details(selected_book_id);

    m_aui_manager.GetPane("chapters").Hide();
    if (m_edit_notebook) m_aui_manager.GetPane("editor").Hide();

    m_aui_manager.GetPane("library").Left().Layer(0).Position(0).BestSize(250, -1).Show();
    m_aui_manager.GetPane("details").CentrePane().Show();

    _update_library_view_layout(true);
    _update_toolbar_state(STATE_DETAILS);

    m_app_handler->set_dirty(false);
    set_dirty_status_in_title(false);
    _update_view_menu_items_state();

    m_aui_manager.Update();
}

void MainWindow::on_edit_book_tool_click(wxCommandEvent& event) {
    if (!m_current_book_id.has_value()) {
        wxMessageBox("Por favor, seleccione un libro para editar sus capítulos.", "Aviso", wxOK | wxICON_INFORMATION, this);
        return;
    }

    if (m_current_app_state == STATE_DETAILS && m_book_details_view && m_book_details_view->is_dirty()) {
        int res = wxMessageBox("Hay cambios sin guardar en los detalles. ¿Desea guardarlos antes de editar capítulos?", "Guardar Detalles", wxYES_NO | wxCANCEL | wxICON_QUESTION);
        if (res == wxYES) {
            if (!m_book_details_view->save_changes()) {
                wxMessageBox("Error al guardar detalles. No se puede continuar.", "Error", wxOK | wxICON_ERROR);
                return;
            }
        }
        else if (res == wxCANCEL) {
            return;
        }
        else {
            m_book_details_view->set_view_dirty(false);
            _reevaluate_global_dirty_state();
        }
    }

    _ensure_edit_notebook();
    m_current_app_state = STATE_EDIT;

    m_aui_manager.GetPane("library").Hide();
    m_aui_manager.GetPane("details").Hide();

    m_aui_manager.GetPane("chapters").Left().Layer(0).Position(0).BestSize(300, -1).Show();
    m_aui_manager.GetPane("editor").CentrePane().Show();

    _update_notebook_pages_state(false);
    if (m_chapter_list_view) m_chapter_list_view->load_chapters(m_current_book_id);
    if (m_edit_notebook && m_edit_notebook->GetPageCount() > 0) m_edit_notebook->SetSelection(0);

    _update_toolbar_state(STATE_EDIT);
    set_dirty_status_in_title(m_app_handler->is_application_dirty());
    _update_view_menu_items_state();

    m_aui_manager.Update();
}

void MainWindow::on_main_window_chapter_selected(std::optional<int> chapter_id) {
    bool current_views_dirty = false;

    if (m_current_chapter_id.has_value() && chapter_id != m_current_chapter_id) {
        if (m_chapter_content_view && m_chapter_content_view->is_dirty()) current_views_dirty = true;
        if (!current_views_dirty && m_abstract_idea_view && m_abstract_idea_view->is_dirty()) current_views_dirty = true;
    }

    if (current_views_dirty) {
        int res = wxMessageBox("Hay cambios sin guardar en el capítulo actual. ¿Guardar antes de cambiar?", "Guardar Cambios", wxYES_NO | wxCANCEL | wxICON_QUESTION);
        if (res == wxYES) {
            bool save_ok = true;
            if (m_chapter_content_view && m_chapter_content_view->is_dirty() && !m_chapter_content_view->save_changes()) save_ok = false;
            if (m_abstract_idea_view && m_abstract_idea_view->is_dirty() && !m_abstract_idea_view->save_changes()) save_ok = false;

            if (!save_ok) {
                wxMessageBox("Error al guardar.", "Error", wxOK | wxICON_ERROR);
                if (m_chapter_list_view) m_chapter_list_view->select_chapter_by_id(m_current_chapter_id);
                return;
            }
        }
        else if (res == wxCANCEL) {
            if (m_chapter_list_view) m_chapter_list_view->select_chapter_by_id(m_current_chapter_id);
            return;
        }
        else {
            if (m_chapter_content_view) m_chapter_content_view->set_view_dirty(false);
            if (m_abstract_idea_view) m_abstract_idea_view->set_view_dirty(false);
        }
    }

    m_current_chapter_id = chapter_id;
    _load_chapter_data_into_edit_views(chapter_id);
    _update_notebook_pages_state(chapter_id.has_value());
    _reevaluate_global_dirty_state();
}

void MainWindow::on_save_current_book_tool_click(wxCommandEvent& event) {
    if (!m_current_book_id.has_value()) return;

    bool all_ok = true;

    if (m_current_app_state == STATE_DETAILS && m_book_details_view && m_book_details_view->is_dirty()) {
        if (!m_book_details_view->save_changes()) all_ok = false;
    }
    else if (m_current_app_state == STATE_EDIT && m_current_chapter_id.has_value()) {
        if (m_chapter_content_view && m_chapter_content_view->is_dirty()) {
            if (!m_chapter_content_view->save_changes()) all_ok = false;
        }
        if (m_abstract_idea_view && m_abstract_idea_view->is_dirty()) {
            if (!m_abstract_idea_view->save_changes()) all_ok = false;
        }
    }

    if (all_ok) {
        m_app_handler->set_dirty(false);
    }
    else {
        wxMessageBox("Algunos cambios no pudieron guardarse.", "Error", wxOK | wxICON_ERROR);
    }
    _reevaluate_global_dirty_state();
}

void MainWindow::on_menu_new_book(wxCommandEvent& event) {
    if (m_app_handler->is_application_dirty() && !_confirm_discard_changes()) return;

    NewBookDialog dlg(this, m_app_handler, wxString::Format("Nuevo Libro - %s", m_app_name));
    if (dlg.ShowModal() == wxID_OK) {
        auto data = dlg.get_book_data();
        auto nid = m_app_handler->create_new_book(data["title"], data["author"], data["synopsis"], "", "", "");
        if (nid.has_value()) {
            m_app_handler->set_dirty(false);
            set_dirty_status_in_title(false);
            if (m_library_view) m_library_view->load_books();
            on_library_book_selected(nid.value());
        }
    }
}

void MainWindow::on_export_txt(wxCommandEvent& event) {
    if (!m_current_book_id.has_value()) return;

    wxFileDialog saveDlg(this, "Exportar a TXT", "", "", "Archivos TXT (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        auto book = m_app_handler->get_book_details(*m_current_book_id);
        auto chapters = m_app_handler->get_chapters_by_book_id(*m_current_book_id);

        if (book.has_value()) {
            TxtExporter exp;
            if (exp.export_book(book.value(), chapters, saveDlg.GetPath().ToStdString())) {
                wxMessageBox("Libro exportado a TXT exitosamente.");
            }
            else {
                wxMessageBox("Fallo al exportar el libro.", "Error", wxOK | wxICON_ERROR);
            }
        }
    }
}

void MainWindow::on_export_docx(wxCommandEvent& event) {
    if (!m_current_book_id.has_value()) return;

    wxFileDialog saveDlg(this, "Exportar a DOCX", "", "", "Archivos DOCX (*.docx)|*.docx", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        auto book = m_app_handler->get_book_details(*m_current_book_id);
        auto chapters = m_app_handler->get_chapters_by_book_id(*m_current_book_id);

        if (book.has_value()) {
            DocxExporter exp;
            exp.export_book(book.value(), chapters, saveDlg.GetPath().ToStdString());
            wxMessageBox("Aviso: Exportación DOCX invocada (Generando aviso TXT por ahora).");
        }
    }
}

void MainWindow::on_export_pdf(wxCommandEvent& event) {
    if (!m_current_book_id.has_value()) return;

    wxFileDialog saveDlg(this, "Exportar a PDF", "", "", "Archivos PDF (*.pdf)|*.pdf", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        auto book = m_app_handler->get_book_details(*m_current_book_id);
        auto chapters = m_app_handler->get_chapters_by_book_id(*m_current_book_id);

        if (book.has_value()) {
            PdfExporter exp;
            exp.export_book(book.value(), chapters, saveDlg.GetPath().ToStdString());
            wxMessageBox("Aviso: Exportación PDF invocada (Generando aviso TXT por ahora).");
        }
    }
}

void MainWindow::on_menu_about(wxCommandEvent& event) {
    ReinventProseAboutInfo info;
    info.SetName(m_app_name.ToStdString());
    info.SetVersion("3.0.0");
    info.SetCopyright("(C) 2024 Mauricio Jose Tobares");
    info.SetDescription("Herramienta profesional para escritores.\nDesarrollada en C++ puro para un máximo rendimiento.");
    info.SetLicense("MIT License\nReinventProse es software libre.");

    wxString icon_path = _get_resource_path(APP_ICON);
    if (!icon_path.IsEmpty() && wxFileName::FileExists(icon_path)) {
        wxIcon icon(icon_path, wxBITMAP_TYPE_ICO);
        if (icon.IsOk()) info.SetIcon(icon);
    }

    ReinventProseAboutFrame* frame = new ReinventProseAboutFrame(this, info);
    frame->Show();
}

void MainWindow::_on_toggle_pane(const wxString& pane_name, wxMenuItem* item) {
    wxAuiPaneInfo& p = m_aui_manager.GetPane(pane_name);
    if (p.IsOk()) {
        p.Show(!p.IsShown());
        m_aui_manager.Update();
        if (item) item->Check(p.IsShown());
    }
}

void MainWindow::on_close(wxCloseEvent& event) {
    _save_state();
    if (m_app_handler->is_application_dirty() && !_confirm_discard_changes()) {
        event.Veto();
        return;
    }
    this->Destroy();
}

// Redirecciones
void MainWindow::on_menu_exit(wxCommandEvent& e) { this->Close(); }
void MainWindow::on_back_to_library_tool_click(wxCommandEvent& e) { on_show_library_as_center(e); }
void MainWindow::on_undo_tool_click(wxCommandEvent& e) { if (m_chapter_content_view) m_chapter_content_view->force_save_if_dirty(); }
void MainWindow::on_redo_tool_click(wxCommandEvent& e) {}

// ============================================================================
// PERSISTENCIA Y ARCHIVOS
// ============================================================================

wxString MainWindow::_get_config_path(const wxString& filename) {
    wxString config_dir = wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() + CONFIG_DIR;
    if (!wxFileName::DirExists(config_dir)) {
        wxFileName::Mkdir(config_dir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    return config_dir + wxFileName::GetPathSeparator() + filename;
}

void MainWindow::_save_state() {
    std::ofstream f(_get_config_path(PERSP_FILE).ToStdString());
    if (f.is_open()) {
        f << m_aui_manager.SavePerspective().ToStdString();
        f.close();
    }
}

void MainWindow::_load_state() {
    std::ifstream f(_get_config_path(PERSP_FILE).ToStdString());
    if (f.is_open()) {
        std::stringstream ss;
        ss << f.rdbuf();
        m_aui_manager.LoadPerspective(wxString::FromUTF8(ss.str().c_str()));
        f.close();
    }
    _update_view_menu_items_state();
}