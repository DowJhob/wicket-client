#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H
enum state {
    ready,
    //locked,
    unlocked,
    busy_send_entry_barcode,
    busy_send_exit_barcode,
    busy_wait_pass_entry,
    busy_wait_pass_exit,
    passed,
    wrong,
    undefined,
    network_search_host,
    disconnected,
    error
};
enum direction_state {
    dir_entry,
    dir_exit
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
