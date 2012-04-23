mod_logcpu
==========

What is it?
-----------

mod_logcpu adds the ability to include elapsed CPU seconds per request using
a new LogFormat directive: %...E

This logs CPU seconds for forked processes, as well as requests handled
exclusively by the Apache children themselves (e.g. mod_php). This has been
tested with mpm_event and mpm_prefork, both of which work.

Installation & Configuration
----------------------------

Just compile with apxs:

	apxs -ci mod_logcpu.c

... then make sure it is loaded in httpd.conf:

	LoadModule logcpu_module modules/mod_logcpu.so

... finally, define a LogFormat using the new %...E directive:

	<IfModule mod_logcpu.so>
		LogFormat "%t %f %E" cpulog
		CustomLog logs/cpu_log cpulog
	</IfModule>
