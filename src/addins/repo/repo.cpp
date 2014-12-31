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
#include "sharp/string.hpp"


#include <boost/format.hpp>
#include "repo.hpp"


namespace repo {

vcsModule::vcsModule()
{
	ADD_INTERFACE_IMPL(vcs);
}

vcs::vcs()
{
	printf("%s: called\n", __func__);
}

vcs::~vcs()
{
	printf("%s: called\n", __func__);
	
}

int vcs::print_notes()
{

	return 0;
}

int vcs::do_sync()
{
	printf("%s: called\n", __func__);

	return 0;
}

void vcs::initialize()
{
	printf(">>>> %s called\n", __func__);
	return;
}

void vcs::shutdown()
{
	printf(">>>> %s called\n", __func__);
	return;
}

void vcs::on_note_opened()
{
	printf(">>>> %s called\n", __func__);
	Glib::RefPtr<gnote::NoteWindow::NonModifyingAction> action =
	gnote::NoteWindow::NonModifyingAction::create("PushNoteAction", _("Push note"),	_("Push note to repo"));
	printf("<<<< DONE\n"); 
	action->signal_activate().connect(
		sigc::mem_fun(*this, &vcs::export_button_clicked));
	add_note_action(action, gnote::PUSH_TO_REPO_ORDER);

	printf("pushed\n");
}

void vcs::export_button_clicked()
{
	printf("%s: called\n", __func__);
	return;
}

}

