/*
* File Name: Exporter.h
* Descripción: Módulo para gestionar la exportación de datos de libros a TXT, DOCX y PDF.
* Autor: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 1.0.0
* License: MIT License
*/

#ifndef EXPORTER_H
#define EXPORTER_H

#include <string>
#include <vector>
#include <optional>
#include "DBManager.h" // Importamos DBRow directamente para evitar conflictos de tipos

// Banderas de disponibilidad (Para futuras integraciones de librerías)
extern const bool PYTHON_DOCX_AVAILABLE_CONST;
extern const bool REPORTLAB_AVAILABLE_CONST;

/**
 * Clase base abstracta para los exportadores de formatos de libro.
 */
class BaseExporter {
public:
    virtual ~BaseExporter() = default;

    /**
     * Método principal de exportación (virtual puro).
     */
    virtual bool export_book(const DBRow& book_data,
        const std::vector<DBRow>& chapters_data,
        const std::string& output_path) = 0;

protected:
    /**
     * Limpia una cadena de contenido HTML para obtener texto plano.
     * (Mantenida por si quedan rastros de la versión anterior)
     */
    std::string _clean_html_for_plaintext(const std::string& html_content);

    /**
     * Desescapa entidades HTML básicas (&amp;, &lt;, etc.)
     */
    std::string _html_unescape(std::string text);
};

/**
 * Exportador para generar archivos de texto plano (.txt).
 */
class TxtExporter : public BaseExporter {
public:
    bool export_book(const DBRow& book_data,
        const std::vector<DBRow>& chapters_data,
        const std::string& output_path) override;
};

/**
 * Exportador para generar archivos Microsoft Word (.docx).
 */
class DocxExporter : public BaseExporter {
public:
    bool export_book(const DBRow& book_data,
        const std::vector<DBRow>& chapters_data,
        const std::string& output_path) override;
};

/**
 * Exportador para generar archivos PDF.
 */
class PdfExporter : public BaseExporter {
public:
    bool export_book(const DBRow& book_data,
        const std::vector<DBRow>& chapters_data,
        const std::string& output_path) override;
};

#endif // EXPORTER_H