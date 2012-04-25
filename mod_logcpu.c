/**
 * Copyright (C) 2012 Paul Stengel
 *
 * This file is part of mod_logcpu.
 *
 * mod_logcpu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This module adds the following LogFormat arguments, denoted by the following
 * '%' directives:
 *
 * %...E: CPU seconds elapsed for the request
 *
 */

#include "apr_strings.h"
#include "apr_tables.h"
#include "apr_pools.h"
#include "apr_thread_proc.h"
#include "apr_optional.h"

#include "mod_log_config.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "ap_mpm.h"

#include <sys/times.h>
#include <sys/resource.h>
#include <unistd.h>

#include "mod_logcpu.h"

/*
 * Per child variables
 */
static int mpm_is_threaded = 0;
static clock_t cumulative = 0;
static apr_thread_mutex_t *logcpu_cumulative_lock = NULL;

/*
 * Gets the current CPU clock total using times()
 */
static clock_t get_total_cpu()
{
	struct tms cpu_time;
	times(&cpu_time);

	clock_t parent_time = cpu_time.tms_utime + cpu_time.tms_stime;
	clock_t child_time = cpu_time.tms_cutime + cpu_time.tms_cstime;

	return (parent_time + child_time);
}

/*
 * Calculates time elapsed and returns the formatted value
 */
static const char *log_cpu_elapsed(request_rec *r, char *a)
{
	struct apr_pool_t *p = r->pool;

	if (mpm_is_threaded) {
		struct process_chain *pc = p->subprocesses;

		if (pc && pc->proc)
			apr_proc_wait(pc->proc, NULL, NULL, APR_WAIT);

		apr_thread_mutex_lock(logcpu_cumulative_lock);
	}
	else {
		apr_proc_wait_all_procs(NULL, NULL, NULL, APR_WAIT, p);
	}

	clock_t elapsed = get_total_cpu() - cumulative;
	cumulative += elapsed;

	if (mpm_is_threaded)
		apr_thread_mutex_unlock(logcpu_cumulative_lock);

	return apr_psprintf(p, "%.2f", (double)elapsed / sysconf(_SC_CLK_TCK));
}

/*
 * Registers the log directives with mod_log_config.c
 */
static int logcpu_pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
	static APR_OPTIONAL_FN_TYPE(ap_register_log_handler) *log_pfn_register;

	log_pfn_register = APR_RETRIEVE_OPTIONAL_FN(ap_register_log_handler);

	if (log_pfn_register)
		log_pfn_register(p, "E", log_cpu_elapsed, 0);

	return OK;
}

/*
 * Figure out threading for the current MPM and setup the mutex if necessary
 */
static void logcpu_child_init(apr_pool_t *p, server_rec *s)
{
	ap_mpm_query(AP_MPMQ_IS_THREADED, &mpm_is_threaded);

	if (mpm_is_threaded)
		apr_thread_mutex_create(&logcpu_cumulative_lock, APR_THREAD_MUTEX_DEFAULT, p);
}

/*
 * Apache 2.X hooks
 */
static void logcpu_hooks(apr_pool_t *p)
{
	static const char *pre[] = { "mod_log_config.c", NULL };

	ap_hook_pre_config(logcpu_pre_config, NULL, NULL, APR_HOOK_REALLY_FIRST);
	ap_hook_child_init(logcpu_child_init, NULL, NULL, APR_HOOK_FIRST);
}

/*
 * Standard module declaration
 */
module AP_MODULE_DECLARE_DATA logcpu_module = {
	STANDARD20_MODULE_STUFF,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	logcpu_hooks
};
