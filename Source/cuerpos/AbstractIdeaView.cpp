/**
* @file AbstractIdeaView.cpp
* @brief Implementación de la vista para la gestión de la idea abstracta de un capítulo.
* @author AutoDoc AI (Transcripción a C++20)
* @date 07/06/2025
* @version 1.1.1 (Refactorización de Blindaje Unicode)
* @license MIT License
*/

#include "../encabezados/AbstractIdeaView.h"
#include "../encabezados/AppHandler.h"
#include <wx/sizer.h>
#include <iostream>

/**
* @brief Constructor de la clase AbstractIdeaView.
*/
AbstractIdeaView::AbstractIdeaView(wxWindow* parent, AppHandler* app_handler)
    : wxPanel(parent), app_handler(app_handler), chapter_id(std::nullopt),
    _is_dirty_view(false), _loading_data(false)
{
    // Inicialización de la interfaz de usuario
    _create_controls();
    _layout_controls();

    // Carga inicial: por defecto no hay ningún capítulo seleccionado
    load_idea(std::nullopt);
}

/**
* @brief Crea los componentes visuales (widgets) del panel.
*/
void AbstractIdeaView::_create_controls()
{
    // BLINDAJE: Uso explícito de FromUTF8 para literales duros con caracteres especiales ("Capítulo")
    label_ctrl = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Idea Abstracta del Capítulo:"));

    // Instancia un control de texto multilínea que procesa la tecla Enter
    abstract_idea_ctrl = new wxTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_PROCESS_ENTER
    );

    // Vinculación dinámica del evento para evitar problemas con tablas de eventos estáticas
    abstract_idea_ctrl->Bind(wxEVT_TEXT, &AbstractIdeaView::on_text_changed, this);
}

/**
* @brief Organiza los controles en el panel utilizando sizers para un diseño responsivo.
*/
void AbstractIdeaView::_layout_controls()
{
    // Uso de un sizer vertical para apilar los elementos
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Agrega la etiqueta con un margen de 5 píxeles en todos los bordes
    main_sizer->Add(label_ctrl, 0, wxALL, 5);

    // Añade el control de texto ocupando el espacio restante (proporción 1) y expandiéndose
    main_sizer->Add(abstract_idea_ctrl, 1, wxEXPAND | wxALL, 5);

    this->SetSizer(main_sizer);
}

/**
* @brief Actualiza el estado de modificación ("sucio") de la vista.
*/
void AbstractIdeaView::set_view_dirty(bool is_dirty)
{
    if (this->_is_dirty_view != is_dirty)
    {
        this->_is_dirty_view = is_dirty;

        // Si la vista local está sucia, notificamos al manejador global de la aplicación
        if (this->_is_dirty_view)
        {
            app_handler->set_dirty(true);
        }
    }
}

/**
* @brief Carga la información de la idea abstracta desde la base de datos.
*/
void AbstractIdeaView::load_idea(std::optional<int> id)
{
    // Activamos bandera para evitar que el evento de cambio de texto se dispare durante la carga programática
    _loading_data = true;
    chapter_id = id;

    // Reinicia el contenido del control
    abstract_idea_ctrl->SetValue(wxEmptyString);

    // El control solo es editable si hay un capítulo cargado
    bool is_chapter_present = chapter_id.has_value();
    abstract_idea_ctrl->Enable(is_chapter_present);

    if (is_chapter_present)
    {
        // Recupera los datos del capítulo a través del AppHandler
        std::optional<DBRow> chapter_details = app_handler->get_chapter_details(chapter_id.value());

        if (chapter_details.has_value())
        {
            DBRow details = chapter_details.value();

            // Buscamos la clave específica en el mapa de detalles de manera segura
            if (details.count("abstract_idea"))
            {
                DBValue val = details.at("abstract_idea");
                if (std::holds_alternative<std::string>(val))
                {
                    // Convertimos el std::string en UTF-8 a wxString para visualizar la "Ñ" correctamente
                    wxString idea_text = wxString::FromUTF8(std::get<std::string>(val));
                    abstract_idea_ctrl->SetValue(idea_text);
                }
            }
        }
        else
        {
            // BLINDAJE: Reemplazamos la concatenación << por wxString::Format 
            // y convertimos a StdString de manera segura para la consola
            std::cerr << wxString::Format(
                "Advertencia (AbstractIdeaView): No se encontraron detalles para capítulo ID %d",
                chapter_id.value()
            ).ToStdString() << std::endl;
        }
    }

    // Coloca el cursor al inicio del texto
    abstract_idea_ctrl->SetInsertionPoint(0);

    // Al terminar la carga, la vista no se considera "sucia"
    _is_dirty_view = false;
    _loading_data = false;
}

/**
* @brief Persiste los cambios realizados en el control de texto hacia la Base de Datos.
*/
bool AbstractIdeaView::save_changes()
{
    // Si no hay cambios o no hay un capítulo seleccionado, no hacemos nada
    if (!_is_dirty_view || !chapter_id.has_value())
    {
        return false;
    }

    // --- EL BLINDAJE DEL JEFE ---
    // Usamos wxString::Format para empaquetar el texto y evitar que Windows rompa la codificación local.
    // AppHandler se encargará de hacer el .ToUTF8().data() antes de tocar SQLite.
    wxString text_to_save = wxString::Format("%s", abstract_idea_ctrl->GetValue());

    // Intenta actualizar la información mediante el manejador de la aplicación
    bool success = app_handler->update_chapter_abstract_idea_via_handler(
        chapter_id.value(),
        text_to_save
    );

    if (success)
    {
        // Si tuvo éxito, la vista ya no está sucia
        set_view_dirty(false);
        return true;
    }

    return false;
}

/**
* @brief Manejador disparado cuando el usuario modifica el texto en el control.
*/
void AbstractIdeaView::on_text_changed(wxCommandEvent& event)
{
    // Ignoramos el evento si estamos cargando datos programáticamente o si el control está deshabilitado
    if (_loading_data || !abstract_idea_ctrl->IsEnabled())
    {
        event.Skip();
        return;
    }

    // Marcamos la vista como modificada
    set_view_dirty(true);
    event.Skip();
}

/**
* @brief Consulta si la vista actual tiene cambios pendientes de guardado.
*/
bool AbstractIdeaView::is_dirty() const
{
    return _is_dirty_view;
}

/**
* @brief Habilita o deshabilita la interacción del usuario con la vista.
*/
void AbstractIdeaView::enable_view(bool enable)
{
    this->Enable(enable);
    abstract_idea_ctrl->Enable(enable && chapter_id.has_value());
}