/**
* Archivo: MainWindow.cpp
* Descripción: Implementación integral de la ventana principal de ReinventProse 3.0.
* Estándar: Blindaje universal con wxString::Format y prohibición de código comprimido.
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

// Recursos de imagen embebidos
#include "IconData.h"
#include "NewBookData.h"
#include "SaveData.h"
#include "EditData.h"
#include "LibraryData.h"
#include "UndoData.h"
#include "RedoData.h"

// ============================================================================
// TABLA DE EVENTOS
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
    m_app_name(title),
    m_app_handler(handler),
    m_current_app_state(-1),
    m_edit_notebook(nullptr),
    m_current_book_id(std::nullopt),
    m_current_chapter_id(std::nullopt)
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

MainWindow::~MainWindow()
{
    m_aui_manager.UnInit();
}

// ============================================================================
// INICIALIZACIÓN TÉCNICA
// ============================================================================

wxString MainWindow::_get_resource_path(const wxString& file_name)
{
    std::string file_name_std = file_name.ToStdString();
    std::optional<std::string> path_opt = Util::GetAssetPath(file_name_std);

    if (path_opt.has_value())
    {
        return wxString::FromUTF8(path_opt.value());
    }

    return wxEmptyString;
}

void MainWindow::_set_application_icon()
{
    wxMemoryInputStream iconStream(app_icon_data, app_icon_data_size);
    wxImage iconImage;

    bool loaded = iconImage.LoadFile(iconStream, wxBITMAP_TYPE_ICO);
    if (loaded)
    {
        wxIcon windowIcon;
        windowIcon.CopyFromBitmap(wxBitmap(iconImage));
        this->SetIcon(windowIcon);
    }
}

wxBitmap MainWindow::_load_tool_icon(const wxString& icon_name, const wxSize& icon_size)
{
    return Util::LoadIconBitmap(icon_name, icon_size);
}

wxBitmap MainWindow::_get_res_bmp(const unsigned char* data, unsigned int size)
{
    wxMemoryInputStream stream(data, size);
    wxImage img;

    bool success = img.LoadFile(stream, wxBITMAP_TYPE_ANY);
    if (success)
    {
        wxImage rescaled = img.Rescale(24, 24, wxIMAGE_QUALITY_HIGH);
        return wxBitmap(rescaled);
    }

    return wxNullBitmap;
}

void MainWindow::_create_status_bar()
{
    CreateStatusBar(2);

    wxString text_ready = wxString::Format("Listo");
    SetStatusText(text_ready, 0);

    wxString text_id = wxString::Format("Libro ID: N/A");
    SetStatusText(text_id, 1);
}

void MainWindow::_create_views()
{
    // 1. Biblioteca
    m_library_view = new LibraryView(m_base_panel, m_app_handler);

    m_library_view->set_on_book_card_selected_callback([this](int id) {
        this->on_library_book_selected(id);
        });

    m_library_view->set_on_book_read_callback([this](int id) {
        this->on_library_book_read(id);
        });

    // 2. Detalles del Libro
    m_book_details_view = new BookDetailsView(m_base_panel, m_app_handler);

    // 3. Lista de Capítulos (Lateral)
    m_chapter_list_view = new ChapterListView(m_base_panel, m_app_handler);

    m_chapter_list_view->set_on_chapter_selected_callback([this](std::optional<int> id) {
        this->on_main_window_chapter_selected(id);
        });

    // Registro en AUI
    wxString name_lib = wxString::Format("library");
    wxString cap_lib = wxString::Format("Biblioteca");
    m_aui_manager.AddPane(m_library_view, wxAuiPaneInfo().Name(name_lib).Caption(cap_lib).CenterPane().Hide());

    wxString name_det = wxString::Format("details");
    wxString cap_det = wxString::Format("Detalles del Libro");
    m_aui_manager.AddPane(m_book_details_view, wxAuiPaneInfo().Name(name_det).Caption(cap_det).CenterPane().Hide());

    wxString name_chap = wxString::Format("chapters");
    wxString cap_chap = wxString::Format("Capítulos");
    m_aui_manager.AddPane(m_chapter_list_view, wxAuiPaneInfo().Name(name_chap).Caption(cap_chap).Left().BestSize(250, -1).Hide());

    m_library_view->load_books();
}

void MainWindow::_ensure_edit_notebook()
{
    if (m_edit_notebook != nullptr)
    {
        return;
    }

    long style = wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS;
    m_edit_notebook = new wxAuiNotebook(m_base_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

    m_chapter_content_view = new ChapterContentView(m_edit_notebook, m_app_handler);
    m_abstract_idea_view = new AbstractIdeaView(m_edit_notebook, m_app_handler);
    m_concrete_idea_view = new ConcreteIdeaView(m_edit_notebook, m_app_handler);
    m_word_stats_view = new WordStatsView(m_edit_notebook, m_app_handler);

    wxString tab1 = wxString::Format("Escritura");
    m_edit_notebook->AddPage(m_chapter_content_view, tab1);

    wxString tab2 = wxString::Format("Idea Abstracta");
    m_edit_notebook->AddPage(m_abstract_idea_view, tab2);

    wxString tab3 = wxString::Format("Ideas Concretas");
    m_edit_notebook->AddPage(m_concrete_idea_view, tab3);

    wxString tab4 = wxString::Format("Estadísticas");
    m_edit_notebook->AddPage(m_word_stats_view, tab4);

    wxString name_edit = wxString::Format("editor");
    wxString cap_edit = wxString::Format("Editor de Capítulo");
    m_aui_manager.AddPane(m_edit_notebook, wxAuiPaneInfo().Name(name_edit).Caption(cap_edit).CenterPane().Hide());
}

void MainWindow::_create_menu_bar()
{
    wxMenuBar* mb = new wxMenuBar();

    // MENÚ ARCHIVO
    wxMenu* fileMenu = new wxMenu();

    wxString m_new = wxString::Format("Nuevo Libro\tCtrl+N");
    fileMenu->Append(wxID_NEW, m_new);

    wxString m_edit_lib = wxString::Format("Editar Libro Seleccionado\tCtrl+E");
    fileMenu->Append(ID_TOOL_EDIT_BOOK, m_edit_lib);

    wxString m_save = wxString::Format("Guardar Cambios\tCtrl+S");
    fileMenu->Append(wxID_SAVE, m_save);

    fileMenu->AppendSeparator();

    // SUBMENÚ EXPORTAR
    wxMenu* exportMenu = new wxMenu();

    wxString e_txt = wxString::Format("Como TXT...");
    exportMenu->Append(ID_EXPORT_TXT, e_txt);

    wxString e_docx = wxString::Format("Como DOCX...");
    exportMenu->Append(ID_EXPORT_DOCX, e_docx);

    wxString e_pdf = wxString::Format("Como PDF...");
    exportMenu->Append(ID_EXPORT_PDF, e_pdf);

    wxString m_export = wxString::Format("Exportar");
    fileMenu->AppendSubMenu(exportMenu, m_export);

    fileMenu->AppendSeparator();

    wxString m_lib = wxString::Format("Mostrar Biblioteca\tCtrl+L");
    fileMenu->Append(ID_TOOL_BACK_TO_LIBRARY, m_lib);

    fileMenu->AppendSeparator();

    wxString m_exit = wxString::Format("Salir\tCtrl+Q");
    fileMenu->Append(wxID_EXIT, m_exit);

    // MENÚ EDITAR
    wxMenu* editMenu = new wxMenu();

    wxString m_conf = wxString::Format("Configuraciones...");
    wxString m_conf_h = wxString::Format("Fuentes, Idioma y preferencias");
    editMenu->Append(ID_MENU_CONFIG, m_conf, m_conf_h);

    wxString m_skin = wxString::Format("Skin Editor (Beta)");
    wxString m_skin_h = wxString::Format("Personaliza el aspecto visual del programa");
    editMenu->Append(ID_MENU_SKIN, m_skin, m_skin_h);

    // MENÚ AYUDA
    wxMenu* helpMenu = new wxMenu();

    wxString m_about = wxString::Format("Acerca de...");
    helpMenu->Append(wxID_ABOUT, m_about);

    // Ensamblaje
    wxString label_file = wxString::Format("&Archivo");
    mb->Append(fileMenu, label_file);

    wxString label_edit = wxString::Format("&Editar");
    mb->Append(editMenu, label_edit);

    wxString label_help = wxString::Format("Ayuda");
    mb->Append(helpMenu, label_help);

    SetMenuBar(mb);
}

void MainWindow::_create_toolbar()
{
    wxToolBar* tb = this->CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT | wxTB_FLAT);
    tb->SetToolBitmapSize(wxSize(24, 24));

    _update_toolbar_state(STATE_LIBRARY);
}

// ============================================================================
// LÓGICA DE INTERFAZ Y ESTADOS
// ============================================================================

void MainWindow::_update_toolbar_state(int state)
{
    wxToolBar* tb = GetToolBar();
    if (tb == nullptr)
    {
        return;
    }

    tb->ClearTools();
    m_current_app_state = state;

    if (state == STATE_LIBRARY)
    {
        wxString tool_new = wxString::Format("Nuevo");
        wxString tool_new_h = wxString::Format("Crear nuevo libro");
        tb->AddTool(wxID_NEW, tool_new, _get_res_bmp(new_book_data, new_book_data_size), tool_new_h);
    }
    else if (state == STATE_DETAILS)
    {
        wxString tool_lib = wxString::Format("Biblioteca");
        wxString tool_lib_h = wxString::Format("Volver a la biblioteca");
        tb->AddTool(ID_TOOL_BACK_TO_LIBRARY, tool_lib, _get_res_bmp(library_data, library_data_size), tool_lib_h);

        wxString tool_edit = wxString::Format("Editar");
        wxString tool_edit_h = wxString::Format("Editar capítulos");
        tb->AddTool(ID_TOOL_EDIT_BOOK, tool_edit, _get_res_bmp(edit_data, edit_data_size), tool_edit_h);

        wxString tool_save = wxString::Format("Guardar");
        wxString tool_save_h = wxString::Format("Guardar cambios");
        tb->AddTool(wxID_SAVE, tool_save, _get_res_bmp(save_data, save_data_size), tool_save_h);
    }
    else if (state == STATE_EDIT)
    {
        wxString tool_lib = wxString::Format("Biblioteca");
        wxString tool_lib_h = wxString::Format("Volver a la biblioteca");
        tb->AddTool(ID_TOOL_BACK_TO_LIBRARY, tool_lib, _get_res_bmp(library_data, library_data_size), tool_lib_h);

        wxString tool_save = wxString::Format("Guardar");
        wxString tool_save_h = wxString::Format("Guardar cambios");
        tb->AddTool(wxID_SAVE, tool_save, _get_res_bmp(save_data, save_data_size), tool_save_h);

        tb->AddSeparator();

        wxString tool_undo = wxString::Format("Deshacer");
        wxString tool_undo_h = wxString::Format("Deshacer (Ctrl+Z)");
        tb->AddTool(wxID_UNDO, tool_undo, _get_res_bmp(undo_data, undo_data_size), tool_undo_h);

        wxString tool_redo = wxString::Format("Rehacer");
        wxString tool_redo_h = wxString::Format("Rehacer (Ctrl+Y)");
        tb->AddTool(wxID_REDO, tool_redo, _get_res_bmp(redo_data, redo_data_size), tool_redo_h);
    }

    tb->Realize();
}

void MainWindow::on_update_ui_save_button(wxUpdateUIEvent& event)
{
    bool is_dirty = m_app_handler->is_application_dirty();
    event.Enable(is_dirty);
}

void MainWindow::on_update_ui_needs_book(wxUpdateUIEvent& event)
{
    bool has_book = m_current_book_id.has_value();
    event.Enable(has_book);
}

void MainWindow::set_dirty_status_in_title(bool is_dirty)
{
    wxString final_title = m_app_name;

    bool needs_book_name = (m_current_app_state == STATE_DETAILS || m_current_app_state == STATE_EDIT);

    if (m_current_book_id.has_value() && needs_book_name)
    {
        auto book_details = m_app_handler->get_book_details(*m_current_book_id);
        if (book_details && book_details->count("title"))
        {
            wxString book_name = wxString::FromUTF8(std::get<std::string>(book_details->at("title")));
            final_title = wxString::Format("%s - %s", m_app_name, book_name);
        }
    }

    if (is_dirty)
    {
        final_title = wxString::Format("%s*", final_title);
    }

    this->SetTitle(final_title);
}

void MainWindow::_update_library_view_layout(bool is_sidebar)
{
    if (m_library_view != nullptr)
    {
        m_library_view->set_layout_mode(is_sidebar);
    }

    if (is_sidebar)
    {
        _highlight_active_book_in_library(m_current_book_id);
    }
    else
    {
        _highlight_active_book_in_library(std::nullopt);
    }
}

void MainWindow::_highlight_active_book_in_library(std::optional<int> active_id)
{
    if (m_library_view == nullptr)
    {
        return;
    }

    const std::vector<BookCardPanel*>& cards = m_library_view->get_book_card_panels();

    for (auto card : cards)
    {
        bool is_active = false;

        if (active_id.has_value())
        {
            if (card->get_book_id() == active_id.value())
            {
                is_active = true;
            }
        }

        card->set_active_style(is_active);
    }
}

void MainWindow::_update_notebook_pages_state(bool chapter_is_selected)
{
    if (m_edit_notebook == nullptr)
    {
        return;
    }

    m_edit_notebook->Enable(chapter_is_selected);

    if (m_chapter_content_view != nullptr)
    {
        m_chapter_content_view->enable_view(chapter_is_selected);
    }

    if (m_abstract_idea_view != nullptr)
    {
        m_abstract_idea_view->enable_view(chapter_is_selected);
    }

    if (m_concrete_idea_view != nullptr)
    {
        m_concrete_idea_view->enable_view(chapter_is_selected);
    }

    if (m_word_stats_view != nullptr)
    {
        m_word_stats_view->enable_view(chapter_is_selected);
    }
}

void MainWindow::_load_chapter_data_into_edit_views(std::optional<int> chapter_id)
{
    _ensure_edit_notebook();

    if (m_chapter_content_view != nullptr)
    {
        m_chapter_content_view->load_content(chapter_id);
    }

    if (m_abstract_idea_view != nullptr)
    {
        m_abstract_idea_view->load_idea(chapter_id);
    }

    if (m_concrete_idea_view != nullptr)
    {
        m_concrete_idea_view->load_ideas(chapter_id);
    }

    if (m_word_stats_view != nullptr)
    {
        m_word_stats_view->load_stats(chapter_id);
    }
}

void MainWindow::_reevaluate_global_dirty_state()
{
    bool is_dirty = false;

    if (m_current_app_state == STATE_DETAILS)
    {
        if (m_book_details_view != nullptr)
        {
            if (m_book_details_view->is_dirty())
            {
                is_dirty = true;
            }
        }
    }
    else if (m_current_app_state == STATE_EDIT)
    {
        if (m_chapter_content_view != nullptr)
        {
            if (m_chapter_content_view->is_dirty())
            {
                is_dirty = true;
            }
        }

        if (!is_dirty && m_abstract_idea_view != nullptr)
        {
            if (m_abstract_idea_view->is_dirty())
            {
                is_dirty = true;
            }
        }
    }

    if (!is_dirty)
    {
        if (m_app_handler->is_application_dirty())
        {
            is_dirty = true;
        }
    }

    m_app_handler->set_dirty(is_dirty);
    set_dirty_status_in_title(is_dirty);
}

void MainWindow::_clear_chapter_views_and_selection()
{
    m_current_chapter_id = std::nullopt;

    if (m_chapter_list_view != nullptr)
    {
        m_chapter_list_view->load_chapters(std::nullopt);
    }

    if (m_edit_notebook != nullptr)
    {
        _load_chapter_data_into_edit_views(std::nullopt);
        _update_notebook_pages_state(false);
    }
}

bool MainWindow::_confirm_discard_changes()
{
    if (!m_app_handler->is_application_dirty())
    {
        return true;
    }

    wxString book_context = "";
    if (m_current_book_id.has_value())
    {
        auto book_data = m_app_handler->get_book_details(*m_current_book_id);
        if (book_data && book_data->count("title"))
        {
            wxString b_name = wxString::FromUTF8(std::get<std::string>(book_data->at("title")));
            book_context = wxString::Format(" en '%s'", b_name);
        }
    }

    wxString msg = wxString::Format("Hay cambios sin guardar%s. ¿Desea descartarlos?", book_context);
    wxString cap = wxString::Format("Descartar Cambios");

    wxMessageDialog dlg(this, msg, cap, wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);

    int result = dlg.ShowModal();
    if (result == wxID_YES)
    {
        if (m_current_app_state == STATE_DETAILS && m_book_details_view != nullptr)
        {
            m_book_details_view->set_view_dirty(false);
        }
        else if (m_current_app_state == STATE_EDIT)
        {
            if (m_chapter_content_view != nullptr)
            {
                m_chapter_content_view->set_view_dirty(false);
            }
            if (m_abstract_idea_view != nullptr)
            {
                m_abstract_idea_view->set_view_dirty(false);
            }
        }

        m_app_handler->set_dirty(false);
        return true;
    }

    return false;
}

// ============================================================================
// MANEJADORES DE EVENTOS
// ============================================================================

void MainWindow::on_show_library_as_center(wxCommandEvent& event, bool force_clean)
{
    if (!force_clean)
    {
        if (m_app_handler->is_application_dirty())
        {
            bool can_proceed = _confirm_discard_changes();
            if (!can_proceed)
            {
                return;
            }
        }
    }

    m_current_book_id = std::nullopt;
    m_current_app_state = STATE_LIBRARY;

    _clear_chapter_views_and_selection();

    if (m_book_details_view != nullptr)
    {
        m_book_details_view->load_book_details(std::nullopt);
    }

    wxString st_lib = wxString::Format("Biblioteca");
    GetStatusBar()->SetStatusText(st_lib, 0);

    wxString st_na = wxString::Format("Libro ID: N/A");
    GetStatusBar()->SetStatusText(st_na, 1);

    m_aui_manager.GetPane("details").Hide();
    m_aui_manager.GetPane("chapters").Hide();

    if (m_edit_notebook != nullptr)
    {
        m_aui_manager.GetPane("editor").Hide();
    }

    m_aui_manager.GetPane("library").Show().CenterPane();

    _update_library_view_layout(false);

    if (force_clean)
    {
        m_app_handler->set_dirty(false);
    }
    else if (m_app_handler->is_application_dirty())
    {
        m_app_handler->set_dirty(false);
    }

    _update_toolbar_state(STATE_LIBRARY);
    set_dirty_status_in_title(m_app_handler->is_application_dirty());

    m_aui_manager.Update();
}

void MainWindow::on_library_book_selected(int selected_book_id)
{
    if (selected_book_id == m_current_book_id)
    {
        if (m_current_app_state == STATE_DETAILS)
        {
            wxCommandEvent dummy;
            on_show_library_as_center(dummy);
            return;
        }
    }

    if (m_app_handler->is_application_dirty())
    {
        bool can_switch = _confirm_discard_changes();
        if (!can_switch)
        {
            _highlight_active_book_in_library(m_current_book_id);
            return;
        }
    }

    if (m_current_app_state == STATE_EDIT)
    {
        _clear_chapter_views_and_selection();
        m_aui_manager.GetPane("editor").Hide();
        m_aui_manager.GetPane("chapters").Hide();
    }

    m_current_book_id = selected_book_id;
    m_current_app_state = STATE_DETAILS;

    auto book_details = m_app_handler->get_book_details(selected_book_id);
    wxString b_title = wxString::Format("Desconocido");

    if (book_details && book_details->count("title"))
    {
        b_title = wxString::FromUTF8(std::get<std::string>(book_details->at("title")));
    }

    wxString st_info = wxString::Format("Libro: %s", b_title);
    GetStatusBar()->SetStatusText(st_info, 0);

    wxString st_id = wxString::Format("Libro ID: %d", selected_book_id);
    GetStatusBar()->SetStatusText(st_id, 1);

    if (m_book_details_view != nullptr)
    {
        m_book_details_view->load_book_details(selected_book_id);
    }

    m_aui_manager.GetPane("chapters").Hide();

    if (m_edit_notebook != nullptr)
    {
        m_aui_manager.GetPane("editor").Hide();
    }

    m_aui_manager.GetPane("library").Left().Layer(0).Position(0).BestSize(250, -1).Show();
    m_aui_manager.GetPane("details").CentrePane().Show();

    _update_library_view_layout(true);
    _update_toolbar_state(STATE_DETAILS);

    m_app_handler->set_dirty(false);
    set_dirty_status_in_title(false);

    m_aui_manager.Update();
}

void MainWindow::on_library_book_read(int selected_book_id)
{
    if (selected_book_id == m_current_book_id)
    {
        if (m_current_app_state == STATE_EDIT)
        {
            return;
        }
    }

    if (m_app_handler->is_application_dirty())
    {
        bool can_proceed = _confirm_discard_changes();
        if (!can_proceed)
        {
            return;
        }
    }

    m_current_book_id = selected_book_id;
    m_current_app_state = STATE_EDIT;

    auto book_details = m_app_handler->get_book_details(selected_book_id);
    wxString b_title = wxString::Format("Desconocido");

    if (book_details && book_details->count("title"))
    {
        b_title = wxString::FromUTF8(std::get<std::string>(book_details->at("title")));
    }

    wxString st_info = wxString::Format("Libro: %s", b_title);
    GetStatusBar()->SetStatusText(st_info, 0);

    wxString st_id = wxString::Format("Libro ID: %d", selected_book_id);
    GetStatusBar()->SetStatusText(st_id, 1);

    if (m_book_details_view != nullptr)
    {
        m_book_details_view->load_book_details(selected_book_id);
    }

    _ensure_edit_notebook();

    m_aui_manager.GetPane("library").Hide();
    m_aui_manager.GetPane("details").Hide();

    m_aui_manager.GetPane("chapters").Left().Layer(0).Position(0).BestSize(300, -1).Show();
    m_aui_manager.GetPane("editor").CentrePane().Show();

    _update_notebook_pages_state(false);

    if (m_chapter_list_view != nullptr)
    {
        m_chapter_list_view->load_chapters(m_current_book_id);
    }

    if (m_edit_notebook != nullptr)
    {
        if (m_edit_notebook->GetPageCount() > 0)
        {
            m_edit_notebook->SetSelection(0);
        }
    }

    _update_toolbar_state(STATE_EDIT);

    m_app_handler->set_dirty(false);
    set_dirty_status_in_title(false);

    m_aui_manager.Update();
}

void MainWindow::on_edit_book_tool_click(wxCommandEvent& event)
{
    if (!m_current_book_id.has_value())
    {
        return;
    }

    if (m_current_app_state == STATE_DETAILS)
    {
        if (m_book_details_view != nullptr)
        {
            if (m_book_details_view->is_dirty())
            {
                wxString msg = wxString::Format("Hay cambios sin guardar en los detalles. ¿Desea guardarlos antes de editar capítulos?");
                wxString cap = wxString::Format("Guardar Detalles");

                int res = wxMessageBox(msg, cap, wxYES_NO | wxCANCEL | wxICON_QUESTION);

                if (res == wxYES)
                {
                    bool save_ok = m_book_details_view->save_changes();
                    if (!save_ok)
                    {
                        wxString err = wxString::Format("Error al guardar detalles. No se puede continuar.");
                        wxString err_cap = wxString::Format("Error");
                        wxMessageBox(err, err_cap, wxOK | wxICON_ERROR);
                        return;
                    }
                }
                else if (res == wxCANCEL)
                {
                    return;
                }
                else
                {
                    m_book_details_view->set_view_dirty(false);
                    _reevaluate_global_dirty_state();
                }
            }
        }
    }

    _ensure_edit_notebook();
    m_current_app_state = STATE_EDIT;

    m_aui_manager.GetPane("library").Hide();
    m_aui_manager.GetPane("details").Hide();

    m_aui_manager.GetPane("chapters").Left().Layer(0).Position(0).BestSize(300, -1).Show();
    m_aui_manager.GetPane("editor").CentrePane().Show();

    _update_notebook_pages_state(false);

    if (m_chapter_list_view != nullptr)
    {
        m_chapter_list_view->load_chapters(m_current_book_id);
    }

    if (m_edit_notebook != nullptr)
    {
        if (m_edit_notebook->GetPageCount() > 0)
        {
            m_edit_notebook->SetSelection(0);
        }
    }

    _update_toolbar_state(STATE_EDIT);

    bool d_state = m_app_handler->is_application_dirty();
    set_dirty_status_in_title(d_state);

    m_aui_manager.Update();
}

void MainWindow::on_save_current_book_tool_click(wxCommandEvent& event)
{
    if (!m_current_book_id.has_value())
    {
        return;
    }

    bool all_ok = true;

    if (m_current_app_state == STATE_DETAILS)
    {
        if (m_book_details_view != nullptr)
        {
            if (m_book_details_view->is_dirty())
            {
                if (!m_book_details_view->save_changes())
                {
                    all_ok = false;
                }
            }
        }
    }
    else if (m_current_app_state == STATE_EDIT)
    {
        if (m_current_chapter_id.has_value())
        {
            if (m_chapter_content_view != nullptr)
            {
                if (m_chapter_content_view->is_dirty())
                {
                    if (!m_chapter_content_view->save_changes())
                    {
                        all_ok = false;
                    }
                }
            }

            if (m_abstract_idea_view != nullptr)
            {
                if (m_abstract_idea_view->is_dirty())
                {
                    if (!m_abstract_idea_view->save_changes())
                    {
                        all_ok = false;
                    }
                }
            }
        }
    }

    if (all_ok)
    {
        m_app_handler->set_dirty(false);
    }
    else
    {
        wxString err_msg = wxString::Format("Algunos cambios no pudieron guardarse.");
        wxString err_cap = wxString::Format("Error de Guardado");
        wxMessageBox(err_msg, err_cap, wxOK | wxICON_ERROR);
    }

    _reevaluate_global_dirty_state();
}

void MainWindow::on_main_window_chapter_selected(std::optional<int> chapter_id)
{
    bool current_views_dirty = false;

    if (m_current_chapter_id.has_value())
    {
        if (chapter_id != m_current_chapter_id)
        {
            if (m_chapter_content_view != nullptr)
            {
                if (m_chapter_content_view->is_dirty())
                {
                    current_views_dirty = true;
                }
            }

            if (!current_views_dirty && m_abstract_idea_view != nullptr)
            {
                if (m_abstract_idea_view->is_dirty())
                {
                    current_views_dirty = true;
                }
            }
        }
    }

    if (current_views_dirty)
    {
        wxString msg = wxString::Format("Hay cambios sin guardar en el capítulo actual. ¿Desea guardarlos?");
        wxString cap = wxString::Format("Guardar Cambios");

        int res = wxMessageBox(msg, cap, wxYES_NO | wxCANCEL | wxICON_QUESTION);

        if (res == wxYES)
        {
            bool save_ok = true;

            if (m_chapter_content_view != nullptr)
            {
                if (m_chapter_content_view->is_dirty())
                {
                    if (!m_chapter_content_view->save_changes())
                    {
                        save_ok = false;
                    }
                }
            }

            if (m_abstract_idea_view != nullptr)
            {
                if (m_abstract_idea_view->is_dirty())
                {
                    if (!m_abstract_idea_view->save_changes())
                    {
                        save_ok = false;
                    }
                }
            }

            if (!save_ok)
            {
                wxString err = wxString::Format("Error al guardar.");
                wxMessageBox(err, wxString::Format("Error"), wxOK | wxICON_ERROR);

                if (m_chapter_list_view != nullptr)
                {
                    m_chapter_list_view->select_chapter_by_id(m_current_chapter_id);
                }
                return;
            }
        }
        else if (res == wxCANCEL)
        {
            if (m_chapter_list_view != nullptr)
            {
                m_chapter_list_view->select_chapter_by_id(m_current_chapter_id);
            }
            return;
        }
        else
        {
            if (m_chapter_content_view != nullptr)
            {
                m_chapter_content_view->set_view_dirty(false);
            }
            if (m_abstract_idea_view != nullptr)
            {
                m_abstract_idea_view->set_view_dirty(false);
            }
        }
    }

    m_current_chapter_id = chapter_id;

    _load_chapter_data_into_edit_views(chapter_id);

    bool has_selection = chapter_id.has_value();
    _update_notebook_pages_state(has_selection);

    _reevaluate_global_dirty_state();
}

void MainWindow::on_menu_new_book(wxCommandEvent& event)
{
    if (m_app_handler->is_application_dirty())
    {
        if (!_confirm_discard_changes())
        {
            return;
        }
    }

    wxString dlg_cap = wxString::Format("Nuevo Libro - %s", m_app_name);
    NewBookDialog dlg(this, m_app_handler, dlg_cap);

    if (dlg.ShowModal() == wxID_OK)
    {
        std::map<std::string, wxString> data = dlg.get_book_data();

        std::vector<uint8_t> empty_cover;

        auto nid = m_app_handler->create_new_book(
            data["title"],
            data["author"],
            data["synopsis"],
            wxString::Format(""),
            wxString::Format(""),
            empty_cover
        );

        if (nid.has_value())
        {
            m_app_handler->set_dirty(false);
            set_dirty_status_in_title(false);

            if (m_library_view != nullptr)
            {
                m_library_view->load_books();
            }

            on_library_book_selected(nid.value());
        }
    }
}

void MainWindow::on_menu_configuraciones(wxCommandEvent& event)
{
    wxString msg = wxString::Format("El panel de Configuraciones estará disponible próximamente.");
    wxString cap = wxString::Format("Configuración de Sistema");
    wxMessageBox(msg, cap);
}

void MainWindow::on_menu_skin_editor(wxCommandEvent& event)
{
    wxString msg = wxString::Format("Skin Editor: En futuras actualizaciones podrás personalizar los colores.");
    wxString cap = wxString::Format("Personalización Visual");
    wxMessageBox(msg, cap);
}

void MainWindow::on_export_txt(wxCommandEvent& event)
{
    if (!m_current_book_id.has_value())
    {
        return;
    }

    wxString cap = wxString::Format("Exportar a TXT");
    wxString wild = wxString::Format("Archivos TXT (*.txt)|*.txt");

    wxFileDialog saveDlg(this, cap, "", "", wild, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDlg.ShowModal() == wxID_OK)
    {
        auto book = m_app_handler->get_book_details(*m_current_book_id);
        auto chapters = m_app_handler->get_chapters_by_book_id(*m_current_book_id);

        if (book.has_value())
        {
            TxtExporter exp;
            std::string path = saveDlg.GetPath().ToStdString();

            bool success = exp.export_book(book.value(), chapters, path);

            if (success)
            {
                wxString msg = wxString::Format("Libro exportado a TXT exitosamente.");
                wxMessageBox(msg);
            }
            else
            {
                wxString err = wxString::Format("Fallo al exportar el libro.");
                wxMessageBox(err, wxString::Format("Error"), wxOK | wxICON_ERROR);
            }
        }
    }
}

void MainWindow::on_export_docx(wxCommandEvent& event)
{
    if (!m_current_book_id.has_value())
    {
        return;
    }

    wxString cap = wxString::Format("Exportar a DOCX");
    wxString wild = wxString::Format("Archivos DOCX (*.docx)|*.docx");

    wxFileDialog saveDlg(this, cap, "", "", wild, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDlg.ShowModal() == wxID_OK)
    {
        auto book = m_app_handler->get_book_details(*m_current_book_id);
        auto chapters = m_app_handler->get_chapters_by_book_id(*m_current_book_id);

        if (book.has_value())
        {
            DocxExporter exp;
            std::string path = saveDlg.GetPath().ToStdString();
            exp.export_book(book.value(), chapters, path);

            wxString msg = wxString::Format("Aviso: Exportación DOCX invocada (Aviso TXT por ahora).");
            wxMessageBox(msg);
        }
    }
}

void MainWindow::on_export_pdf(wxCommandEvent& event)
{
    if (!m_current_book_id.has_value())
    {
        return;
    }

    wxString cap = wxString::Format("Exportar a PDF");
    wxString wild = wxString::Format("Archivos PDF (*.pdf)|*.pdf");

    wxFileDialog saveDlg(this, cap, "", "", wild, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDlg.ShowModal() == wxID_OK)
    {
        auto book = m_app_handler->get_book_details(*m_current_book_id);
        auto chapters = m_app_handler->get_chapters_by_book_id(*m_current_book_id);

        if (book.has_value())
        {
            PdfExporter exp;
            std::string path = saveDlg.GetPath().ToStdString();
            exp.export_book(book.value(), chapters, path);

            wxString msg = wxString::Format("Aviso: Exportación PDF invocada (Aviso TXT por ahora).");
            wxMessageBox(msg);
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

void MainWindow::on_menu_exit(wxCommandEvent& e)
{
    this->Close();
}

void MainWindow::on_back_to_library_tool_click(wxCommandEvent& e)
{
    wxCommandEvent dummy;
    on_show_library_as_center(dummy);
}

void MainWindow::on_undo_tool_click(wxCommandEvent& e)
{
    if (m_chapter_content_view != nullptr)
    {
        m_chapter_content_view->force_save_if_dirty();
    }
}

void MainWindow::on_redo_tool_click(wxCommandEvent& e)
{
    // Reservado
}

void MainWindow::on_close(wxCloseEvent& event)
{
    _save_state();

    if (m_app_handler->is_application_dirty())
    {
        if (!_confirm_discard_changes())
        {
            event.Veto();
            return;
        }
    }

    this->Destroy();
}

// ============================================================================
// PERSISTENCIA Y CONFIGURACIÓN
// ============================================================================

wxString MainWindow::_get_config_path(const wxString& filename)
{
    wxString user_dir = wxStandardPaths::Get().GetUserDataDir();
    wxString path_sep = wxFileName::GetPathSeparator();

    wxString config_dir = wxString::Format("%s%s%s", user_dir, path_sep, CONFIG_DIR);

    if (!wxFileName::DirExists(config_dir))
    {
        wxFileName::Mkdir(config_dir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }

    return wxString::Format("%s%s%s", config_dir, path_sep, filename);
}

void MainWindow::_save_state()
{
    wxString file_path = _get_config_path(PERSP_FILE);
    std::string path_std = file_path.ToStdString();

    std::ofstream f(path_std);
    if (f.is_open())
    {
        wxString perspective = m_aui_manager.SavePerspective();
        f << perspective.ToStdString();
        f.close();
    }
}

void MainWindow::_load_state()
{
    wxString file_path = _get_config_path(PERSP_FILE);
    std::string path_std = file_path.ToStdString();

    std::ifstream f(path_std);
    if (f.is_open())
    {
        std::stringstream ss;
        ss << f.rdbuf();

        wxString data = wxString::FromUTF8(ss.str().c_str());
        m_aui_manager.LoadPerspective(data);

        f.close();
    }
}