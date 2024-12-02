/*
 * This file is part of Weather Station project <https://github.com/VeniaminGH/Weather-Station>.
 * Copyright (c) 2024 Veniamin Milevski
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed WITHOUT ANY WARRANTY. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "wst_events.h"

//
// Define the events partition, which will contain a memory region that
// will be accessible by both IO and APP threads
//
K_APPMEM_PARTITION_DEFINE(events_partition);

//
// Define a memory pool in the events area
//
K_APP_DMEM(events_partition) struct sys_heap events_pool;
K_APP_DMEM(events_partition) uint8_t events_pool_mem[WST_EVENTS_POOL_SIZE];

//
// Define queues for exchanging data between IO and App threads
//
K_QUEUE_DEFINE(app_events_queue);
K_QUEUE_DEFINE(io_events_queue);
