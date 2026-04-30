/*
* File Name: BookDetailsView.h
* Description: Panel de wxWidgets para mostrar y editar los detalles principales de un libro.
* Author: AutoDoc AI (Transcripción literal a C++20)
* Date: 07/06/2025
* Version: 2.0.0
* License: MIT License
*/

#ifndef BOOKDETAILSVIEW_H
#define BOOKDETAILSVIEW_H

#include <wx/wx.h>
#include <wx/statbmp.h>
#include <optional>
#include <string>
#include <vector>
#include <cstdint>

class AppHandler;

class BookDetailsView : public wxPanel
{
public:
	BookDetailsView(wxWindow* parent, AppHandler* app_handler);

	void load_book_details(std::optional<int> book_id);
	bool save_changes();
	bool is_dirty() const;
	void enable_view(bool enable);
	void set_view_dirty(bool is_dirty = true);

private:
	void _create_controls();
	void _layout_controls();
	void _update_controls_state();

	void on_text_changed(wxCommandEvent& event);
	void on_image_clicked(wxMouseEvent& event);

	AppHandler* app_handler;
	std::optional<int> book_id;
	bool _is_dirty_view;
	bool _loading_data;

	// AHORA ALMACENAMOS LOS BYTES CRUDOS EN LUGAR DE UNA RUTA
	std::optional<std::vector<uint8_t>> m_current_cover_image_data;

	wxStaticText* title_label;
	wxStaticText* author_label;
	wxStaticText* synopsis_label;
	wxStaticText* prologue_label;
	wxStaticText* back_cover_text_label;
	wxStaticText* cover_image_label_text;

	wxTextCtrl* title_ctrl;
	wxTextCtrl* author_ctrl;
	wxTextCtrl* synopsis_ctrl;
	wxTextCtrl* prologue_ctrl;
	wxTextCtrl* back_cover_text_ctrl;

	wxStaticBitmap* cover_image_display;
};

#endif // BOOKDETAILSVIEW_H