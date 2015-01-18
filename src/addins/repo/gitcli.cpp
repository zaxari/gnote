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
	git_libgit2_init();
}

gitcli::~gitcli()
{
	if (m_repo)
		git_repository_free(m_repo);

	git_libgit2_shutdown();
}
int gitcli::sync(const std::list<std::string> files)
{
	git_oid tree_id;
	int ret;

	ret = repo_status();
	if (!ret)
		return 0;

	if (ret < 0)
		return -1;

	add(files, &tree_id);

	commit(&tree_id);

	push();

	printf("sync finished\n");
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

int gitcli::clone()
{
	git_clone_options opts;
	int ret;

	DBG_OUT("Cloning %s\n", m_url.c_str());
	ret = git_clone(&m_repo, m_url.c_str(), m_dir.c_str(), &opts);
	if (ret)
		print_lg2err(ret,"Unable to clone");
	else
		git_repository_free(m_repo);

	return ret;
}

int gitcli::repo_status()
{
	git_status_list *status;
	git_status_options opt = GIT_STATUS_OPTIONS_INIT;
	int ret;

	opt.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
	opt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
		GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
		GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

	ret = git_status_list_new(&status, m_repo, &opt);
	if (ret) {
		print_lg2err(ret, "get status failed.");
		return -1;
	}

	ret = git_status_list_entrycount(status);
	printf("changed files: %d\n", ret);

	git_status_list_free(status);

	return ret;
}

void gitcli::set_remote_callbacks(git_remote *remote)
{
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;

	callbacks.update_tips = &repo::gitcli::update_cb;
	callbacks.sideband_progress = &repo::gitcli::progress_cb;
	callbacks.credentials = &repo::gitcli::cred_acquire_cb;
	git_remote_set_callbacks(remote, &callbacks);
}

int gitcli::get_remote(git_remote **remote)
{
	int ret;

	ret = git_remote_lookup(remote, m_repo, "origin");
	if (!ret) {
		DBG_OUT("remote found\n");
		set_remote_callbacks(*remote);
		return 0;
	}

	DBG_OUT("creating anonymous remote");
	ret = git_remote_create(remote, m_repo, "origin", m_url.c_str());
	if (ret)
		print_lg2err(ret, "Unable to create remote");
	else {
		printf("remote @ %p\n", *remote);
		set_remote_callbacks(*remote);
	}
	return ret;
}

int gitcli::fetch(git_remote *remote)
{
	int ret;

	ret = git_remote_connect(remote, GIT_DIRECTION_FETCH);
	if (ret) {
		print_lg2err(ret, "remote connect failed");
		return -1;
	}

	ret = git_remote_download(remote, NULL);
	if (ret)
		print_lg2err(ret, "remote download failed");

	git_remote_disconnect(remote);
	
	return 0;
}

int gitcli::do_commit_merge(git_tree *tree, git_signature *sig, git_buf msg)
{
	git_oid commit_id;
	git_commit *parents[2];
	int ret;

	ret = git_commit_lookup(&parents[0], m_repo,
				git_reference_target(m_branch));
	if (ret) {
		print_lg2err(ret, "failed to lookup current branch paraent");
		return -1;
	}
	ret = git_commit_lookup(&parents[1], m_repo,
				git_reference_target(m_upstream));
	if (ret) {
		print_lg2err(ret, "failed to lookup current branch paraent");
		return -1;
	}

	ret = git_commit_create(&commit_id, m_repo, "HEAD", sig, sig,
				NULL, msg.ptr, tree, 2,
				(const git_commit **) parents);

	if (ret) {
		print_lg2err(ret, "failed to create commit");
	} else
		git_repository_state_cleanup(m_repo);

	return ret;
}

int gitcli::finalize_merge()
{
	git_index *index;
	git_tree *tree;
	git_oid tree_id;
	git_signature *user;
	git_buf msg = {0};
	int ret;

	ret = git_repository_index(&index, m_repo);
	if (ret) {
		print_lg2err(ret,"get index failed");
		return -1;
	}

	ret = git_index_write_tree(&tree_id, index);

	git_index_free(index);

	if (ret = git_signature_default(&user, m_repo)) {
		print_lg2err(ret,"failed to get user signature");
		return -1;
	}

	if (ret = git_repository_message(&msg, m_repo)) {
		print_lg2err(ret, "failed to get repository message");
		return -1;
	}

	ret = git_tree_lookup(&tree, m_repo, &tree_id);
	if (!ret)
		ret = do_commit_merge(tree, user, msg);
	else
		print_lg2err(ret, "fail to lookup tree");

	git_signature_free(user);
	git_tree_free(tree);

	return ret;
}

int gitcli::get_conflicts()
{
	git_index *index;
	int ret;

	ret = git_repository_index(&index, m_repo);
	if (ret) {
		print_lg2err(ret, "get index failed");
		return -1;
	}

	ret = git_index_has_conflicts(index);

	git_index_free(index);

	return ret;
}

int gitcli::analyze_merge(const git_annotated_commit **merge_heads, size_t len)
{
	git_merge_analysis_t merge_analysis;
	git_merge_preference_t merge_pref = GIT_MERGE_PREFERENCE_NONE;
	int ret;

	ret = git_merge_analysis(&merge_analysis, &merge_pref, m_repo,
				merge_heads, len);
	if (ret) {
		print_lg2err(ret, "merge analysis failed!");
		return 0;
	}

	ret = 1;
	if (merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
		printf("notes up-to-date no need to merge (%d)\n",
			(int)merge_analysis);
		ret = 0;
	}

	return ret;
}

int gitcli::merge()
{
	int ret;
	git_annotated_commit *merge_heads[1];
	m_branch = m_upstream = NULL;

	ret = git_repository_head(&m_branch, m_repo);
	if (ret) {
		print_lg2err(ret, "failed to lookup current branch");
		return -1;
	}
	DBG_OUT("branch: %s\n", git_reference_name(m_branch));

	ret = git_branch_upstream(&m_upstream, m_branch);
	if (ret) {
		print_lg2err(ret, "failed to get upstream branch");
		git_reference_free(m_branch);
		return -1;
	}
	DBG_OUT("upstream branch: %s\n", git_reference_name(m_upstream));

	ret = git_annotated_commit_from_ref(&merge_heads[0], m_repo, m_upstream);
	if (ret) {
		print_lg2err(ret, "failed to create merge head");
		git_reference_free(m_upstream);
		git_reference_free(m_branch);
		return -1;
	}

	if (analyze_merge((const git_annotated_commit **)merge_heads, 1)) {
		ret = git_merge(m_repo,
			        (const git_annotated_commit **) merge_heads,
				1, NULL, NULL);
		if (ret)
			print_lg2err(ret, "failed to merge");

		/* check for conflicts and finalize merge -> commit*/
		if (!get_conflicts())
			ret = finalize_merge();
	}

	git_annotated_commit_free(merge_heads[0]);
	git_reference_free(m_upstream);
	git_reference_free(m_branch);

	return ret;
}

int gitcli::get_repo()
{
	if (git_repository_open(&m_repo, m_dir.c_str()))
		return 0;

	return 1;
}

int gitcli::progress_cb(const char *str, int len, void *data)
{
	(void) str;
	(void) len;
	(void) data;

	return 0;
}

int gitcli::update_cb(const char *refname, const git_oid *a,
		const git_oid *b, void *data)
{
	(void)data;
	(void)a;
	(void)b;
	(void)refname;

	return 0;
}

int gitcli::cred_acquire_cb(git_cred **out,
			const char * UNUSED(url),
			const char * UNUSED(username_from_url),
			unsigned int UNUSED(allowed_types),
			void * UNUSED(payload))
{
	printf("Getting credentials...\n");
	return git_cred_ssh_key_from_agent(out, "zax");
}


int gitcli::update()
{
	int ret = 0;
	/* check if repository is there and if not init new one */
	if (!get_repo()) {
		DBG_OUT("initializing notes reposiotry");
		init_repo();
	}

	git_remote *remote = NULL;
	if (get_remote(&remote))
		return -1;

	printf("remote %p\n", remote);

	git_transfer_progress *stats;
	stats = (git_transfer_progress *)git_remote_stats(remote);

	ret = fetch(remote);
	if (!ret) {
		ret = git_remote_update_tips(remote, NULL, NULL);
		if (ret)
			print_lg2err(ret, "remote update tips failed");
	}

	if (!ret) {
		DBG_OUT("remote fetched");
		/* merge remote ref. here */
		merge();
	}

	git_remote_free(remote);

	return ret;
}

int gitcli::push()
{
	int ret = 0;
	git_strarray specs;
	char *ref = "refs/heads/master:refs/heads/master";
	git_push_options opts = GIT_PUSH_OPTIONS_INIT;

	printf("%s: called\n", __func__);
	specs.count = 1;
	specs.strings = new char * [1];
	if (!specs.strings)
		return -1;

	specs.strings[0] = ref;

	git_remote *remote = NULL;
	if (get_remote(&remote))
		return -1;

	printf("remote @ %p\n", remote);

	/* remote is already fetched */
	git_signature *sig;
	if (git_signature_default(&sig, m_repo))
		print_lg2err(ret, "Unable to create signature");


	opts.pb_parallelism = 0;

	ret = git_remote_push(remote, &specs, &opts, sig, "update notes");

	delete [] specs.strings;
	git_signature_free(sig);

	printf("%s: pushed %d\n", __func__, ret);

	return ret;
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

