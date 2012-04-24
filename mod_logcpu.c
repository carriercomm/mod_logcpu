/**
 * Copyright (C) 2012 Paul Stengel
 *
 * This program is free software: you can redistribute it and/or modify
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
#include "apr_optional.h"

#include "mod_log_config.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"

#include <sys/times.h>
#include <unistd.h>

/* CPU clock running total for the child */
static double cumulative = 0.0;

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
	/* Wait for any children to finish before continuing */
	wait(NULL);

	double elapsed = (double)(get_total_cpu() - cumulative);
	cumulative += elapsed;

	return apr_psprintf(r->pool, "%.2f", elapsed / sysconf(_SC_CLK_TCK));
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
 * Apache 2.X hooks
 */
static void logcpu_hooks(apr_pool_t *p)
{
	static const char *pre[] = { "mod_log_config.c", NULL };

	ap_hook_pre_config(logcpu_pre_config, NULL, NULL, APR_HOOK_REALLY_FIRST);
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
