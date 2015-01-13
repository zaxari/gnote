/*
 * gnote
 *
 * Note syncing using version control systems
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

#include <giomm/dbusproxy.h>
#include <glibmm/i18n.h>

#include "debug.hpp"

#include "iactionmanager.hpp"
#include "notewindow.hpp"
#include "notemanager.hpp"
#include "sharp/string.hpp"
#include "repopreferencesfactory.hpp"
#include "repopreferences.hpp"
#include "sharp/directory.hpp"
#include "sharp/dynamicmodule.hpp"

#include <boost/format.hpp>
#include "repo.hpp"

namespace repo {

vcsModule::vcsModule()
{
	ADD_INTERFACE_IMPL(vcs);
	ADD_INTERFACE_IMPL(RepoPreferencesFactory);
}

vcs::vcs(): m_initialized(false)
{

}

vcs::~vcs()
{
	printf("%s: called\n", __func__);

}

int vcs::print_notes()
{

	return 0;
}

void vcs::initialize()
{
	if (m_initialized)
		return;

	Glib::RefPtr < Gio::Settings > settings =
	    gnote::Preferences::obj().
	    get_schema_settings(SCHEMA_REPO_URL);
	m_url = settings->get_string(REPO_URL);
	m_initialized = true;

	gnote::NoteManager &manager(note_manager());
	const Glib::ustring &note_path = manager.notes_dir();

	/* get ref. to the git client */
	shared_ptr<gitcli> gc(new gitcli(note_path, m_url));
	m_git = gc;

	m_git->update();

	if (m_action == 0) {
		m_action = Gtk::Action::create();
		m_action->set_name("SyncRepository");
		m_action->set_label(_("Sync to repo"));
		m_action->signal_activate().
		    connect(sigc::
			    mem_fun(*this, &vcs::on_sync_to_repo));
		gnote::IActionManager::
		obj().add_main_window_search_action(m_action, 150);
	}
}

void vcs::shutdown()
{
	m_initialized = false;
	return;
}

bool vcs::initialized()
{
	return m_initialized;
}

void vcs::on_sync_to_repo()
{
	gnote::NoteManager &manager(note_manager());
	const Glib::ustring &note_path = manager.notes_dir();

	std::list < std::string > files;
	sharp::directory_get_files_with_ext(manager.notes_dir(),
					    ".note", files);

	m_git->sync(files);
}

}
