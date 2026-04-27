/*
* Nombre del Archivo: Util.cpp
* Descripción: Implementación de las utilidades de sistema e interfaz.
* Autor: AutoDoc AI (Transcrito a C++20)
* Date: 07/06/2025
*/

#include "../encabezados/Util.h"
#include <wx/filename.h>

namespace Util {

    wxString GetBasePath() {
        // Obtenemos la ruta del ejecutable y extraemos su directorio
        wxString executablePath = wxStandardPaths::Get().GetExecutablePath();
        wxFileName fileName(executablePath);
        return fileName.GetPath();
    }

    std::optional<std::string> GetAssetPath(const std::string& filename) {
        if (filename.empty()) {
            return std::nullopt;
        }

        wxString base_path = GetBasePath();

        // 1. Intentar en carpeta "assets"
        wxFileName fn_assets(base_path, "");
        fn_assets.AppendDir("assets");
        fn_assets.SetFullName(wxString::FromUTF8(filename));

        if (fn_assets.FileExists()) {
            return fn_assets.GetFullPath().ToStdString();
        }

        // 2. Intentar en la raíz del ejecutable
        wxFileName fn_root(base_path, wxString::FromUTF8(filename));
        if (fn_root.FileExists()) {
            return fn_root.GetFullPath().ToStdString();
        }

        return std::nullopt;
    }

    std::optional<wxBitmap> LoadImage(const wxString& filepath) {
        if (filepath.IsEmpty()) {
            return std::nullopt;
        }

        if (!wxFileName::FileExists(filepath)) {
            return std::nullopt;
        }

        try {
            // Cargar el bitmap detectando el tipo automáticamente
            wxBitmap image_bitmap;
            if (image_bitmap.LoadFile(filepath, wxBITMAP_TYPE_ANY)) {
                if (image_bitmap.IsOk()) {
                    return image_bitmap;
                }
            }
        }
        catch (...) {
            // Fallo silencioso ante archivos corruptos
        }
        return std::nullopt;
    }

    wxBitmap LoadIconBitmap(const wxString& icon_name, const wxSize& size) {
        // Intentar obtener la ruta física del archivo
        std::optional<std::string> path_opt = GetAssetPath(icon_name.ToStdString());

        if (path_opt.has_value()) {
            std::optional<wxBitmap> bmp_opt = LoadImage(wxString::FromUTF8(path_opt.value()));
            if (bmp_opt && bmp_opt->IsOk()) {
                wxImage img = bmp_opt->ConvertToImage();
                return wxBitmap(img.Rescale(size.GetWidth(), size.GetHeight(), wxIMAGE_QUALITY_HIGH));
            }
        }

        // Fallback a wxArtProvider si el archivo no existe o falla la carga
        wxArtID art_id = wxART_MISSING_IMAGE;

        if (icon_name.Contains("edit")) {
            art_id = wxART_EDIT;
        }
        else if (icon_name.Contains("new")) {
            art_id = wxART_NEW;
        }
        else if (icon_name.Contains("save")) {
            art_id = wxART_FILE_SAVE;
        }
        else if (icon_name.Contains("library")) {
            art_id = wxART_GO_HOME;
        }
        else if (icon_name.Contains("undo")) {
            art_id = wxART_UNDO;
        }
        else if (icon_name.Contains("redo")) {
            art_id = wxART_REDO;
        }
        else if (icon_name.Contains("bold") || icon_name.Contains("italic") || icon_name.Contains("underline")) {
            art_id = wxART_QUESTION;
        }

        return wxArtProvider::GetBitmap(art_id, wxART_TOOLBAR, size);
    }

    wxBitmap CreatePlaceholderBitmap(int width, int height, const wxString& text) {
        wxBitmap bitmap(width, height);
        wxMemoryDC dc(bitmap);

        // Fondo gris claro uniforme
        dc.SetBackground(wxBrush(wxColour(220, 220, 220)));
        dc.Clear();

        // Dibujar borde de un píxel
        dc.SetPen(wxPen(wxColour(150, 150, 150), 1));
        dc.DrawRectangle(0, 0, width, height);

        if (!text.IsEmpty()) {
            // Configurar fuente y color
            dc.SetTextForeground(wxColour(100, 100, 100));
            wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

            if (width < 100 || height < 50) {
                int current_ps = font.GetPointSize();
                font.SetPointSize(std::max(6, current_ps - 2));
            }
            dc.SetFont(font);

            // Centrar el texto en el bitmap
            wxSize text_size = dc.GetTextExtent(text);
            int x = (width - text_size.GetWidth()) / 2;
            int y = (height - text_size.GetHeight()) / 2;

            dc.DrawText(text, x, y);
        }

        dc.SelectObject(wxNullBitmap);
        return bitmap;
    }

} // namespace Util