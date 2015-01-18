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

/* Shamelessly borrowed from http://stackoverflow.com/questions/3417837/
 * with permission of the original author, Martin Pool.
 * http://sourcefrog.net/weblog/software/languages/C/unused.html
 */
#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

namespace repo {

	class gitcli {
public:
		gitcli(const std::string &dir, const std::string &url);
		virtual ~gitcli();

		virtual int sync(const std::list<std::string> files);
		virtual int update();
private:
		virtual int init();
		virtual int add(const std::list<std::string> files, 
				git_oid *tree_id);

		
		virtual int commit(git_oid *tree_id);
		
		virtual int clone();
		virtual int get_remote(git_remote **remote);
		virtual int fetch(git_remote *remote);
		virtual int push();
		virtual int analyze_merge(
				const git_annotated_commit **merge_heads,
				size_t len);
		virtual int merge();
		virtual int do_commit_merge(git_tree *tree, 
					    git_signature *sig, 
					    git_buf msg);

		virtual int finalize_merge();
		virtual int get_conflicts();
		virtual int status();

		/* base on libgit2 examples check_lg2 function */
		virtual void print_lg2err(int error, const std::string msg);

		virtual int do_initial_commit(git_tree *tree, 
					      git_signature *sig);
		virtual int do_commit(git_tree *tree, git_signature *sig, 
				      git_commit *old_head);

		virtual int init_repo();
		virtual int get_repo();

		/* callbacks */
		static int progress_cb(const char *str, int len, void *data);
		static int update_cb(const char *refname, const git_oid *a,
				const git_oid *b, void *data);

		static int cred_acquire_cb(git_cred **out,
				const char * UNUSED(url),
				const char * UNUSED(username_from_url),
				unsigned int UNUSED(allowed_types),
				void * UNUSED(payload));


		std::string m_dir;
		std::string m_url;
		bool m_repo_inited;
	
		/* libgit vars */
		git_repository *m_repo;
		git_remote *m_remote;
		git_reference *m_branch, *m_upstream;
	};
}

#endif
