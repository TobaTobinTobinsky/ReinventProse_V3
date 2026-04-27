/*
* Nombre del Archivo: Util.h
* Descripción: Este módulo proporciona funciones de utilidad esenciales para la aplicación,
*              incluyendo la gestión de rutas de assets, carga de imágenes y iconos,
*              y creación de bitmaps de placeholder.
* Autor: AutoDoc AI (Transcripción a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef UTIL_H
#define UTIL_H

#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/image.h>
#include <wx/settings.h>
#include <wx/dcmemory.h>
#include <wx/stdpaths.h>
#include <filesystem>
#include <string>
#include <optional>

namespace fs = std::filesystem;

namespace Util {
    /**
    * Determina la ruta base de la aplicación.
    * @return wxString con la ruta absoluta al directorio del ejecutable.
    */
    wxString GetBasePath();

    /**
    * Construye la ruta completa a un archivo de asset buscando en /assets o en la raíz.
    * @param filename Nombre del archivo (ej. "app_icon.ico").
    * @return std::optional con la ruta completa si existe.
    */
    std::optional<std::string> GetAssetPath(const std::string& filename);

    /**
    * Carga una imagen desde una ruta de archivo y la convierte en un objeto wxBitmap.
    * @param filepath Ruta al archivo de imagen.
    * @return std::optional con el wxBitmap si la carga fue exitosa.
    */
    std::optional<wxBitmap> LoadImage(const wxString& filepath);

    /**
    * Carga un icono como un objeto wxBitmap del tamaño especificado.
    * Si falla el archivo, recurre al wxArtProvider.
    * @param icon_name Nombre del archivo de icono.
    * @param size Tamaño deseado.
    * @return wxBitmap cargado.
    */
    wxBitmap LoadIconBitmap(const wxString& icon_name, const wxSize& size = wxSize(16, 16));

    /**
    * Crea un objeto wxBitmap de placeholder con un color de fondo, un borde y texto opcional.
    * @param width Ancho en píxeles.
    * @param height Alto en píxeles.
    * @param text Texto a mostrar en el centro.
    * @return wxBitmap generado.
    */
    wxBitmap CreatePlaceholderBitmap(int width, int height, const wxString& text = "Sin Imagen");
}

// --- Funciones de conveniencia globales para simplificar el código de las vistas ---

inline wxBitmap load_icon_bitmap(const wxString& name, const wxSize& sz = wxSize(16, 16)) {
    return Util::LoadIconBitmap(name, sz);
}

inline wxBitmap create_placeholder_bitmap(int w, int h, const wxString& t = "Sin Imagen") {
    return Util::CreatePlaceholderBitmap(w, h, t);
}

inline std::optional<wxBitmap> load_image(const wxString& path) {
    return Util::LoadImage(path);
}

#endif // UTIL_H