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

#include <boost/format.hpp>
#include <glibmm/i18n.h>
#include <git2.h>
#include <libgen.h>
#include <boost/filesystem.hpp>

#include "gitcli.hpp"
#include "debug.hpp"

namespace fs = boost::filesystem ;


namespace repo {

gitcli::gitcli(const std::string &dir, const std::string &url) :
	m_dir(dir), m_url(url)
{
	/* this depends on libgit version */
	git_threads_init();
}

gitcli::~gitcli()
{
	if (m_repo)
		git_repository_free(m_repo);

	/* this depends on libgit version */
	git_threads_shutdown();
}
int gitcli::sync(const std::list<std::string> files)
{
	/* reinitialization of git repo should do no harm */
	init();

	git_oid tree_id;
	add(files, &tree_id);

	commit(&tree_id);

	return 0;
}

int gitcli::init_repo()
{
	int ret;

	ret = git_repository_init(&m_repo, m_dir.c_str(), 0);
	if (ret)
		print_lg2err(ret, "Could not initialize repository");

	return ret;
}

int gitcli::init()
{
	int ret;

	/* TODO: check if repo inited and open instead of initialising it */
	ret = git_repository_open_ext(&m_repo, m_dir.c_str(), 0, NULL);
	if (ret)
		return init_repo();

	DBG_OUT("Repository found\n");
	return ret;
}

int gitcli::add(const std::list<std::string> files, git_oid *tree_id)
{
	int ret;
	git_index *index;

	ret = git_repository_index(&index, m_repo);
	if (ret) {
		print_lg2err(ret,"Could not open repository index");
		return -1;
	}

	for (std::list < std::string >::const_iterator iter =
	     files.begin(); iter != files.end(); ++iter) {
		const std::string & fpath(*iter);
		try {
			std::string fn = fs::basename(fpath.c_str()) +
					 fs::extension(fpath.c_str());
			printf("Note: %s\n", fn.c_str());
			ret = git_index_add_bypath(index, fn.c_str());
			if (ret) {
				std::string str = "Adding " + fn +
					" to index failed";
				print_lg2err(ret, str);
			}
		}
		catch(const std::exception & e) {
			/* TRANSLATORS: first %s is file, second is error */
			ERR_OUT(_
				("Error parsing note XML, skipping \"%s\": %s"),
				fpath.c_str(), e.what());
		}		
	}

	git_index_write(index);
	git_index_write_tree(tree_id, index);
	git_index_free(index);

	return 0;
}

int gitcli::do_initial_commit(git_tree *tree, git_signature *sig)
{
	int ret;
	git_oid commit_id;

	ret = git_commit_create_v(&commit_id, m_repo, "HEAD", sig, sig, NULL,
			"update notes in repo", tree, 0);
	if (ret)
		print_lg2err(ret, "Initial commit failed");

	return ret;
}

int gitcli::do_commit(git_tree *tree, git_signature *sig,
			git_commit *old_head)
{
	int ret;
	git_oid commit_id;
	const git_commit *parents[] = {old_head};


	ret = git_commit_create(&commit_id, m_repo, "HEAD", sig, sig, NULL,
			"update notes in repo", tree, 1, parents);
	if (ret)
		print_lg2err(ret, "Commit failed");

	return 0;
}

int gitcli::commit(git_oid *tree_id)
{
	int ret;
	git_tree *tree;
	git_signature *sig;
	git_commit *old_head = NULL;

	ret = git_tree_lookup(&tree, m_repo, tree_id);
	if (ret) {
		print_lg2err(ret, "Could not find tree OID");
		return -1;
	}

	if (git_signature_default(&sig, m_repo))
		print_lg2err(ret, "Unable to create signature");

	ret = git_revparse_single((git_object **) &old_head, m_repo, "HEAD");
	if (ret) {
		DBG_OUT("No HEAD -> assuming empty repository");
		ret = do_initial_commit(tree, sig);
	} else {
		ret = do_commit(tree, sig, old_head);
	}

	git_tree_free(tree);
	git_signature_free(sig);

	return 0;
}

int gitcli::push()
{
	return 0;
}

void gitcli::print_lg2err(int error, const std::string msg)
{
	const git_error *lg2err;
	const char *lg2msg = "", *lg2spacer = "";
	
	if ((lg2err = giterr_last()) != NULL && lg2err->message != NULL) {
		lg2msg = lg2err->message;
		lg2spacer = " - ";
	}

	/* underscore comes from glib gi18n.h */
	ERR_OUT(_("%s [%d]%s%s\n"), msg.c_str(), error, lg2spacer, lg2msg);
}

}

