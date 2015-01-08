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

#ifndef __GITCLI_HPP__
#define __GITCLI_HPP__

#include <list>
#include <string>
#include <queue>
#include <set>

#include <git2.h>

namespace repo {

	class gitcli {
public:
		gitcli(const std::string &dir, const std::string &url);
		virtual ~gitcli();

		virtual int sync(const std::list<std::string> files);
private:
		virtual int init();
		virtual int add(const std::list<std::string> files, 
				git_oid *tree_id);

		virtual int commit(git_oid *tree_id);
		virtual int push();

		/* base on libgit2 examples check_lg2 function */
		virtual void print_lg2err(int error, const std::string msg);

		virtual int do_initial_commit(git_tree *tree, 
					      git_signature *sig);
		virtual int do_commit(git_tree *tree, git_signature *sig, 
				      git_commit *old_head);

		virtual int init_repo();

		std::string m_dir;
		std::string m_url;
		bool m_repo_inited;
	
		/* libgit vars */
		git_repository *m_repo;
	};
}

#endif
