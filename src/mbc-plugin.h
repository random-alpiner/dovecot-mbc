#ifndef MBC_PLUGIN_H
#define MBC_PLUGIN_H

extern const char *mbc_plugin_dependencies[];

void mbc_plugin_init(struct module *module);
void mbc_plugin_deinit(void);

#endif
