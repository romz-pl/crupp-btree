#include "0root/root.h"

#include "1base/error.h"

#include "socket.h"
#include "file.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifndef UPS_ROOT_H
#  error "root.h was not included"
#endif

namespace upscaledb {

#if 0
#  define os_log(x)      ups_log(x)
#else
#  define os_log(x)
#endif






void
Socket::connect(const char *hostname, uint16_t port, uint32_t timeout_sec)
{
  ups_socket_t s = ::socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    ups_log(("failed creating socket: %s", strerror(errno)));
    throw Exception(UPS_IO_ERROR);
  }

  struct hostent *server = ::gethostbyname(hostname);
  if (!server) {
    ups_log(("unable to resolve hostname %s: %s", hostname,
                hstrerror(h_errno)));
    ::close(s);
    throw Exception(UPS_NETWORK_ERROR);
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);
  addr.sin_port = htons(port);
  if (::connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    ups_log(("unable to connect to %s:%d: %s", hostname, (int)port,
                strerror(errno)));
    ::close(s);
    throw Exception(UPS_NETWORK_ERROR);
  }

  if (timeout_sec) {
    struct timeval tv;
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    if (::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)) < 0) {
      ups_log(("unable to set socket timeout to %d sec: %s", timeout_sec,
                  strerror(errno)));
      // fall through, this is not critical
    }
  }

  m_socket = s;
}

void
Socket::send(const uint8_t *data, size_t len)
{
  File::os_write(m_socket, data, len);
}

void
Socket::recv(uint8_t *data, size_t len)
{
  File::os_read(m_socket, data, len);
}

void
Socket::close()
{
  if (m_socket != UPS_INVALID_FD) {
    if (::close(m_socket) == -1)
      throw Exception(UPS_IO_ERROR);
    m_socket = UPS_INVALID_FD;
  }
}

} // namespace upscaledb
