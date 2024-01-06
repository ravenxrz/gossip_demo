#include "server_flags.h"

DEFINE_uint32(port, 5555, "server port");
DEFINE_string(conf_path, "", "cluster conf path");
DEFINE_uint32(gossip_interval_ms, 1000, "gossip interval");
