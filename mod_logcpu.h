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
