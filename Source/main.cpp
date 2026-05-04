/**
* Archivo: main.cpp
* Descripción: Punto de entrada principal para ReinventProse 3.0.
*/

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include "encabezados/AppHandler.h"
#include "encabezados/DBManager.h"
#include "encabezados/MainWindow.h"

class ReinventProseApp : public wxApp {
public:
    virtual bool OnInit() override {
        // Inicializar manejadores de imágenes (PNG, JPEG, etc.)
        wxInitAllImageHandlers();

        try {
            // 1. Determinar ruta de la base de datos (Blindado con Format)
            wxString dbName = wxString::Format("reinventprose_v3_data.db");
            wxString appPath = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();

            // APLICANDO LA REGLA: Eliminamos la concatenación (+)
            // %s para strings, %c para el char del separador
            wxString fullDBPath = wxString::Format("%s%c%s", appPath, wxFileName::GetPathSeparator(), dbName);

            // 2. Obtener instancias Singleton 
            // BLINDAJE: Si la ruta tiene acentos (C:\Users\José\...), forzamos UTF-8 al pasarlo a std::string
            std::string safe_db_path = std::string(fullDBPath.ToUTF8().data());

            DBManager* db_manager = DBManager::get_instance(safe_db_path);
            AppHandler* app_handler = AppHandler::get_instance(db_manager, fullDBPath);

            // 3. Inicializar DB
            app_handler->initialize_database();

            // 4. Lanzar Ventana Principal (Incluso los textos quemados usan Format por consistencia)
            MainWindow* main_window = new MainWindow(nullptr, wxString::Format("ReinventProse 3.0"), app_handler);
            main_window->Show(true);

            return true;
        }
        catch (const std::exception& e) {
            // BLINDAJE NINJA: Envolvemos e.what() con FromUTF8 para que el %s no rompa la memoria
            wxString errorMsg = wxString::Format("Error fatal en el inicio:\n%s", wxString::FromUTF8(e.what()));

            wxMessageBox(errorMsg, wxString::Format("Error Crítico"), wxOK | wxICON_ERROR);
            return false;
        }
    }
};

// Macro que genera el main() real del sistema operativo
wxIMPLEMENT_APP(ReinventProseApp);