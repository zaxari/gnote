/*
 * gnote
 *
 * Copyright (C) 2014 Zahari Doychev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _REPO_PREFERENCES_
#define _REPO_PREFERENCES_

#include <gtkmm/grid.h>
#include <gtkmm/spinbutton.h>

#include "notemanager.hpp"


namespace repo {

extern const char *SCHEMA_REPO_URL;
extern const char *REPO_URL;


class RepoPreferences
   : public Gtk::Grid
{
public:
  RepoPreferences(gnote::NoteManager &);
private:
  void on_apply_button_clicked();
  void on_name_entry_changed();

  Gtk::Button m_apply_button;
  Gtk::Label m_label;
  Gtk::Entry m_entry;

  Glib::ustring m_url;

};

}

#endif

