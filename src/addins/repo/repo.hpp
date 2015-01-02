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

#ifndef __REPO_HPP__
#define __REPO_HPP__

#include <list>
#include <string>
#include <queue>
#include <set>

#include <sigc++/sigc++.h>

#include <gtkmm/imagemenuitem.h>

#include "base/macros.hpp"
#include "sharp/dynamicmodule.hpp"
#include "sharp/streamwriter.hpp"
#include "note.hpp"
#include "noteaddin.hpp"
#include "applicationaddin.hpp"



namespace repo {

	class vcsModule : public sharp::DynamicModule {
		public:
		  vcsModule();
	};

	DECLARE_MODULE(vcsModule);

	class vcs : public gnote::ApplicationAddin {
	public:
		static vcs* create() {
			return new vcs;
		}
		vcs();
		virtual ~vcs();
		virtual void initialize() override;
		virtual void shutdown() override;
		virtual int print_notes();
	        virtual bool initialized() override;
	private:
		void export_button_clicked();
		void on_sync_to_repo();		
	
		bool m_initialized;
		
		Glib::RefPtr<Gtk::Action> m_action;
		Glib::ustring m_url;
	};
}

#endif

