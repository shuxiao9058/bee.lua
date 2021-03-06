#include <bee/net/endpoint.h>
#if defined(_WIN32)
#   include <Ws2tcpip.h>
#else
#   include <netdb.h>
#   include <sys/un.h>
#   include <arpa/inet.h>
#   ifndef UNIX_PATH_MAX
#       define UNIX_PATH_MAX (sizeof(sockaddr_un::sun_path) / sizeof(sockaddr_un::sun_path[0]))
#   endif
#endif
#include <bee/utility/format.h>
#include <bee/error.h>
#if __has_include(<charconv>)
#   include <charconv>
#endif

//
// need Windows SDK >= 17063
// see the https://blogs.msdn.microsoft.com/commandline/2017/12/19/af_unix-comes-to-windows/
//
#if defined(_WIN32)
    // TODO: Windows SDK 16299 has a wrong `afunix.h`
	#if __has_include(<afunix.h>)
		#include <afunix.h>
	#else
		#define UNIX_PATH_MAX 108
		struct sockaddr_un {
			unsigned short sun_family;
			char sun_path[UNIX_PATH_MAX];
		};
	#endif
#endif

namespace bee::net {
    struct autorelease_addrinfo {
        autorelease_addrinfo(autorelease_addrinfo const&) = delete;
        autorelease_addrinfo& operator=(autorelease_addrinfo const&) = delete;
        autorelease_addrinfo& operator=(autorelease_addrinfo&&) = delete;
        autorelease_addrinfo(addrinfo* i) : info(i) { }
        autorelease_addrinfo(autorelease_addrinfo&& o) : info(o.info) { o.info = 0; }
        ~autorelease_addrinfo() { if (info) ::freeaddrinfo(info); }
        operator bool() const { return !!info; }
        const addrinfo* operator->() const { return info; }
        const addrinfo& operator* () const { return *info; }
        addrinfo* info;
    };

    static bool needsnolookup(const std::string_view& ip) {
        size_t pos = ip.find_first_not_of("0123456789.");
        if (pos == std::string_view::npos) {
            return true;
        }
        pos = ip.find_first_not_of("0123456789abcdefABCDEF:");
        if (pos == std::string_view::npos) {
            return true;
        }
        if (ip[pos] != '.') {
            return false;
        }
        pos = ip.find_last_of(':');
        if (pos == std::string_view::npos) {
            return false;
        }
        pos = ip.find_first_not_of("0123456789.", pos);
        if (pos == std::string_view::npos) {
            return true;
        }
        return false;
    }

    static autorelease_addrinfo gethostaddr(const addrinfo& hint, const std::string_view& ip, const char* port) {
        addrinfo* info = 0;
        int err = ::getaddrinfo(ip.data(), port, &hint, &info);
        if (err != 0) {
            if (info) ::freeaddrinfo(info);
            info = 0;
        }
        return autorelease_addrinfo(info);
    }
    nonstd::expected<endpoint, std::string> endpoint::from_unixpath(const std::string_view& path) {
        if (path.size() >= UNIX_PATH_MAX) {
            return nonstd::make_unexpected("unix domain sockect: pathname too long");
        };
        endpoint ep(offsetof(struct sockaddr_un, sun_path) + path.size() + 1);
        struct sockaddr_un* su = (struct sockaddr_un*)ep.data();
        su->sun_family = AF_UNIX;
        memcpy(&su->sun_path[0], path.data(), path.size());
        su->sun_path[path.size()] = '\0';
        return std::move(ep);
    }
    nonstd::expected<endpoint, std::string> endpoint::from_hostname(const std::string_view& ip, int port) {
        addrinfo hint = { };
        hint.ai_family = AF_UNSPEC;
        if (needsnolookup(ip)) {
            hint.ai_flags = AI_NUMERICHOST;
        }
#if __has_include(<charconv>)
        std::array<char, 10> portstr;
        if (auto[p, ec] = std::to_chars(portstr.data(), portstr.data() + portstr.size() - 1, port); ec != std::errc()) {
            return nonstd::make_unexpected(make_error_code(ec).message());
        }
        else {
            p[0] = '\0';
        }
        auto info = gethostaddr(hint, ip, portstr.data());
#else
        auto info = gethostaddr(hint, ip, bee::format("%d", port).c_str());
#endif
        if (!info) {
            return nonstd::make_unexpected(make_neterror("getaddrinfo").what());
        }
        else if (info->ai_family != AF_INET && info->ai_family != AF_INET6) {
            return nonstd::make_unexpected("unknown address family");
        }
        const addrinfo& addrinfo = *info;
        endpoint ep(addrinfo.ai_addrlen);
        memcpy(ep.data(), addrinfo.ai_addr, ep.size());
        return std::move(ep);
    }
    endpoint endpoint::from_empty() {
        return endpoint(endpoint::kMaxSize);
    }

    endpoint::endpoint(size_t n) : mybase(n)
    { }
    endpoint_info endpoint::info() const {
        const sockaddr* sa = addr();
        if (sa->sa_family == AF_INET) {
            char tmp[sizeof "255.255.255.255"];
#if !defined(__MINGW32__)
            const char* s = inet_ntop(sa->sa_family, (const void*) &((struct sockaddr_in*)sa)->sin_addr, tmp, sizeof tmp);
#else
            const char* s = inet_ntop(sa->sa_family, (void*) &((struct sockaddr_in*)sa)->sin_addr, tmp, sizeof tmp);
#endif
            return { std::string(s), ntohs(((struct sockaddr_in*)sa)->sin_port) };
        }
        else if (sa->sa_family == AF_INET6) {
            char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];
#if !defined(__MINGW32__)
            const char* s = inet_ntop(sa->sa_family, (const void*) &((const struct sockaddr_in6*)sa)->sin6_addr, tmp, sizeof tmp);
#else
            const char* s = inet_ntop(sa->sa_family, (void*) &((const struct sockaddr_in6*)sa)->sin6_addr, tmp, sizeof tmp);
#endif
            return { std::string(s), ntohs(((struct sockaddr_in6*)sa)->sin6_port) };
        }
        else if (sa->sa_family == AF_UNIX) {
            const char* path = ((struct sockaddr_un*)sa)->sun_path;
            int len = addrlen() - offsetof(struct sockaddr_un, sun_path) - 1;
            if (len > 0 && path[0] != 0) {
                return { std::string(path, len), 0 };
            }
            else if (len > 1) {
                return { std::string(path + 1, len - 1), 1 };
            }
            else {
                return { std::string(), 1 };
            }
        }
        return { "", 0 };
    }
    sockaddr* endpoint::addr() {
        return (sockaddr*)mybase::data();
    }
    const sockaddr* endpoint::addr() const {
        return (const sockaddr*)mybase::data();
    }
    socklen_t endpoint::addrlen() const {
        return (socklen_t)mybase::size();
    }
    void endpoint::resize(socklen_t len) {
        if (addrlen() <= len) {
            return;
        }
        *(mybase*)this = std::move(mybase::subspan(0, len));
    }
    int endpoint::family() const {
        return addr()->sa_family;
    }
}
