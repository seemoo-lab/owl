#ifndef APPLE_COREWLAN_H
#define APPLE_COREWLAN_H

int corewlan_init();

void corewlan_free();

int corewlan_disassociate(int ifindex);

int corewlan_set_channel(int ifindex, int channel);

int corewlan_get_hostname(char *name, size_t len);

#endif /* APPLE_COREWLAN_H */
