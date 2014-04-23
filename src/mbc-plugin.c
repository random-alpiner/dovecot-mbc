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
	MODULE_CONTEXT(obj, mbc_user_module)

static struct notify_context *mbc_ctx;
const char *mbc_plugin_dependencies[] = { "notify", NULL };

struct mbc_user {
	union mail_user_module_context module_ctx;
	const char *mbc_script_loc;
};

static MODULE_CONTEXT_DEFINE_INIT(mbc_user_module,
				  &mail_user_module_register);

static void mbc_mail_user_created(struct mail_user *user)
{
	struct mbc_user *muser;
	const char *str;

	muser = p_new(user->pool, struct mbc_user, 1);
	MODULE_CONTEXT_SET(user, mbc_user_module, muser);

	str = mail_user_plugin_getenv(user, "mbc_script");
	muser->mbc_script_loc = str;
}

static struct mail_storage_hooks mbc_mail_storage_hooks = {
	.mail_user_created = mbc_mail_user_created
};

static void
mbc_mailbox_create(struct mailbox *box)
{
	struct mbc_user *muser = MBC_USER_CONTEXT(box->storage->user);

	char **exec_args;
	char *directory;
	struct mail_namespace *ns = mailbox_get_namespace(box);
	char *prefix;
	char *mns_type;

	if (ns->type == NAMESPACE_PRIVATE) {
		i_info("MBC Script: Namespace is private");
		mns_type = "private";
	} else if (ns->type == NAMESPACE_PUBLIC) {
		i_info("MBC Script: Namespace is public");
		mns_type = "public";
	} else if (ns->type == NAMESPACE_SHARED) {
		i_info("MBC Script: Namespace is shared");
		mns_type = "shared";
	}

	prefix = t_strdup(ns->set->prefix);
	i_info("MBC Script: Prefix is %s", prefix);
	if (mail_storage_is_mailbox_file(box->storage)) {
		directory = mailbox_list_get_path(box->list, box->name,
					    MAILBOX_LIST_PATH_TYPE_CONTROL);
	} else {
		directory = mailbox_list_get_path(box->list, box->name,
					    MAILBOX_LIST_PATH_TYPE_MAILBOX);
	}

	exec_args = i_new(const char *, 5);
	exec_args[0] = muser->mbc_script_loc;
	exec_args[1] = box->name;
	exec_args[2] = directory;
	exec_args[3] = prefix;
	exec_args[4] = NULL;

	if (muser->mbc_script_loc){
		env_put(t_strconcat("MBC_MAILBOX=", box->name, NULL));
		env_put(t_strconcat("MBC_DIRECTORY=", directory, NULL));
		env_put(t_strconcat("MBC_PREFIX=", prefix, NULL));
		env_put(t_strconcat("MBC_TYPE=", mns_type, NULL));
		execvp_const(exec_args[0], exec_args);
		env_remove("MBC_MAILBOX");
		env_remove("MBC_DIRECTORY");
		env_remove("MBC_PREFIX");
		env_remove("MBC_TYPE");
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
