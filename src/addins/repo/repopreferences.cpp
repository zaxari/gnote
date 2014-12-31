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


#include <glibmm/i18n.h>

#include "iconmanager.hpp"
#include "utils.hpp"

#include "repopreferences.hpp"
#include "preferences.hpp"

using namespace gnote;

namespace repo {

const char *SCHEMA_NOTE_DIRECTORY_WATCHER = "org.gnome.gnote.note-directory-watcher";
const char *CHECK_INTERVAL = "check-interval";


RepoPreferences::RepoPreferences(gnote::NoteManager &)
  : /* utils::HIGMessageDialog(parent, f, Gtk::MESSAGE_OTHER, Gtk::BUTTONS_NONE), */
    m_check_interval(1)
{
  printf("\n>>>>>> %s called  \n", __func__);
  /* Gtk::Label *label = manage(new Gtk::Label(_("_Directory check interval:"), true));
  attach(*label, 0, 0, 1, 1);
  m_check_interval.set_range(5, 300);
  m_check_interval.set_increments(1, 5);
  m_check_interval.signal_value_changed()
    .connect(sigc::mem_fun(*this, &RepoPreferences::on_interval_changed));
  m_check_interval.set_value(
    gnote::Preferences::obj().get_schema_settings(SCHEMA_NOTE_DIRECTORY_WATCHER)->get_int(CHECK_INTERVAL));
  attach(m_check_interval, 1, 0, 1, 1);*/

      // set_title(_("Create Notebook"));
      Gtk::Table *table = manage(new Gtk::Table (2, 2, false));
      table->set_col_spacings(6);
      
      Gtk::Label *label = manage(new Gtk::Label (_("N_otebook name:"), true));
      label->property_xalign() = 0;
      label->show ();
      
      m_nameEntry.signal_changed().connect(
        sigc::mem_fun(*this, &RepoPreferences::on_name_entry_changed));
      m_nameEntry.set_activates_default(true);
      m_nameEntry.show ();
      label->set_mnemonic_widget(m_nameEntry);
      
      m_errorLabel.property_xalign() = 0;
      m_errorLabel.set_markup(
        str(boost::format("<span foreground='red' style='italic'>%1%</span>")
            % _("Name already taken")));
      
      table->attach (*label, 0, 1, 0, 1);
      table->attach (m_nameEntry, 1, 2, 0, 1);
      table->attach (m_errorLabel, 1, 2, 1, 2);
      table->show ();
      
     // set_extra_widget(table);
      
      add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL, false);
      add_button (IconManager::obj().get_icon(IconManager::NOTEBOOK_NEW, 16),
                  // Translation note: This is the Create button in the Create
                  // New Note Dialog.
                  _("C_reate"), Gtk::RESPONSE_OK, true);
      
      // Only let the Ok response be sensitive when
      // there's something in nameEntry
      set_response_sensitive (Gtk::RESPONSE_OK, false);
      m_errorLabel.hide ();


}

void RepoPreferences::on_interval_changed()
{
  gnote::Preferences::obj().get_schema_settings(SCHEMA_NOTE_DIRECTORY_WATCHER)->set_int(
    CHECK_INTERVAL, m_check_interval.get_value_as_int());
}

void RepoPreferences::on_name_entry_changed()
{
	printf(">>>> %s called\n", __func__);
}

}

