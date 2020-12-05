#include <arpa/inet.h>


namespace sockoptions
{

int createNonblockingOrDie(sa_family_t family);
void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDie(int sockfd);
void close(int sockfd);
int  accept(int sockfd, struct sockaddr_in6* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);

}
