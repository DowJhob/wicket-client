#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H
enum state {
    ready,
    //locked,
    unlocked,
    undefined,
    network_search_host,
    disconnected,
    error
};
enum class net_state
{
    undefined,
    search,
    tcp_searched,
    tcp_connected
};
enum _reader_type {
    _main,
    slave
};
enum dir_type {
    entry,
    exit_
};
enum color {
    red,
    green
};
enum picture {
    pict_service,
    pict_ready,
    pict_onCheck,
    pict_access,
    pict_timeout,
    pict_denied
};
#endif // COMMON_TYPES_H
