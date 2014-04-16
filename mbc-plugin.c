#include "lib.h"
#include "array.h"
#include "llist.h"
#include "str.h"
#include "str-sanitize.h"

#include "mail-storage.h"
#include "mail-storage-private.h"
#include "notify-plugin.h"
#include "mbc-script-plugin.h"

#include <stdlib.h>

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

void mbc_plugin_init(void)
{
	mbc_ctx = notify_register(&mbc_vfuncs);
	mail_storage_hooks_add(module, &mbc_mail_storage_hooks);
}

void mbc_plugin_deinit(void)
{
	notify_unregister(mbc_ctx);
	mail_storage_hooks_remove(&mbc_mail_storage_hooks);
}

static struct mail_storage_hooks mbc_mail_storage_hooks = {
	.mail_user_created = mbc_mail_user_created
};

static void mbc_mail_user_created(struct mail_user *user)
{
	struct mbc_user *muser;
	const char *str;

	muser = p_new(user->pool, struct mbc_user, 1);
	MODULE_CONTEXT_SET(user, mbc_user_module, muser);

	str = mail_user_plugin_getenv(user, "mbc_script");
	muser->mbc_script_loc = str;
}

static const struct notify_vfuncs mbc_vfuncs = {
	.mailbox_create = mbc_mailbox_create,
	.mailbox_rename = mbc_mailbox_rename
};

static void
mbc_mailbox_create(struct mailbox *box)
{
	struct mbc_user *muser = mbc_USER_CONTEXT(box->storage->user);
	struct mail_namespace *ns;

	char *directory;
	char *prefix;
	
	ns = mailbox_get_namespace(box);
	if (mail_storage_is_mailbox_file(ns->storage)) {
		directory = mailbox_list_get_path(ns->list, box->name,
					    MAILBOX_LIST_PATH_TYPE_CONTROL);
	} else {
		directory = mailbox_list_get_path(ns->list, box->name,
					    MAILBOX_LIST_PATH_TYPE_MAILBOX);
	}

	if (muser->script_loc){
		setenv("MBC_MAILBOX", box->name);
		setenv("MBC_DIRECTORY", directory);
		setenv("MBC_PREFIX", ns->prefix);
		system(muser->mbc_script_loc);
		unsetenv("MBC_MAILBOX");
		unsetenv("MBC_DIRECTORY");
		unsetenv("MBC_PREFIX");
	}
}

static void
mbc_mailbox_rename(struct mailbox *src,
			struct mailbox *dest, bool rename_children ATTR_UNUSED)
{
	mbc_mailbox_create(dest);
}
