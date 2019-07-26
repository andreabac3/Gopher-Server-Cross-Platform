#ifndef GOPHER_LINUX_MEMORY_MAPPING_H
#define GOPHER_LINUX_MEMORY_MAPPING_H
void* linux_memory_mapping(void *params);
int SendFileMapped(int write_fd, char *fileToSend, int fileSize);
int fileSize(int fd);
int l_sendFile(int fd_client, char *message_to_send, int message_len);
#endif //GOPHER_LINUX_MEMORY_MAPPING_H
