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

	const char *SCHEMA_REPO_URL = "org.gnome.gnote.repo-url";
	const char *REPO_URL = "repo-url";

	 RepoPreferences::RepoPreferences(gnote::NoteManager &)
	:m_apply_button(_("_Apply"), true)
	, m_label(_("Git Server URL:  ")) {
		printf("\n>>>>>> %s called  <<<<<<<\n", __func__);
		set_row_spacing(12);
		m_label.set_line_wrap(true);
		m_label.set_use_markup(true);
		m_label.set_vexpand(true);
		attach(m_label, 0, 0, 1, 1);

		m_entry.set_hexpand(true);
		m_entry.signal_changed().
		    connect(sigc::
			    mem_fun(*this,
				    &RepoPreferences::on_name_entry_changed));
		attach(m_entry, 1, 0, 2, 1);

		m_apply_button.set_use_underline(true);
		m_apply_button.signal_clicked().
		    connect(sigc::
			    mem_fun(*this,
				    &RepoPreferences::on_apply_button_clicked));
		attach(m_apply_button, 2, 2, 1, 1);

		m_url =
		    gnote::Preferences::obj().
		    get_schema_settings(SCHEMA_REPO_URL)->get_string(REPO_URL);
		printf("%s: current url: %s \n", __func__, m_url.c_str());

	} void RepoPreferences::on_apply_button_clicked() {
		printf(">>>> %s calledi %s\n",
		       __func__, m_entry.get_text().c_str());

	}

	void RepoPreferences::on_name_entry_changed() {
		printf(">>>> %s called\n", __func__);
		m_url = m_entry.get_text();
		gnote::Preferences::obj().get_schema_settings(SCHEMA_REPO_URL)->
		    set_string(REPO_URL, m_url);
	}

}
