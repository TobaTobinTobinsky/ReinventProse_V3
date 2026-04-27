/*
* File Name: AbstractIdeaView.h
* Description: Implementa un panel de wxWidgets para visualizar y editar la idea abstracta de un capítulo.
* Author: AutoDoc AI (Transcripción a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef ABSTRACTIDEAVIEW_H
#define ABSTRACTIDEAVIEW_H

#include <wx/wx.h>
#include <optional>
#include <string>

// Forward declaration para evitar dependencias circulares
class AppHandler;

class AbstractIdeaView : public wxPanel {
public:
    /**
     * Inicializa el panel AbstractIdeaView.
     * @param parent: La ventana padre de este panel.
     * @param app_handler: Una instancia del manejador de la aplicación.
     */
    AbstractIdeaView(wxWindow* parent, AppHandler* app_handler);

    /**
     * Carga la idea abstracta para un capítulo específico.
     * @param chapter_id: El ID del capítulo, o std::nullopt para limpiar.
     */
    void load_idea(std::optional<int> chapter_id);

    /**
     * Guarda los cambios en la idea abstracta.
     * @return True si se guardó exitosamente.
     */
    bool save_changes();

    /**
     * Verifica si la vista tiene cambios no guardados.
     */
    bool is_dirty() const;

    /**
     * Habilita o deshabilita la interacción con el control principal.
     * @param enable: Booleano para habilitar/deshabilitar.
     */
    void enable_view(bool enable);

    /**
     * Fuerza el estado de "sucio" (modificado) de la vista.
     */
    void set_view_dirty(bool is_dirty = true);

private:
    // Métodos privados de inicialización y lógica
    void _create_controls();
    void _layout_controls();

    // Manejador de eventos (Conectado dinámicamente vía Bind)
    void on_text_changed(wxCommandEvent& event);

    // Atributos de lógica de negocio
    AppHandler* app_handler;
    std::optional<int> chapter_id;
    bool _is_dirty_view;
    bool _loading_data;

    // Controles visuales
    wxTextCtrl* abstract_idea_ctrl;
    wxStaticText* label_ctrl;
};

#endif // ABSTRACTIDEAVIEW_H