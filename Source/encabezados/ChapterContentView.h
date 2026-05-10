/**
* File Name: ChapterContentView.h
* Descripción: Vista de escritura refactorizada para soporte WYSIWYG integral.
*              Gestiona la integración del StructuredEditor y la lógica de estado.
* Autor: AutoDoc AI (Protocolo de Estabilidad C++20)
* Versión: 3.2.0 (Detección de Cambios de Formato)
*/

#ifndef CHAPTERCONTENTVIEW_H
#define CHAPTERCONTENTVIEW_H

#include <wx/wx.h>
#include <wx/timer.h>
#include <optional>
#include <string>

// Inyección del componente de edición enriquecida
#include "StructuredEditor.h" 

class AppHandler;

/**
* @class ChapterContentView
* @brief Panel principal para la prosa del capítulo. Actúa como mediador entre
*        el editor visual y el manejador de la base de datos (AppHandler).
*/
class ChapterContentView : public wxPanel
{
public:
    /**
     * Constructor del panel de contenido de prosa.
     * @param parent: Ventana contenedora.
     * @param app_handler: Puntero al manejador de lógica de la aplicación.
     */
    ChapterContentView(wxWindow* parent, AppHandler* app_handler);

    /**
     * Carga el contenido desde la base de datos.
     * Realiza la detección automática entre texto plano legado y XML enriquecido.
     * @param chapter_id: ID del capítulo a cargar o std::nullopt para limpiar.
     */
    void load_content(std::optional<int> chapter_id);

    /**
     * Extrae la estructura visual (XML) del editor y la persiste en la base de datos.
     * @return True si la actualización en SQLite fue exitosa.
     */
    bool save_changes();

    /**
     * Consulta si el editor permite la entrada de datos actualmente.
     */
    bool is_editable() const;

    /**
     * Consulta si existen cambios (letras o formato) pendientes de guardado.
     */
    bool is_dirty() const;

    /**
     * Habilita o deshabilita la interacción con el control y sus herramientas.
     */
    void enable_view(bool enable);

    /**
     * Ejecuta el guardado de emergencia si la vista está en estado modificado.
     */
    bool force_save_if_dirty();

    /**
     * Modifica el estado interno de "vista modificada".
     * @param is_dirty: Booleano para activar/desactivar el flag.
     */
    void set_view_dirty(bool is_dirty = true);

private:
    /**
     * Inicializa los componentes, sustituyendo el wxTextCtrl por el StructuredEditor.
     */
    void _create_controls();

    /**
     * Organiza los sizers para un ajuste perfecto del área visual.
     */
    void _layout_controls();

    /**
     * Sincroniza visualmente el estado del editor con el interruptor "Modo Edición".
     */
    void _update_edit_mode_ui();

    /**
     * Calcula el volumen léxico utilizando la función de extracción plana del motor visual.
     */
    void _update_word_count();

    /**
     * Manejador para el evento de cambio (disparado por texto o formato).
     */
    void on_text_changed(wxCommandEvent& event);

    /**
     * Manejador para el interruptor animado de edición.
     */
    void on_toggle_switch(wxCommandEvent& event);

    // Atributos de Lógica y Estado
    AppHandler* app_handler;
    std::optional<int> chapter_id;
    bool _is_dirty_view;
    bool _loading_data;
    bool _is_in_edit_mode;

    // Referencias Visuales
    wxStaticText* content_label;
    wxStaticText* m_word_count_label;

    // --- EL NUEVO MOTOR VISUAL WYSIWYG ---
    StructuredEditor* m_visual_editor;

    // Interruptor Procedural Animado
    class ModernToggleSwitch* m_toggle;
    wxStaticText* m_toggle_label;

    enum {
        ID_TOGGLE_SWITCH = 4001,
        ID_VISUAL_EDITOR_CTRL = 4002
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // CHAPTERCONTENTVIEW_H