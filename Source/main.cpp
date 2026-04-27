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
            // 1. Determinar ruta de la base de datos
            wxString dbName = "reinventprose_v3_data.db";
            wxString appPath = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
            wxString fullDBPath = appPath + wxFileName::GetPathSeparator() + dbName;

            // 2. Obtener instancias Singleton (Convertimos wxString a std::string para el DBManager)
            DBManager* db_manager = DBManager::get_instance(fullDBPath.ToStdString());
            AppHandler* app_handler = AppHandler::get_instance(db_manager, fullDBPath);

            // 3. Inicializar DB
            app_handler->initialize_database();

            // 4. Lanzar Ventana Principal
            MainWindow* main_window = new MainWindow(nullptr, "ReinventProse 3.0", app_handler);
            main_window->Show(true);

            return true;
        }
        catch (const std::exception& e) {
            wxMessageBox(wxString::Format("Error fatal en el inicio:\n%s", e.what()),
                "Error Crítico", wxOK | wxICON_ERROR);
            return false;
        }
    }
};

// Macro que genera el main() real del sistema operativo
wxIMPLEMENT_APP(ReinventProseApp);