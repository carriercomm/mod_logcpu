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

struct process_chain {
	apr_proc_t *proc;
	apr_kill_conditions_e kill_how;
	struct process_chain *next;
};

/* Hackish struct scaffolding */
struct apr_pool_t {
	void *not_impl1[3];
	void **not_impl2;
	void *not_impl3[3];
	struct process_chain *subprocesses;
	apr_abortfunc_t abort_fn;
	void *not_impl4[4];
};
