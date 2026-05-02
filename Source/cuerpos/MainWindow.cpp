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
#include "../encabezados/WordStatsView.h"
#include "../encabezados/LibraryView.h"
#include "../encabezados/ChapterListView.h"
#include "../encabezados/NewBookDialog.h"
#include "../encabezados/Exporter.h"
#include "../encabezados/CustomAboutDialog.h"
#include "../encabezados/Util.h"

#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/stdpaths.h>
#include <fstream>
#include <sstream>
#include <wx/mstream.h>

// Resources
#include "IconData.h"
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
EVT_MENU(MainWindow::ID_MENU_CONFIG, MainWindow::on_menu_configuraciones)
EVT_MENU(MainWindow::ID_MENU_SKIN, MainWindow::on_menu_skin_editor)
EVT_MENU(MainWindow::ID_EXPORT_TXT, MainWindow::on_export_txt)
EVT_MENU(MainWindow::ID_EXPORT_DOCX, MainWindow::on_export_docx)
EVT_MENU(MainWindow::ID_EXPORT_PDF, MainWindow::on_export_pdf)

// Inteligencia de UI
EVT_UPDATE_UI(wxID_SAVE, MainWindow::on_update_ui_save_button)
EVT_UPDATE_UI(MainWindow::ID_TOOL_EDIT_BOOK, MainWindow::on_update_ui_needs_book)
EVT_UPDATE_UI(MainWindow::ID_EXPORT_TXT, MainWindow::on_update_ui_needs_book)
EVT_UPDATE_UI(MainWindow::ID_EXPORT_DOCX, MainWindow::on_update_ui_needs_book)
EVT_UPDATE_UI(MainWindow::ID_EXPORT_PDF, MainWindow::on_update_ui_needs_book)
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
    wxMemoryInputStream iconStream(app_icon_data, app_icon_data_size);
    wxImage iconImage;
    if (iconImage.LoadFile(iconStream, wxBITMAP_TYPE_ICO)) {
        wxIcon windowIcon;
        windowIcon.CopyFromBitmap(wxBitmap(iconImage));
        this->SetIcon(windowIcon);
    }
}

wxBitmap MainWindow::_load_tool_icon(const wxString& icon_name, const wxSize& icon_size) {
    return Util::LoadIconBitmap(icon_name, icon_size);
}

wxBitmap MainWindow::_get_res_bmp(const unsigned char* data, unsigned int size) {
    wxMemoryInputStream stream(data, size);
    wxImage img;
    if (img.LoadFile(stream, wxBITMAP_TYPE_ANY)) {
        return wxBitmap(img.Rescale(24, 24, wxIMAGE_QUALITY_HIGH));
    }
    return wxNullBitmap;
}

void MainWindow::_create_status_bar() {
    CreateStatusBar(2);
    SetStatusText("Listo", 0);
    SetStatusText("Libro ID: N/A", 1);
}

void MainWindow::_create_views() {
    m_library_view = new LibraryView(m_base_panel, m_app_handler);
    m_library_view->set_on_book_card_selected_callback([this](int id) { on_library_book_selected(id); });
    m_library_view->set_on_book_read_callback([this](int id) { on_library_book_read(id); });

    m_book_details_view = new BookDetailsView(m_base_panel, m_app_handler);

    m_chapter_list_view = new ChapterListView(m_base_panel, m_app_handler);
    m_chapter_list_view->set_on_chapter_selected_callback([this](std::optional<int> id) { on_main_window_chapter_selected(id); });

    m_aui_manager.AddPane(m_library_view, wxAuiPaneInfo().Name("library").Caption("Biblioteca").CenterPane().Hide());
    m_aui_manager.AddPane(m_book_details_view, wxAuiPaneInfo().Name("details").Caption("Detalles del Libro").CenterPane().Hide());
    m_aui_manager.AddPane(m_chapter_list_view, wxAuiPaneInfo().Name("chapters").Caption("Capítulos").Left().BestSize(250, -1).Hide());

    m_library_view->load_books();
}

void MainWindow::_ensure_edit_notebook() {
    if (!m_edit_notebook) {
        m_edit_notebook = new wxAuiNotebook(m_base_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS);

        m_chapter_content_view = new ChapterContentView(m_edit_notebook, m_app_handler);
        m_abstract_idea_view = new AbstractIdeaView(m_edit_notebook, m_app_handler);
        m_concrete_idea_view = new ConcreteIdeaView(m_edit_notebook, m_app_handler);
        m_word_stats_view = new WordStatsView(m_edit_notebook, m_app_handler);

        m_edit_notebook->AddPage(m_chapter_content_view, "Escritura");
        m_edit_notebook->AddPage(m_abstract_idea_view, "Idea Abstracta");
        m_edit_notebook->AddPage(m_concrete_idea_view, "Ideas Concretas");
        m_edit_notebook->AddPage(m_word_stats_view, "Estadísticas");

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
    editMenu->Append(ID_MENU_CONFIG, "Configuraciones...", "Fuentes, Idioma y preferencias");
    editMenu->Append(ID_MENU_SKIN, "Skin Editor (Beta)", "Personaliza el aspecto visual del programa");
    mb->Append(editMenu, "&Editar");

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "Acerca de...");
    mb->Append(helpMenu, "Ayuda");

    SetMenuBar(mb);
}

