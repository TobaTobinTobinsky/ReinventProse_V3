/**
* File Name: Exporter.cpp
* Descripción: Implementación de la exportación de libros.
*/

#include "../encabezados/Exporter.h"
#include <fstream>
#include <algorithm>
#include <regex>
#include <iostream>

// En esta versión base de C++, simulamos que no están disponibles
// hasta que no enlaces duckx (Word) o libharu (PDF).
const bool PYTHON_DOCX_AVAILABLE_CONST = false;
const bool REPORTLAB_AVAILABLE_CONST = false;

// --- Implementación de BaseExporter ---

std::string BaseExporter::_html_unescape(std::string text) {
    std::vector<std::pair<std::string, std::string>> entities = {
        {"&nbsp;", " "}, {"&amp;", "&"}, {"&lt;", "<"},
        {"&gt;", ">"}, {"&quot;", "\""}, {"&apos;", "'"}
    };

    for (const auto& pair : entities) {
        size_t pos = 0;
        while ((pos = text.find(pair.first, pos)) != std::string::npos) {
            text.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length();
        }
    }
    return text;
}

std::string BaseExporter::_clean_html_for_plaintext(const std::string& html_content) {
    if (html_content.empty()) return "";

    std::string text = html_content;

    // Reemplaza <br> y <br/> con saltos de línea
    std::regex br_re(R"(<br\s*/?>)", std::regex_constants::icase);
    text = std::regex_replace(text, br_re, "\n");

    // Elimina todas las demás etiquetas HTML
    std::regex tags_re(R"(<[^>]+>)");
    text = std::regex_replace(text, tags_re, "");

    // Decodifica entidades HTML
    text = _html_unescape(text);

    return text;
}

// --- Función auxiliar para extraer strings seguros de DBRow ---
std::string get_string_from_dbrow(const DBRow& row, const std::string& key, const std::string& default_val = "") {
    if (row.count(key) && std::holds_alternative<std::string>(row.at(key))) {
        return std::get<std::string>(row.at(key));
    }
    return default_val;
}

long long get_int_from_dbrow(const DBRow& row, const std::string& key, long long default_val = 0) {
    if (row.count(key) && std::holds_alternative<long long>(row.at(key))) {
        return std::get<long long>(row.at(key));
    }
    return default_val;
}

// --- Implementación de TxtExporter ---

bool TxtExporter::export_book(const DBRow& book_data, const std::vector<DBRow>& chapters_data, const std::string& output_path) {
    try {
        // Abrimos el archivo en modo escritura binaria para asegurar que no se corrompa el UTF-8 en Windows
        std::ofstream f(output_path, std::ios::out | std::ios::binary);
        if (!f.is_open()) return false;

        // Título del libro
        std::string title = get_string_from_dbrow(book_data, "title", "Libro Sin Título");
        std::transform(title.begin(), title.end(), title.begin(), ::toupper);

        f << title << "\n";
        f << std::string(title.length(), '=') << "\n\n";

        // Autor
        std::string author = get_string_from_dbrow(book_data, "author", "Autor Desconocido");
        f << "Por: " << author << "\n\n";
        f << std::string(30, '-') << "\n\n";

        // Ordenar capítulos (Copiamos para poder ordenar)
        std::vector<DBRow> sorted_chapters = chapters_data;
        std::sort(sorted_chapters.begin(), sorted_chapters.end(), [](const DBRow& a, const DBRow& b) {
            return get_int_from_dbrow(a, "chapter_number") < get_int_from_dbrow(b, "chapter_number");
            });

        // Índice
        f << "\xC3\x8DNDICE\n"; // "ÍNDICE" en UTF-8 hexadecimal
        f << "------\n";
        if (sorted_chapters.empty()) {
            f << "(No hay capítulos)\n";
        }
        else {
            for (const auto& chapter : sorted_chapters) {
                long long ch_num = get_int_from_dbrow(chapter, "chapter_number");
                std::string ch_title = _clean_html_for_plaintext(get_string_from_dbrow(chapter, "title"));
                f << "Capítulo " << ch_num << ": " << ch_title << "\n";
            }
        }
        f << "\n" << std::string(30, '-') << "\n\n";

        // Prólogo
        std::string prologue = get_string_from_dbrow(book_data, "prologue");
        if (!prologue.empty()) {
            f << "PR\xC3\x93LOGO\n"; // "PRÓLOGO" en UTF-8
            f << "---------\n";
            f << _clean_html_for_plaintext(prologue) << "\n\n";
            f << std::string(30, '-') << "\n\n";
        }

        // Contenido de capítulos
        if (sorted_chapters.empty()) {
            f << "No hay contenido de capítulos para exportar.\n";
        }
        else {
            for (const auto& chapter : sorted_chapters) {
                long long ch_num = get_int_from_dbrow(chapter, "chapter_number");
                std::string ch_title = _clean_html_for_plaintext(get_string_from_dbrow(chapter, "title"));

                std::string header = "CAPITULO " + std::to_string(ch_num) + ": " + ch_title;
                std::transform(header.begin(), header.end(), header.begin(), ::toupper);

                f << header << "\n";
                f << std::string(header.length(), '-') << "\n";

                std::string content = get_string_from_dbrow(chapter, "content");
                f << _clean_html_for_plaintext(content) << "\n\n";
            }
        }

        f.close();
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error inesperado durante la exportación TXT: " << e.what() << std::endl;
        return false;
    }
}

// --- Implementación de DocxExporter (Stub) ---

bool DocxExporter::export_book(const DBRow& book_data, const std::vector<DBRow>& chapters_data, const std::string& output_path) {
    // Al no tener python-docx, generamos un aviso en texto plano.
    std::ofstream f(output_path);
    if (f.is_open()) {
        f << "Exportacion DOCX no disponible nativamente en esta compilacion de C++.\n";
        f << "Por favor, utilice la exportacion a TXT.\n";
        f.close();
        return true; // Retorna true para que no falle la interfaz
    }
    return false;
}

// --- Implementación de PdfExporter (Stub) ---

bool PdfExporter::export_book(const DBRow& book_data, const std::vector<DBRow>& chapters_data, const std::string& output_path) {
    // Al no tener reportlab, generamos un aviso.
    std::ofstream f(output_path);
    if (f.is_open()) {
        f << "Exportacion PDF no disponible nativamente en esta compilacion de C++.\n";
        f << "Por favor, utilice la exportacion a TXT.\n";
        f.close();
        return true;
    }
    return false;
}