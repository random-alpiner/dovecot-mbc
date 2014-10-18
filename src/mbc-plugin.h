#ifndef MBC_PLUGIN_H
#define MBC_PLUGIN_H
#define LOGFILE "/var/log/mail.log" // all Log(); messages will be appended to this file

extern const char *mbc_plugin_dependencies[];

void mbc_plugin_init(struct module *module);
void mbc_plugin_deinit(void);

#endif
