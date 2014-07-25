#include <stdio.h>
#include <stdlib.h>
#include "lib.h"
#include "array.h"
#include "llist.h"
#include "str.h"
#include "str-sanitize.h"
#include "imap-util.h"
#include "module-context.h"
#include "mail-user.h"
#include "mail-storage-private.h"
#include "mail-namespace.h"
#include "notify-plugin.h"
#include "mbc-plugin.h"

#define MAILBOX_NAME_LEN 64

#define MBC_USER_CONTEXT(obj) \
	MODULE_CONTEXT(obj, mbc_mail_user_module)

static struct notify_context *mbc_ctx;
const char *mbc_plugin_dependencies[] = { "notify", NULL };

struct mbc_mail_user {
	union mail_user_module_context module_ctx;
	const char *mbc_script_loc;
};

static MODULE_CONTEXT_DEFINE(mbc_mail_user_module,
				  &mail_user_module_register);

static void mbc_mail_user_created(struct mail_user *user)
{
	struct mbc_mail_user *muser;
	const char *str;

	muser = p_new(user->pool, struct mbc_mail_user, 1);
	MODULE_CONTEXT_SET(user, mbc_mail_user_module, muser);

	str = mail_user_plugin_getenv(user, "mbc_script");
	muser->mbc_script_loc = str;
}

static struct mail_storage_hooks mbc_mail_storage_hooks = {
	.mail_user_created = mbc_mail_user_created
};

static void
mbc_mailbox_create(struct mailbox *box)
{
	struct mbc_mail_user *muser = MBC_MAIL_USER_CONTEXT(box->storage->user);

	char **exec_args;
	char *directory;
	struct mail_namespace *ns = mailbox_get_namespace(box);
	char *prefix;
	char *mns_type;
	char *is_inbox = "false", *is_hidden = "false", *handles_subscriptions = "false";
	char *listed;
	const char **path_r;

	if (ns->set->inbox) {
		is_inbox = "true";
	}
	if (ns->set->hidden) {
		is_hidden = "true";
	}
	if (ns->set->subscriptions) {
		handles_subscriptions = "true";
	}
	if (ns->set->list) {
		listed = "true";
	}

	if (ns->type == MAIL_NAMESPACE_TYPE_PRIVATE) {
		mns_type = "private";
	} else if (ns->type == MAIL_NAMESPACE_TYPE_PUBLIC) {
		mns_type = "public";
	} else if (ns->type == MAIL_NAMESPACE_TYPE_SHARED) {
		mns_type = "shared";
	}

	prefix = t_strdup(ns->set->prefix);
	if (mail_storage_is_mailbox_file(box->storage)) {
		directory = mailbox_list_get_path(box->list, box->name,
					    MAILBOX_LIST_PATH_TYPE_CONTROL, path_r);
	} else {
		directory = mailbox_list_get_path(box->list, box->name,
					    MAILBOX_LIST_PATH_TYPE_MAILBOX, path_r);
	}

	exec_args = i_new(const char *, 9);
	exec_args[0] = muser->mbc_script_loc;
	exec_args[1] = box->name;
	exec_args[2] = directory;
	exec_args[3] = prefix;
	exec_args[4] = is_inbox;
	exec_args[5] = is_hidden;
	exec_args[6] = handles_subscriptions;
	exec_args[7] = listed;
	exec_args[8] = NULL;

	if (muser->mbc_script_loc){
		env_put(t_strconcat("MBC_MAILBOX=", box->name, NULL));
		env_put(t_strconcat("MBC_DIRECTORY=", directory, NULL));
		env_put(t_strconcat("MBC_PREFIX=", prefix, NULL));
		env_put(t_strconcat("MBC_TYPE=", mns_type, NULL));
		env_put(t_strconcat("MBC_INBOX=", is_inbox, NULL));
		env_put(t_strconcat("MBC_HIDDEN=", is_hidden, NULL));
		env_put(t_strconcat("MBC_SUBSCRIPTIONS=", handles_subscriptions, NULL));
		env_put(t_strconcat("MBC_LIST=", listed, NULL));
		execvp_const(exec_args[0], exec_args);
		env_remove("MBC_MAILBOX");
		env_remove("MBC_DIRECTORY");
		env_remove("MBC_PREFIX");
		env_remove("MBC_TYPE");
		env_remove("MBC_INBOX");
		env_remove("MBC_HIDDEN");
		env_remove("MBC_SUBSCRIPTIONS");
		env_remove("MBC_LIST");
	}
}

static void
mbc_mailbox_rename(struct mailbox *src,
			struct mailbox *dest, bool rename_children ATTR_UNUSED)
{
	mbc_mailbox_create(dest);
}

static const struct notify_vfuncs mbc_vfuncs = {
	.mailbox_create = mbc_mailbox_create,
	.mailbox_rename = mbc_mailbox_rename
};

void mbc_plugin_init(struct module *module)
{
	mbc_ctx = notify_register(&mbc_vfuncs);
	mail_storage_hooks_add(module, &mbc_mail_storage_hooks);
}

void mbc_plugin_deinit(void)
{
	notify_unregister(mbc_ctx);
	mail_storage_hooks_remove(&mbc_mail_storage_hooks);
}