void MainWindow::_create_toolbar() {
    wxToolBar* tb = this->CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT | wxTB_FLAT);
    tb->SetToolBitmapSize(wxSize(24, 24));
    _update_toolbar_state(STATE_LIBRARY);
}

// ============================================================================
// LÓGICA DE INTERFAZ Y ESTADOS
// ============================================================================

void MainWindow::_update_toolbar_state(int state) {
    wxToolBar* tb = GetToolBar();
    if (!tb) return;

    tb->ClearTools();
    m_current_app_state = state;

    if (state == STATE_LIBRARY) {
        tb->AddTool(wxID_NEW, "Nuevo", _get_res_bmp(new_book_data, new_book_data_size), "Crear nuevo libro");
    }
    else if (state == STATE_DETAILS) {
        tb->AddTool(ID_TOOL_BACK_TO_LIBRARY, "Biblioteca", _get_res_bmp(library_data, library_data_size), "Volver a la biblioteca");
        tb->AddTool(ID_TOOL_EDIT_BOOK, "Editar", _get_res_bmp(edit_data, edit_data_size), "Editar capítulos");
        tb->AddTool(wxID_SAVE, "Guardar", _get_res_bmp(save_data, save_data_size), "Guardar cambios");
    }
    else if (state == STATE_EDIT) {
        tb->AddTool(ID_TOOL_BACK_TO_LIBRARY, "Biblioteca", _get_res_bmp(library_data, library_data_size), "Volver a la biblioteca");
        tb->AddTool(wxID_SAVE, "Guardar", _get_res_bmp(save_data, save_data_size), "Guardar cambios");
        tb->AddSeparator();
        tb->AddTool(wxID_UNDO, "Deshacer", _get_res_bmp(undo_data, undo_data_size), "Deshacer (Ctrl+Z)");
        tb->AddTool(wxID_REDO, "Rehacer", _get_res_bmp(redo_data, redo_data_size), "Rehacer (Ctrl+Y)");
    }

    tb->Realize();
}

void MainWindow::on_update_ui_save_button(wxUpdateUIEvent& event) {
    event.Enable(m_app_handler->is_application_dirty());
}

void MainWindow::on_update_ui_needs_book(wxUpdateUIEvent& event) {
    event.Enable(m_current_book_id.has_value());
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
    if (m_word_stats_view) m_word_stats_view->enable_view(chapter_is_selected);
}

void MainWindow::_load_chapter_data_into_edit_views(std::optional<int> chapter_id) {
    _ensure_edit_notebook();
    if (m_chapter_content_view) m_chapter_content_view->load_content(chapter_id);
    if (m_abstract_idea_view) m_abstract_idea_view->load_idea(chapter_id);
    if (m_concrete_idea_view) m_concrete_idea_view->load_ideas(chapter_id);
    if (m_word_stats_view) m_word_stats_view->load_stats(chapter_id);
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

    m_aui_manager.Update();
}

void MainWindow::on_library_book_read(int selected_book_id) {
    if (selected_book_id == m_current_book_id && m_current_app_state == STATE_EDIT) {
        return;
    }

    if (m_app_handler->is_application_dirty() && !_confirm_discard_changes()) {
        return;
    }

    m_current_book_id = selected_book_id;
    m_current_app_state = STATE_EDIT;

    auto book_details = m_app_handler->get_book_details(selected_book_id);
    wxString title = book_details && book_details->count("title") ? wxString::FromUTF8(std::get<std::string>(book_details->at("title"))) : wxString("Desconocido");

    GetStatusBar()->SetStatusText(wxString::Format("Libro: %s", title), 0);
    GetStatusBar()->SetStatusText(wxString::Format("Libro ID: %d", selected_book_id), 1);

    if (m_book_details_view) m_book_details_view->load_book_details(selected_book_id);

    _ensure_edit_notebook();

    m_aui_manager.GetPane("library").Hide();
    m_aui_manager.GetPane("details").Hide();

    m_aui_manager.GetPane("chapters").Left().Layer(0).Position(0).BestSize(300, -1).Show();
    m_aui_manager.GetPane("editor").CentrePane().Show();

    _update_notebook_pages_state(false);

    if (m_chapter_list_view) m_chapter_list_view->load_chapters(m_current_book_id);
    if (m_edit_notebook && m_edit_notebook->GetPageCount() > 0) m_edit_notebook->SetSelection(0);

    _update_toolbar_state(STATE_EDIT);

    m_app_handler->set_dirty(false);
    set_dirty_status_in_title(false);

    m_aui_manager.Update();
}

