mod_logcpu
==========

What is it?
-----------

mod_logcpu adds the ability to include elapsed CPU seconds per request using
a new LogFormat directive: %...E

The CPU seconds account for the total processing time inside the Apache child,
as well as processes exec()'d or fork()'d by the Apache child on a per request
basis.

mod_logcpu has been tested with Apache 2.2, but it should work on Apache 2.X.
It's been successfully tested with mpm_event running PHP with suPHP, but should
work with any MPM and any handler assuming the handler is processed either as a
child of the Apache child, or within the Apache child itself (e.g. mod_php).

This should allow accurate process accounting even when mod_php is being used
to serve requests. It can also augment process accounting by tracking CPU
seconds for specific requests, allowing a complete view of which specific pages
are utilizing the most CPU resources.

Installation & Configuration
----------------------------

Just compile with apxs:

	apxs -ci mod_logcpu.c

... then make sure it is loaded in httpd.conf:

	LoadModule logcpu_module modules/mod_logcpu.so

... finally, define a LogFormat using the new %...E directive:

	<IfModule mod_logcpu.c>
		LogFormat "%t %f %E" cpulog
		CustomLog logs/cpu_log cpulog
	</IfModule>