void MainWindow::on_edit_book_tool_click(wxCommandEvent& event) {
    if (!m_current_book_id.has_value()) {
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

    m_aui_manager.Update();
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

void MainWindow::on_menu_new_book(wxCommandEvent& event) {
    if (m_app_handler->is_application_dirty() && !_confirm_discard_changes()) return;

    NewBookDialog dlg(this, m_app_handler, wxString::Format("Nuevo Libro - %s", m_app_name));
    if (dlg.ShowModal() == wxID_OK) {
        auto data = dlg.get_book_data();

        std::vector<uint8_t> empty_cover;
        auto nid = m_app_handler->create_new_book(data["title"], data["author"], data["synopsis"], "", "", empty_cover);

        if (nid.has_value()) {
            m_app_handler->set_dirty(false);
            set_dirty_status_in_title(false);
            if (m_library_view) m_library_view->load_books();
            on_library_book_selected(nid.value());
        }
    }
}

void MainWindow::on_menu_configuraciones(wxCommandEvent& event) {
    wxMessageBox("El panel de Configuraciones estará disponible próximamente para gestionar fuentes, idiomas y el motor de base de datos.", "Configuración de Sistema");
}

void MainWindow::on_menu_skin_editor(wxCommandEvent& event) {
    wxMessageBox("Skin Editor: En futuras actualizaciones podrás personalizar los colores RGBa de las tarjetas y la tipografía de la interfaz.", "Personalización Visual");
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

    info.SetName(wxString::Format("ReinventProse 3.0").ToUTF8().data());
    info.SetVersion(wxString::Format("3.0.42 (Pure C++ Edition)").ToUTF8().data());
    info.SetCopyright(wxString::Format("(C) 2023-2024 Mauricio José Tobares. Todos los derechos reservados.").ToUTF8().data());

    wxString desc = wxString::Format(
        "Una aplicación de escritorio de alto rendimiento para la gestión integral "
        "y organización de proyectos de escritura creativa.\n\n"
        "ReinventProse permite al autor transitar desde la concepción de la idea "
        "abstracta hasta la estructuración técnica de capítulos y el tablero de ideas concretas.\n\n"
        "Esta versión %s ha sido reconstruida íntegramente en %s para ofrecer "
        "una experiencia de escritura con latencia cero.",
        "3.0", "C++20"
    );
    info.SetDescription(desc.ToUTF8().data());

    wxString lic_text = wxString::Format(
        "MIT License\n\n"
        "Copyright (c) 2023-2024 Mauricio José Tobares\n\n"
        "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
        "of this software and associated documentation files (the \"Software\"), to deal\n"
        "in the Software without restriction, including without limitation the rights\n"
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
        "copies of the Software, and to permit persons to whom the Software is\n"
        "furnished to do so, subject to the following conditions:\n\n"
        "The above copyright notice and this permission notice shall be included in all\n"
        "copies or substantial portions of the Software.\n\n"
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
        "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
        "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
        "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
        "SOFTWARE."
    );
    info.SetLicense(lic_text.ToUTF8().data());

    auto set_safe = [](const char* texto) -> std::string {
        return std::string(wxString::Format("%s", texto).ToUTF8().data());
        };

    info.AddDeveloper(set_safe("Mauricio José Tobares (El Jefe) - Ideólogo y Director del Proyecto"));
    info.AddDeveloper(set_safe("PJ (Programador Jefe Asistente IA) - Desarrollo Principal y Arquitectura"));
    info.AddDeveloper(set_safe("IP (Ingeniero de Pruebas IA) - Aseguramiento de Calidad"));

    info.AddDocWriter(set_safe("GP (Planificador de Proyectos IA) - Documentación Técnica y Planificación"));

    info.AddArtist(set_safe("DUXUI (Diseñador UX/UI IA) - Diseño de Interfaz y Experiencia de Usuario"));

    info.AddTranslator(set_safe("Mauricio José Tobares - Único responsable (aunque todavía no traduje nada XD)"));

    info.AddCollaborator(set_safe("Amigo Tester #1"), set_safe("Valiosas pruebas y sugerencias de usabilidad."));
    info.AddCollaborator(set_safe("Comunidad de Betas Anónimos"), set_safe("Por el feedback constructivo."));

    info.SetWebSite(
        wxString::Format("https://github.com/TobaTobinTobinsky/ReinventProse_V3").ToUTF8().data(),
        wxString::Format("Repositorio Oficial GitHub").ToUTF8().data()
    );

    ReinventProseAboutFrame* aboutFrame = new ReinventProseAboutFrame(this, info);
    aboutFrame->Show();
}

void MainWindow::on_menu_exit(wxCommandEvent& e) { this->Close(); }
void MainWindow::on_back_to_library_tool_click(wxCommandEvent& e) { on_show_library_as_center(e); }
void MainWindow::on_undo_tool_click(wxCommandEvent& e) { if (m_chapter_content_view) m_chapter_content_view->force_save_if_dirty(); }
void MainWindow::on_redo_tool_click(wxCommandEvent& e) {}

void MainWindow::on_close(wxCloseEvent& event) {
    _save_state();
    if (m_app_handler->is_application_dirty() && !_confirm_discard_changes()) {
        event.Veto();
        return;
    }
    this->Destroy();
}

// ============================================================================
// PERSISTENCIA Y CONFIGURACIÓN
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
}